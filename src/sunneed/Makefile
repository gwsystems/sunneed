# Builds the main sunneed executable.

ifeq ($(origin CC),default)
CC = gcc
endif

CFLAGS ?= -Wall
PROTOC ?= protoc-c

SUNNEED_BUILD_PIP ?= bq27441
SUNNEED_BUILD_OUT_DIR ?= build
SUNNEED_BUILD_BIN_FILE ?= sunneed

cflags_deps = -lnng -lpthread -ldl

src_dir = src
sources = $(wildcard $(src_dir)/*.c)
clientlib_sources = $(wildcard $(src_dir)/client/*.c)

out_dir = $(SUNNEED_BUILD_OUT_DIR)
bin_file = $(SUNNEED_BUILD_BIN_FILE)
pip_obj = pip.o
pip_name = $(SUNNEED_BUILD_PIP)

protobuf_dir = $(src_dir)/protobuf
protobuf_sources = $(wildcard $(protobuf_dir)/*.proto)
protobuf_out_files = $(foreach src,$(protobuf_sources),$(subst !!!, ,$(join $(src:.proto=.pb-c.c!!!),$(src:.proto=.pb-c.h))))
protobuf_out_dir = $(src_dir)/protobuf/c

device_objs = $(patsubst %.c, %.o, $(wildcard $(src_dir)/device/*.c))
util_objs = $(patsubst %.c, %.o, $(wildcard $(src_dir)/util/*.c))

all: main util clientlib
	@echo "My work here is done..."

main: protobuf pip devices
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

protobuf: $(protobuf_out_files)
	@rm -rf "$(protobuf_out_dir)" && mkdir "$(protobuf_out_dir)"
	mv $(protobuf_out_files) $(protobuf_out_dir)

$(protobuf_out_files): $(protobuf_sources)
	@$(MAKE) --no-print-directory -C $(protobuf_dir)

clientlib:
	@if [ ! -d "$(out_dir)/client" ]; then mkdir "$(out_dir)/client"; fi
	$(CC) $(CFLAGS) -c -fPIC -o $(out_dir)/client/clientlib.o $(clientlib_sources) $(cflags_deps)
	$(CC) $(CFLAGS) -shared -o $(out_dir)/client/libsunneedclient.so $(out_dir)/client/clientlib.o

clean:
	rm -rf "$(SUNNEED_BUILD_OUT_DIR)"/*
	rm -rf "$(protobuf_out_dir)"/*

.PHONY: all main pip util clean
