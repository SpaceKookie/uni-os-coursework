//#define DEBUG 0

#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include "ult.h"
#include "tcb.h"
#include <setjmp.h>
#include <stdbool.h>
#include <sys/select.h>
#include <errno.h>

// 64kB stack
#define STACK 1024*64

// structs

struct new_thread_list {
    int id;
    ult_func f;
    struct new_thread_list *next;
};

struct thread {
    int id;
    jmp_buf jumpPoint;
    stack_t stack;
    bool isZombie;
};

struct thread_list_item
{
    struct thread thread;
    struct thread_list_item *next;
};


// global variables

struct new_thread_list *newThreadRoot;
jmp_buf scheduling_start_buf;
struct thread_list_item *threadListRoot;
int threadCounter;
int schedCounter;
struct thread *currRunningThread;


// todo: strategie? performance? O(n) will work for now!
struct thread *selectNextThread() {
    struct thread *selected = NULL;
    
    do {
        if(!threadListRoot->next) {
            selected = &threadListRoot->thread;
        } else {
            struct thread_list_item *c = threadListRoot;
            struct thread_list_item *p = NULL;
            while(c->next) {
                p = c;
                c = c->next;
            }
            
            // c is the last, p is previous
            
            p->next = NULL;
            c->next = threadListRoot;
            threadListRoot = c;
            
            selected = &threadListRoot->thread;
        }
    } while (selected->isZombie);
    return selected;
}

/*
 This function only exists to tell the process to use an empty stack for the thread
 */
ult_func newStackFunc;
jmp_buf *newStackJmpBuf;
// jmp_buf newStackReturnAfterHandlersJmpBuf;

void createStackSignalHandler(int arg) {
//    printf("Create a new thread\n");
    ult_func function = newStackFunc; // copy the func pointer to stack
    if (setjmp(*newStackJmpBuf)) {
        function();
        
        // thread is dead! o.O!
        printf("Thread run out");
        ult_exit(-1);
        // Run the waitpid thing here? :/
    }
    
    // longjmp(newStackReturnAfterHandlersJmpBuf, 1);
}

// TODO: check if correct
struct thread createThread(ult_func f, int id) {
    struct sigaction sa;
    struct thread t;
    memset(&t, 0, sizeof(struct thread)); //clean
    t.id = id;
    
    // Jetzt müssen wir für den Thread einen stack initiieren und einen jmp auf diesen setzen.
    t.stack.ss_flags = 0;
    t.stack.ss_size = STACK;
    t.stack.ss_sp = malloc(STACK);

    // Wenn die stack Größe 0 ist.
    if (t.stack.ss_sp == 0)
    {
        perror("Could not allocate stack.");
        exit(1);
    }
    
    sigaltstack(&t.stack, 0); // magic. use this stack for signals. what signals?

    // Kopiert aus der Vorlage
    sa.sa_handler = &createStackSignalHandler;
    sa.sa_flags = SA_ONSTACK; // magic
    sigemptyset(&sa.sa_mask); // magic
    sigaction(SIGUSR1, &sa, 0);


    // First time this is run returns a 0 and skips the if statement but setsups "setjump" point.
    // Then with a longjump returns a 1 and goes into the if statement.
//    if(setjmp(newStackReturnAfterHandlersJmpBuf)) {
//    }
    newStackFunc = f; // give into the signal
    newStackJmpBuf = &t.jumpPoint;
    raise(SIGUSR1); // go on the stack. never returns

    newStackFunc = NULL;
    newStackJmpBuf = NULL;

    // Then returns from the fuction.
    return t;
}

/* LK: Run a thread
 * EVERY Call to run a thread MUST be invoked from here!
 * Here we can make sure to save whar thread is running so we know what is returning in a yiel/exit.
 * This function will NEVER RETURN because of a longjmp.
 */
void runThread(struct thread *t) {
    currRunningThread = t;
    longjmp(currRunningThread->jumpPoint, 1);
}

void handleNewThreads() {
    struct new_thread_list *c = newThreadRoot;
    while(c) {
        struct thread_list_item *ti = calloc(1, sizeof(struct thread_list_item));
        ti->thread = createThread(c->f, c->id);
        ti->next = threadListRoot;
        threadListRoot = ti;
        
        // struct new_thread_list *p = c;
        c = c->next;
        // free(p);
    }
    
    newThreadRoot = NULL;
}

// tricky. never returns!
void startScheduling() {
    struct thread *t;
    setjmp(scheduling_start_buf);

//    printf("SchedCycle => %i\n", schedCounter++);
    
    // ok ab hier keine Variablen mehr deklarieren!
    // wir laufen hier mehrmals rein, und überschreiben jeweils unseren alten stack dabei.
    
    handleNewThreads();
    
    t = selectNextThread();
    runThread(t); // never returns. but the stack will be reset by a longjmp to the scheduling_start_buf.
}

// spawn a new thread, return its ID
/*	Die Funktion ult_spawn() erzeugt einen Thread-Control-Block (TCB) fuer
 die Funktion f() und stellt den Thread in die Run-Queue. Sie liefert
 den ID des erzeugten Threads zurueck. An dieser Stelle erfolgt kein
 Scheduling, sondern die Abarbeitung wird mit dem laufenden Thread
 fortgesetzt, um die Erzeugung mehrerer Threads zu ermoeglichen.
 */
int ult_spawn(ult_func f) {
    struct new_thread_list *c = calloc(1, sizeof(struct new_thread_list));
    c->next = newThreadRoot;
    c->f = f;
    c->id = threadCounter++;
    newThreadRoot = c;
    
    return c->id;
}


