#tcd : Tax Collectors Dilemma

#CFLAGS=-Wall -Werror -g
CFLAGS=-g -Wall

default:
	gcc -lpthread $(CFLAGS) tcd.c -o tcd

biglock:
	gcc -lpthread $(CFLAGS) -DBIGLOCK tcd.c -o tcd_biglock

nolock:
	gcc -lpthread $(CFLAGS) -DNOLOCK tcd.c -o tcd_nolock

clean:
	rm -f tcd tcd_biglock tcd_nolock

pack: clean
	tar cvf ../lab-5.tar ../src


