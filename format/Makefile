
CC := gcc
CFLAGS := -DFORMAT_UNITTEST -Wformat-security -Wall

format: format.o
	$(CC) $(CFLAGS) -o $@ format.o

format.o: format.c
	$(CC) $(CFLAGS) -c $^

clean:
	rm -f format *.o
