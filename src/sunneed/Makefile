# Builds the main sunneed executable.

ifeq ($(origin CC),default)
CC = gcc
endif

CFLAGS ?= -Wall
SUNNEED_BUILD_PIP ?= bq27441
SUNNEED_BUILD_OUT_DIR ?= build
SUNNEED_BUILD_BIN_FILE ?= sunneed

cflags_deps = -lnng -lpthread

src_dir = src
sources = $(wildcard $(src_dir)/*.c)

out_dir = $(SUNNEED_BUILD_OUT_DIR)
bin_file = $(SUNNEED_BUILD_BIN_FILE)
pip_obj = pip.o
pip_name = $(SUNNEED_BUILD_PIP)

util_objs = $(patsubst %.c, %.o, $(wildcard $(src_dir)/util/*.c))

all: main pip util
	@echo "My work here is done..."

main: pip
	$(CC) $(CFLAGS) $(sources) $(cflags_deps) $(out_dir)/$(pip_obj) -o $(out_dir)/$(bin_file)
 
pip: $(src_dir)/pip/$(pip_name).c
	$(CC) $(CFLAGS) -c $(src_dir)/pip/$(pip_name).c -o $(out_dir)/$(pip_obj)

util: $(util_objs)

$(src_dir)/util/%.o: $(src_dir)/util/%.c
	$(CC) $(CFLAGS) -o $(patsubst $(src_dir)/util/%.o, $(out_dir)/%, $@) $^ $(cflags_deps)

.PHONY: all main pip util
