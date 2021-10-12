CC = gcc
CFLAGS = -Wall -Werror -Wextra -pedantic -std=c99
SRC = systime.c
SRC2 = contime.c

all: systime contime

systime: $(SRC)
	$(CC) -o systime $(CFLAGS) $(SRC)

contime: $(SRC2)
	$(CC) -o contime $(CFLAGS) $(SRC2)
	
clean:
	rm -f systime contime
