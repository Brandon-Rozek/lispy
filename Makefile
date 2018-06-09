run: prompt.c loperations.o lnumbers.o lexpressions.o lio.o lerror.o lenvironment.o mpc.o
	cc -std=c99 -Wall prompt.c loperations.o lnumbers.o lexpressions.o lio.o lerror.o lenvironment.o mpc.o -ledit -lm -o prompt
loperations.o: lval/operations.c lval/operations.h
	cc -std=c99 -Wall -c lval/operations.c -o loperations.o
lnumbers.o: lval/numbers.c lval/numbers.h
	cc -std=c99 -Wall -c lval/numbers.c -o lnumbers.o
lexpressions.o: lval/expressions.c lval/expressions.h
	cc -std=c99 -Wall -c lval/expressions.c -o lexpressions.o
lio.o: lval/io.c lval/io.h
	cc -std=c99 -Wall -c lval/io.c -o lio.o
lerror.o: lval/error.c lval/error.h
	cc -std=c99 -Wall -c lval/error.c -o lerror.o
lenvironment.o: lval/environment.c lval/environment.h
	cc -std=c99 -Wall -c lval/environment.c -o lenvironment.o
mpc.o: mpc.c mpc.h
	cc -std=c99 -Wall -lm -c mpc.c 
clean:
	rm *.o 
