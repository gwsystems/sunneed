PROTOC ?= protoc-c 

protobuf_sources = $(wildcard *.proto)
protobuf_out_files = $(foreach src,$(protobuf_sources),$(subst !!!, ,$(join $(src:.proto=.pb-c.c!!!),$(src:.proto=.pb-c.h))))

$(protobuf_out_files): $(protobuf_sources)
	$(PROTOC) --c_out=. $(patsubst $(protobuf_dir)/%,./%,$^)
