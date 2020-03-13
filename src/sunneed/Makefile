# Builds the main sunneed executable.

ifeq ($(origin CC),default)
CC = gcc
endif

CFLAGS ?= -Wall
SUNNEED_BUILD_PIP ?= bq27441
SUNNEED_BUILD_OUT_DIR ?= build
SUNNEED_BUILD_BIN_FILE ?= sunneed

cflags_deps = -lnng -lpthread -ldl

src_dir = src
sources = $(wildcard $(src_dir)/*.c)

out_dir = $(SUNNEED_BUILD_OUT_DIR)
bin_file = $(SUNNEED_BUILD_BIN_FILE)
pip_obj = pip.o
pip_name = $(SUNNEED_BUILD_PIP)

device_objs = $(patsubst %.c, %.o, $(wildcard $(src_dir)/device/*.c))
util_objs = $(patsubst %.c, %.o, $(wildcard $(src_dir)/util/*.c))

all: main util
	@echo "My work here is done..."

main: pip devices
	$(CC) $(CFLAGS) $(sources) $(cflags_deps) $(out_dir)/$(pip_obj) -o $(out_dir)/$(bin_file)
 
pip: $(src_dir)/pip/$(pip_name).c
	$(CC) $(CFLAGS) -c $(src_dir)/pip/$(pip_name).c -o $(out_dir)/$(pip_obj)

devices: $(device_objs)
util: $(util_objs)

$(src_dir)/util/%.o: $(src_dir)/util/%.c
	$(CC) $(CFLAGS) -o $(patsubst $(src_dir)/util/%.o, $(out_dir)/%, $@) $^ $(cflags_deps)

$(src_dir)/device/%.o: $(src_dir)/device/%.c
	@if [ ! -d "$(out_dir)/device" ]; then mkdir "$(out_dir)/device"; fi
	$(CC) $(CFLAGS) -c -fPIC -o $(patsubst $(src_dir)/device/%.o, $(out_dir)/device/%.o, $@) $^ $(cflags_deps)
	$(CC) $(CFLAGS) -shared -o $(patsubst $(src_dir)/device/%.o, $(out_dir)/device/%.so, $@) $(cflags_deps)

.PHONY: all main pip util
