CC=gcc
CFLAGS=-Wall -Wextra -std=gnu11 -g -lpthread

all: dlx_mpi.out clean-objs

dlx_mpi.out: dlx_mpi.o dlx.o parser.o stack.o dlink.o
	mpicc $(CFLAGS) -o dlx_mpi.out dlx_mpi.o dlx.o parser.o stack.o dlink.o

dlx_mpi.o : dlx_mpi.c parser.h stack.h dlink.h
	mpicc $(CFLAGS) -c dlx_mpi.c

dlx.o: dlx.c parser.h stack.h dlink.h
	$(CC) $(CFLAGS) -c dlx.c

parser.o: parser.c dlink.h
	$(CC) $(CFLAGS) -c parser.c

stack_mwe.out: stack_mwe.o stack.o
	$(CC) $(CFLAGS) -o stack_mwe.out stack_mwe.o stack.o

stack_mwe.o: stack_mwe.c stack.h
	$(CC) $(CFLAGS) -c stack_mwe.c

stack.o: stack.c stack.h
	$(CC) $(CFLAGS) -c stack.c

dlink_mwe.out: dlink_mwe.o dlink.o
	$(CC) $(CFLAGS) -o dlink_mwe.out dlink_mwe.o dlink.o

dlink_mwe.o: dlink_mwe.c dlink.h
	$(CC) $(CFLAGS) -c dlink_mwe.c

dlink.o: dlink.c dlink.h
	$(CC) $(CFLAGS) -c dlink.c

clean-objs: 
	rm -f *.o

clean: clean-objs
	rm -f *.out