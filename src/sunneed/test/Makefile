test_sources = $(wildcard *.c) munit/munit.c

all:
	$(CC) $(CFLAGS) -o $(test_runner_name) $(test_sources) $(cflags_deps)

clean:
	rm $(test_runner_name)

.PHONY: all clean
