CC = gcc
CFLAGS = -Wall -Werror -Wextra -pedantic -std=c99
SRC = systime.c

systime: $(SRC)
	$(CC) -o systime $(CFLAGS) $(SRC)
	
clean:
	rm -f systime
