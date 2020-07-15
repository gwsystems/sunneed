CC=gcc
CFLAGS=-lcap

handoff: handoff.c
	$(CC) handoff.c -o handoff  $(CFLAGS)

debug: handoff.c
	$(CC) -DDEBUG=1 handoff.c -o handoff $(CFLAGS)

clean:
	rm -f handoff
