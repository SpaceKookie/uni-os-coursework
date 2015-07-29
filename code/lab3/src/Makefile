all:
	gcc sar_shell.c sar_lib.c sar_backgroundjob.c sar_call.c -o sar_shell
	gcc sar_prog.c -o sar_prog

clean: 
	-rm -f *.o sar_shell sar_prog

pack:
	tar cvf ../lab-3.tar ../src
