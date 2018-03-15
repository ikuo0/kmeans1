
CC := gcc
CFLAGS := -Wall -fopenmp -lm -O2

main: format.o mylog.o mempool.o exception.o array.o main.o
	$(CC) $(CFLAGS) -o $@ format.o mylog.o mempool.o exception.o array.o main.o

array.o: ../array/array.c
	$(CC) $(CFLAGS) -c $^

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
