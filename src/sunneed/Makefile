# Builds the main sunneed executable.

ifeq ($(origin CC),default)
	export CC = gcc
endif

CFLAGS ?= -Wall -Wextra -g
PROTOC ?= protoc-c

SUNNEED_BUILD_TYPE ?= devel
SUNNEED_BUILD_PIP ?= bq27441
SUNNEED_BUILD_OUT_DIR ?= build
SUNNEED_BUILD_BIN_FILE ?= sunneed
SUNNEED_BUILD_CLIENT_LIB_NAME ?= libsunneedclient
SUNNEED_BUILD_OVERLAY_LIB_NAME ?= sunneed_overlay

SOURCE_FORMATTER = clang-format -style=file -i

export cflags_deps = -lnng -lpthread -ldl -lprotobuf-c

ifeq ($(SUNNEED_BUILD_TYPE),devel)
	util_cflags = -Wl,-rpath,$(CURDIR)/$(clientlib_out_dir)
endif

src_dir = src
sources = $(wildcard $(src_dir)/*.c)

out_dir = $(SUNNEED_BUILD_OUT_DIR)
bin_file = $(SUNNEED_BUILD_BIN_FILE)

pip_out_dir = $(out_dir)
pip_obj = $(pip_out_dir)/pip.o
pip_name = $(SUNNEED_BUILD_PIP)

clientlib_sources = $(wildcard $(src_dir)/client/*.c)
clientlib_out_dir = $(out_dir)/client
clientlib_obj = $(clientlib_out_dir)/$(SUNNEED_BUILD_CLIENT_LIB_NAME).so

overlay_sources = $(wildcard $(src_dir)/overlay/*.c)
overlay_out_dir = $(out_dir)
overlay_obj = $(overlay_out_dir)/$(SUNNEED_BUILD_OVERLAY_LIB_NAME).so
overlay_testing_obj = $(patsubst %.so,%_testing.so,$(overlay_obj))
overlay_runner = $(out_dir)/run-with-overlay

protobuf_dir = $(src_dir)/protobuf
protobuf_sources = $(wildcard $(protobuf_dir)/*.proto)
protobuf_out_files = $(foreach src,$(protobuf_sources),$(subst !!!, ,$(join $(src:.proto=.pb-c.c!!!),$(src:.proto=.pb-c.h))))
protobuf_out_dir = $(src_dir)/protobuf/c
protobuf_out_sources = $(wildcard $(protobuf_out_dir)/*.c)

export test_home = test
export test_runner_name = run-tests
test_runner = $(test_home)/$(test_runner_name)

device_objs = $(patsubst %.c, %.o, $(wildcard $(src_dir)/device/*.c))
util_objs = $(patsubst %.c, %.o, $(wildcard $(src_dir)/util/*.c))

all: pre-all overlay main util
pre-all:
	@echo "Starting all build..."

main: protobuf pip devices
	$(call section_title,main executable)
	$(CC) $(CFLAGS) $(sources) $(protobuf_out_sources) $(cflags_deps) $(pip_obj) -o $(out_dir)/$(bin_file)
 
pip: pre-pip $(src_dir)/pip/$(pip_name).c
	$(CC) $(CFLAGS) -c $(src_dir)/pip/$(pip_name).c -o $(pip_obj)
pre-pip:
	$(call section_title,pip)

devices: pre-devices $(device_objs)
pre-devices:
	$(call section_title,devices)

util: clientlib pre-util $(util_objs)
pre-util:
	$(call section_title,utils)

$(src_dir)/util/%.o: $(src_dir)/util/%.c
	$(CC) $(CFLAGS) -o $(patsubst $(src_dir)/util/%.o, $(out_dir)/%, $@) $^ $(protobuf_out_sources) -L$(clientlib_out_dir) -lsunneedclient $(util_cflags) $(cflags_deps)

$(src_dir)/client/%.o: $(src_dir)/client/%.c
	$(CC) $(CFLAGS) -o $(patsubst $(src_dir)/util/%.o, $(out_dir)/%, $@) $^ $(protobuf_out_sources) $(cflags_deps)

$(src_dir)/device/%.o: $(src_dir)/device/%.c
	@if [ ! -d "$(out_dir)/device" ]; then mkdir "$(out_dir)/device"; fi
	$(CC) $(CFLAGS) -g -shared -o $(patsubst $(src_dir)/device/%.o, $(out_dir)/device/%.so, $@) -fPIC $^ $(cflags_deps)

protobuf: pre-protobuf $(protobuf_out_files)
	@rm -rf "$(protobuf_out_dir)" && mkdir "$(protobuf_out_dir)"
	mv $(protobuf_out_files) $(protobuf_out_dir)
pre-protobuf:
	$(call section_title,protobuf)

$(protobuf_out_files): $(protobuf_sources)
	$(MAKE) --no-print-directory -C $(protobuf_dir)

clientlib:
	$(call section_title,client library)
	@if [ ! -d "$(out_dir)/client" ]; then mkdir "$(out_dir)/client"; fi
	$(CC) $(CFLAGS) -c -fPIC -o $(out_dir)/client/clientlib.o $(clientlib_sources) $(cflags_deps)
	$(CC) $(CFLAGS) -shared -o $(clientlib_obj) $(out_dir)/client/clientlib.o

test: tests
	$(test_runner)
tests:
	make -C $(test_home)

# Note that we compile two overlay libraries. One is a tester, which contains additional output meant for
#  debugging/testing purposes.
overlay: clientlib
	$(call section_title,overlay)
	$(CC) $(CFLAGS) -g -fPIC -shared $(overlay_sources) -o $(overlay_obj)
	$(CC) $(CFLAGS) -g -fPIC -shared -DTESTING $(overlay_sources) -o $(overlay_testing_obj)
	@echo Generate overlay runscript at $(overlay_runner)
	@echo "#!/usr/bin/env bash\nLD_PRELOAD=$(abspath $(overlay_testing_obj)) "'$$1' > $(overlay_runner)
	chmod +x $(overlay_runner)

clean:
	rm -rf "$(SUNNEED_BUILD_OUT_DIR)"/*
	rm -rf "$(protobuf_out_dir)"/*
	make -C $(test_home) clean

format:
	$(SOURCE_FORMATTER) $(shell find '$(src_dir)' -not -path '$(protobuf_dir)/*' -type f -regex '.*\.[ch]')

.PHONY: all main pip util test clean format

LeftParens := (
RightParens := )

# Prints a nice little header graphic in the form:
#
# 	=======================
# 	=== Building <name> ===
# 	=======================
#
# When using in the Makefile, in simple cases you can just call this at the beginning of the target. For more
#  complicated targets, however, you will have to make a `pre-<target>` target and run that before resolving the rest
#  of the target. Remember to put your `pre-` target *after* the targets that your depends upon, but *before* the list
#  of files (if any). See the `main`, `util`, and `devices` targets for examples of usage in different situations.
section_count := 1
num_sections := $(shell grep -E '^\s*\$$\$(LeftParens)\s*call\s+section_title,' Makefile | wc -l)
define section_title
	@echo
	$(eval _var := $(section_count)/$(num_sections) $(1))
	$(eval _len := $(shell x="$(_var)"; echo -n $${#x}))
	@printf '=%.0s' $(shell seq -16 $(_len))
	@echo
	@echo === $(section_count)/$(num_sections) Building $(1) ===
	@printf '=%.0s' $(shell seq -16 $(_len))
	@echo
	$(eval section_count := $(shell expr $(section_count) + 1))
endef
