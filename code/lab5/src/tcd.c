// Include header files
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sched.h>
#include <unistd.h>
#include <time.h>
#include <stdbool.h>

// Set lock mode (NOLOCK, BIGLOCK or none for individual locks)
//#define BIGLOCK

// Count of collectors in simulation
#define NUM_COLLECTORS  5

// Start amount per collector
#define START_AMOUNT_OF_MONEY   300

// Collector structure
typedef struct
{
    int id;
    int money;

    int levy_count;
    int payment_count;

    #if !(defined NOLOCK || defined BIGLOCK)
        // Declare individual lock
        pthread_mutex_t lock;
    #endif

    pthread_t thread;
} Collector;

#ifdef BIGLOCK
    // Declare big lock
    pthread_mutex_t biglock;
#endif

// Running indicator
bool running;

// Collectors
Collector *collectors;
int num_collectors;

// Random number
Collector* get_rand_collector(Collector *this)
{
    // Get random index
    int collectorIndex = rand() % (num_collectors - 1);

    // Check whether in front or after current collector
    if (collectorIndex < this->id)
    {
        return &collectors[collectorIndex];
    }
    else
    {
        return &collectors[collectorIndex + 1];
    }
}

// Lock collectors
void collectors_lock(Collector *c1, Collector *c2)
{
    #ifdef BIGLOCK
        // Lock big lock
        pthread_mutex_lock(&biglock);
    #elif !(defined NOLOCK)
        // Lock collectors in right order
        if (c1->id < c2->id)
        {
            pthread_mutex_lock(&c1->lock);
            pthread_mutex_lock(&c2->lock);
        }
        else
        {
            pthread_mutex_lock(&c2->lock);
            pthread_mutex_lock(&c1->lock);
        }
    #endif
}

// Unlock collectors
void collectors_unlock(Collector *c1, Collector *c2)
{
    #ifdef BIGLOCK
        // Unlock big lock
        pthread_mutex_unlock(&biglock);
    #elif !(defined NOLOCK)
        // Unlock collectors
        pthread_mutex_unlock(&c1->lock);
        pthread_mutex_unlock(&c2->lock);
    #endif
}

// Transfer money
void exec_transaction(Collector *collector, Collector *victim)
{
    // Calculate amount
    int amount = victim->money / 2;
    amount = ((int)((amount - 1) / 100)) * 100 + 100;

    // Transfer amount
    collector->money += amount;
    victim->money -= amount;

    // Increase transaction counters
    ++collector->levy_count;
    ++victim->payment_count;
}

// Collector thread
void *thread_collector(void *this_collector)
{
    // Cast argument to collector
    Collector *this = (Collector *) this_collector;

    // Levy taxes as long as running
    while (running)
    {
        // Select random collector
        Collector *selected = get_rand_collector(this);

        // Lock collectors
        collectors_lock(this, selected);

        // Check whether selected collector can pay
        if (selected->money >= 100)
        {
            // Transfer money
            exec_transaction(this, selected);
        }

        // Unlock collectors
        collectors_unlock(this, selected);
        
        // Yield to let others work
        sched_yield();
    }

    // Exit thread
    pthread_exit(NULL);
}

// Print lock mode info
void print_lock_mode_info()
{
    #if defined BIGLOCK
        printf("Biglock\n");
    #elif defined NOLOCK
        printf("No Lock\n");
    #else
        printf("1 lock per c\n");
    #endif
}

// Initialize locks
void init_locks()
{
    #ifdef BIGLOCK
        // Initialize big lock
        pthread_mutex_init(&biglock, NULL);
    #elif !(defined NOLOCK)
        // Initialize individual locks
        int i;
        for (i = 0; i < num_collectors; ++i)
        {
            pthread_mutex_init(&collectors[i].lock, NULL);
        }
    #endif
}

// Destroy locks
void destroy_locks()
{
    #ifdef BIGLOCK
        // Destroy big lock
        pthread_mutex_destroy(&biglock);
    #elif !(defined NOLOCK)
        // Destroy individual locks
        int i;
        for (i = 0; i < num_collectors; ++i)
        {
            pthread_mutex_destroy(&collectors[i].lock);
        }
    #endif
}

// Print simulation statistics
void print_statictics()
{
    // Initialize total counters
    int total_money = 0;
    int total_payment_count = 0;
    int total_levy_count = 0;

    // Calculate and print statistics
    int i;
    for (i = 0; i < num_collectors; ++i)
    {
        // Print individual statistics
        printf("Collector %i:\n", i);
        printf("\tLevied %i times\n", collectors[i].levy_count);
        printf("\tPaid %i times\n", collectors[i].payment_count);
        printf("\tMoney: %i\n", collectors[i].money);

        // Collect total statistics
        total_money += collectors[i].money;
        total_levy_count += collectors[i].levy_count;
        total_payment_count += collectors[i].payment_count;
    }

    printf("\nTotal money: %i\n", total_money);
    printf("Total levy count: %i\n", total_levy_count);
    printf("Total payment count: %i\n", total_payment_count);
}

// Main function
int main(int argc, char **argv)
{
    // Print lock mode info
    print_lock_mode_info();

    // Set default collector count and start money
    num_collectors = NUM_COLLECTORS;
    int start_money = START_AMOUNT_OF_MONEY;
    
    // Replace if custom values given
    if (argc > 1)
    {
        num_collectors = atoi(argv[1]);
        if (argc > 2)
        {
            start_money = atoi(argv[2]);
        }
    }

    // Print setup info
    printf("Tax Collectors: %i\nAmount of money: %i\n\n", num_collectors, start_money);

    // Allocate collectors
    collectors = calloc(num_collectors, sizeof(Collector));

    int i;

    // Initialize collectors
    for (i = 0; i < num_collectors; ++i)
    {
        collectors[i].id = i;
        collectors[i].money = start_money;
    }

    // Initialize locks
    init_locks();

    // Randomize
    srand((int)time(NULL));

    // Start threads
    running = true;
    for (i = 0; i < num_collectors; ++i)
    {
        pthread_create(&collectors[i].thread, NULL, thread_collector, &collectors[i]);
    }

    // Stop threads after one second
    sleep(1);
    running = false;

    // Join threads
    for (i = 0; i < num_collectors; ++i)
    {
        pthread_join(collectors[i].thread, NULL);
    }

    // Print statistics
    print_statictics();

    // Destroy locks
    destroy_locks();

    // Free memory
    free(collectors);

    // Return success code
    return EXIT_SUCCESS;
}