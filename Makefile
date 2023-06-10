CFLAGS = -g -Wall -Werror -Wpedantic

all:
	$(CC) test.c $(CFLAGS) -o test
.PHONY:  all
