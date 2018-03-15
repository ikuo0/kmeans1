
CC := gcc
CFLAGS := -DMEMPOOL_UNITTEST

mempool: exception.o mempool.o
	$(CC) $(CFLAGS) -o $@ exception.o mempool.o

exception.o: ../exception/exception.c
	$(CC) $(CFLAGS) -c $^

mempool.o: mempool.c
	$(CC) $(CFLAGS) -c $^

clean:
	rm -f mempool *.o
