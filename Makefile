run: prompt.c mpc.o
	cc -std=c99 -Wall prompt.c mpc.o -ledit -lm -o prompt
mpc:o mpc.c mpc.h
	cc -std=c99 -Wall -lm -c mpc.c 
clean:
	rm *.o 
