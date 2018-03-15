
CC := gcc
CFLAGS := -DARRAY_UNITTEST
LFLAGS := -lm

array: format.o mylog.o mempool.o exception.o array.o
	$(CC) $(CFLAGS) -o $@ format.o mylog.o mempool.o exception.o array.o $(LFLAGS)

exception.o: ../exception/exception.c
	$(CC) $(CFLAGS) -c $^

mempool.o: ../mempool/mempool.c
	$(CC) $(CFLAGS) -c $^

mylog.o: ../mylog/mylog.c
	$(CC) $(CFLAGS) -c $^

format.o: ../format/format.c
	$(CC) $(CFLAGS) -c $^

array.o: array.c
	$(CC) $(CFLAGS) -c $^

clean:
	rm -f array *.o
