
CC := gcc
CFLAGS := -Wall -fopenmp -O2
LFLAGS := -lm

main: format.o mylog.o mempool.o exception.o main.o
	$(CC) $(CFLAGS) -o $@ format.o mylog.o mempool.o exception.o main.o $(LFLAGS)

exception.o: ../exception/exception.c
	$(CC) $(CFLAGS) -c $^

mempool.o: ../mempool/mempool.c
	$(CC) $(CFLAGS) -c $^

mylog.o: ../mylog/mylog.c
	$(CC) $(CFLAGS) -c $^

format.o: ../format/format.c
	$(CC) $(CFLAGS) -c $^

main.o: main.c
	$(CC) $(CFLAGS) -c $^

clean:
	rm -f main *.o
