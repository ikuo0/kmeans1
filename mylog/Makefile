
CC := gcc
CFLAGS := -DMYLOG_UNITTEST -Wformat-security

mylog: mylog.o
	$(CC) $(CFLAGS) -o $@ mylog.o

mylog.o: mylog.c
	$(CC) $(CFLAGS) -c $^

clean:
	rm -f mempool *.o
