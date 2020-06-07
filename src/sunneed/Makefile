# Builds the main sunneed executable.

ifeq ($(origin CC),default)
CC = gcc
endif

CFLAGS ?= -Wall -Wextra -g
PROTOC ?= protoc-c

SUNNEED_BUILD_TYPE ?= devel
SUNNEED_BUILD_PIP ?= bq27441
SUNNEED_BUILD_OUT_DIR ?= build
SUNNEED_BUILD_BIN_FILE ?= sunneed
SUNNEED_BUILD_CLIENT_LIB_NAME ?= libsunneedclient

SOURCE_FORMATTER = clang-format -style=file -i

cflags_deps = -lnng -lpthread -ldl -lprotobuf-c

ifeq ($(SUNNEED_BUILD_TYPE),devel)
	util_cflags = -Wl,-rpath,$(CURDIR)/$(clientlib_out_dir)
endif

src_dir = src
sources = $(wildcard $(src_dir)/*.c)

# TODO Consistent naming on all this stuff.
out_dir = $(SUNNEED_BUILD_OUT_DIR)
bin_file = $(SUNNEED_BUILD_BIN_FILE)
pip_obj = pip.o
pip_name = $(SUNNEED_BUILD_PIP)

clientlib_sources = $(wildcard $(src_dir)/client/*.c)
clientlib_file = $(SUNNEED_BUILD_CLIENT_LIB_NAME).so
clientlib_out_dir = $(out_dir)/client
clientlib_obj = $(clientlib_out_dir)/$(clientlib_file)

overlay_sources = $(wildcard $(src_dir)/overlay/*.c)
overlay_library_file = sunneed_overlay.so
overlay_library_testing_file = sunneed_overlay_test.so
overlay_runner = $(out_dir)/run-with-overlay

protobuf_dir = $(src_dir)/protobuf
protobuf_sources = $(wildcard $(protobuf_dir)/*.proto)
protobuf_out_files = $(foreach src,$(protobuf_sources),$(subst !!!, ,$(join $(src:.proto=.pb-c.c!!!),$(src:.proto=.pb-c.h))))
protobuf_out_dir = $(src_dir)/protobuf/c
protobuf_out_sources = $(wildcard $(protobuf_out_dir)/*.c)

device_objs = $(patsubst %.c, %.o, $(wildcard $(src_dir)/device/*.c))
util_objs = $(patsubst %.c, %.o, $(wildcard $(src_dir)/util/*.c))

all: overlay main util
	@echo "My work here is done..."

main: protobuf pip devices
	$(CC) $(CFLAGS) $(sources) $(protobuf_out_sources) $(cflags_deps) $(out_dir)/$(pip_obj) -o $(out_dir)/$(bin_file)
 
pip: $(src_dir)/pip/$(pip_name).c
	$(CC) $(CFLAGS) -c $(src_dir)/pip/$(pip_name).c -o $(out_dir)/$(pip_obj)

devices: $(device_objs)
util: clientlib $(util_objs)

$(src_dir)/util/%.o: $(src_dir)/util/%.c
	$(CC) $(CFLAGS) -o $(patsubst $(src_dir)/util/%.o, $(out_dir)/%, $@) $^ $(protobuf_out_sources) -L$(clientlib_out_dir) -lsunneedclient $(util_cflags) $(cflags_deps)

$(src_dir)/client/%.o: $(src_dir)/client/%.c
	$(CC) $(CFLAGS) -o $(patsubst $(src_dir)/util/%.o, $(out_dir)/%, $@) $^ $(protobuf_out_sources) $(cflags_deps)

$(src_dir)/device/%.o: $(src_dir)/device/%.c
	@if [ ! -d "$(out_dir)/device" ]; then mkdir "$(out_dir)/device"; fi
	$(CC) $(CFLAGS) -g -shared -o $(patsubst $(src_dir)/device/%.o, $(out_dir)/device/%.so, $@) -fPIC $^ $(cflags_deps)

protobuf: $(protobuf_out_files)
	@rm -rf "$(protobuf_out_dir)" && mkdir "$(protobuf_out_dir)"
	mv $(protobuf_out_files) $(protobuf_out_dir)

$(protobuf_out_files): $(protobuf_sources)
	@$(MAKE) --no-print-directory -C $(protobuf_dir)

clientlib:
	@if [ ! -d "$(out_dir)/client" ]; then mkdir "$(out_dir)/client"; fi
	$(CC) $(CFLAGS) -c -fPIC -o $(out_dir)/client/clientlib.o $(clientlib_sources) $(cflags_deps)
	$(CC) $(CFLAGS) -shared -o $(clientlib_obj) $(out_dir)/client/clientlib.o

# Note that we compile two overlay libraries. One is a tester, which contains additional output meant for
#  debugging/testing purposes.
overlay: clientlib
	$(CC) $(CFLAGS) -g -fPIC -shared $(overlay_sources) -o $(out_dir)/$(overlay_library_file)	
	$(CC) $(CFLAGS) -g -fPIC -shared -DTESTING $(overlay_sources) -o $(out_dir)/$(overlay_library_testing_file)	
	@echo Generate overlay runscript at $(overlay_runner)
	@echo "#!/usr/bin/env bash\nLD_PRELOAD=$(abspath $(out_dir)/$(overlay_library_testing_file)) "'$$1' > $(overlay_runner)
	chmod +x $(overlay_runner)

clean:
	rm -rf "$(SUNNEED_BUILD_OUT_DIR)"/*
	rm -rf "$(protobuf_out_dir)"/*

format:
	$(SOURCE_FORMATTER) $(shell find '$(src_dir)' -not -path '$(protobuf_dir)/*' -type f -regex '.*\.[ch]')

.PHONY: all main pip util clean format
