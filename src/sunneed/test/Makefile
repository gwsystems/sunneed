test_sources = $(wildcard *.c) munit/munit.c
test_headers = $(patsubst %.c, %.h, $(filter-out test_main.c, $(test_sources)))

all: $(test_headers)
	$(CC) $(CFLAGS) -o $(test_runner_name) $(test_sources) $(cflags_deps)

$(test_headers):
	./generate-test-header $(patsubst %.h, %.c, $@)

clean:
	rm $(test_runner_name)
	find . -name '*.h' -not -name 'test.h' -exec rm {} \;

.PHONY: all clean