// yield the CPU to another thread
/*	Die Funktion ult_yield() gibt innerhalb eines Threads freiwillig den
 Prozessor auf. Es erfolgt der Aufruf des Schedulers und die Umschaltung
 auf den vom Scheduler ausgewaehlten Thread.
 */
void ult_yield() {
	// okay, der trick ist folgender:
	// enviroment save, speichern, scheduler aufrufen.
    if (setjmp(currRunningThread->jumpPoint)) {
        // If this setjump is jumped to with longjump it returns a 1
        // Thus we want to return to the thread and continue after the yield.
        return;
    } else {
        // Means that we just yielded and want to go back to scheduling.
        // Long jumps back to scheduling thread.
        longjmp(scheduling_start_buf, 1); // start scheduling. never returns, but setjmp will return with non zero later.
    }
}


// current thread exits
/*	Wird innerhalb eines Threads ult_exit() aufgerufen, so wird der Thread
 zum Zombie und der Exit-Status wird abgespeichert.
 */
void ult_exit(int status) {
    currRunningThread->isZombie = true;
    memset(&currRunningThread->jumpPoint, 0, sizeof(jmp_buf)); // never jump into this thread again!
    longjmp(scheduling_start_buf, 1); // never returns
}

struct thread * findThread(int id) {
    struct thread_list_item *t = threadListRoot;
    while(t && t->thread.id != id) {
        t = t->next;
    }
    
    // Wild west code: it was hard to write, it should be hard to read :)
    return t ? &t->thread : NULL;
}

// thread waits for termination of another thread
// returns 0 for success, -1 for failure
// exit-status of terminated thread is obtained
/*	Mit ult_waitpid() kann abgefragt werden, ob der Thread mit dem angegebenen
 ID bereits beendet wurde. Ist der Thread bereits beendet, so kehrt die
 Funktion sofort zurueck und liefert in status den Exit-Status des Threads
 (welcher als Argument an ult_exit() uebergeben wurde). Laeuft der Thread noch,
 so soll der aktuelle Thread blockieren und es muss auf einen anderen Thread
 umgeschaltet werden. Bei nachfolgenden Aufrufen des Schedulers soll ueberprueft
 werden, ob der Thread mittlerweile beendet wurde; ist dies der Fall, so soll
 der aktuelle Thread geweckt werden (am besten so, dass er auch als naechster
 Thread die CPU erhaelt).
 */
int ult_waitpid(int tid, int *status) {
    ult_yield();
    struct thread *t = findThread(tid);
    
    if(!t) {
        return -1;
    }
    
    while (!t->isZombie) {
        ult_yield();
    }
    
    return 0;
}



// read from file, block the thread if no data available
/*	Hinter ult_read() verbirgt sich die Funktion read() der Unix-API, welche Daten
 aus einer Datei (File-Deskriptor fd) in einen Puffer (buf) liest. Die Funktion
 ult_read() ist eine Wrapper-Funktion fuer read(), welche zusaetzlich ueberprueft, ob
 ausreichend Daten verfuegbar sind. Ist dies nicht der Fall, so wird der Thread
 blockiert und ein anderer Thread erhaelt die CPU. Bei nachfolgenden Aufrufen des
 Schedulers soll ueberprueft werden, ob mittlerweile ausreichend Daten vorliegen; ist
 dies der Fall, so soll der aktuelle Thread geweckt werden (s.o.). Dies loest ein
 Problem mit Systemrufen, welche im Kernel blockieren koennen (wie z.B. read()).
 Ohne diesen Mechanismus wuerde die gesamte User-Level-Thread-Bibliothek, die aus Sicht
 des Kernels ein Prozess ist, blockieren (selbst wenn andere User-Level-Threads
 lauffaehig waeren). Wir kuemmern uns hier nur um die read()-Funktion, obwohl auch andere
 Systemrufe blockieren koennen.
 */
int ult_read(int fd, void *buf, int count) {
    fd_set fd_read;
    struct timeval t;// = { .tv_usec = 1000 };
    t.tv_sec = 0;
    t.tv_usec = 10;
    
    int sel;
    fflush(stdout);
    FD_ZERO(&fd_read);
    FD_SET(fd, &fd_read);
    sel = select(fd+1, &fd_read, NULL, NULL, &t);
    while (!FD_ISSET(fd, &fd_read)) {
        if(sel < 0) {
            printf("err: %d", errno);
        }
        ult_yield();
        fflush(stdout);
        FD_ZERO(&fd_read);
        FD_SET(STDIN_FILENO, &fd_read);
        sel = select(fd+1, &fd_read, NULL, NULL, &t);
    }
    
    ssize_t res =read(fd, buf, count);
    
    // cast to int because return should be ssize_t but is ssize_t. why the hell?
    return (int)res;
}


// start scheduling and spawn a thread running function f
/*	Die Funktion ult_init() initialisiert die Bibliothek (insbesondere die Datenstrukturen
 des Schedulers wie z.B. Thread-Listen) und muss vor allen anderen Funktionen aufgerufen
 werden. Sie erzeugt einen einzigen Thread (analog zum Init-Thread� bei Unix), aus
 welchem heraus dann alle anderen Threads erzeugt werden und welcher danach mit
 ult_waitpid() auf das Ende aller Threads wartet. 
 */
void ult_init(ult_func f) {
    newThreadRoot = NULL;
    threadCounter = 0;
    schedCounter = 0;
    threadListRoot = NULL;
    currRunningThread = NULL;

    // Erzeugt einen Thread
    ult_spawn(f);
    
    startScheduling(); // never returns (hopefully)
}
