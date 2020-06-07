test_sources = $(wildcard *.c) munit/munit.c
suite_sources = $(filter-out test_main.c, $(wildcard *.c))
suites = $(patsubst %.c,%,$(suite_sources))

glue_header = test.gen.h

all: $(glue_header)
	$(CC) $(CFLAGS) -o $(test_runner_name) $(test_sources) $(cflags_deps)

$(glue_header):
	./scripts/generate-test-header $(suites) > $@

clean:
	rm $(glue_header)
	rm $(test_runner_name)

.PHONY: all clean
