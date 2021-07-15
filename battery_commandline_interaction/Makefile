CC = gcc
CFLAGS = -g -Wall -Werror
SOURCES = battery_info.c
OBJ = battery_info
PREFIX = /usr/local/bin

LINK_DEPS = -I/usr/local/lib -I/usr/local/include -li2c -lbq27441

all: battery_info

battery_info:
	$(CC) $(CFLAGS) -o $(OBJ) $(SOURCES) $(LINK_DEPS)

install: battery_info
	cp $(OBJ) $(PREFIX)

uninstall:
	rm $(PREFIX)/$(OBJ)

clean:
	rm -f $(OBJ)
