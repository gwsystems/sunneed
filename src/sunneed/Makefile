# Builds the main sunneed executable.

CC = gcc
CFLAGS = -Wall
CFLAGS_DEPS = -lnng -lpthread

CFLAGS_LIB = -c

SRC_DIR = src
SOURCES = $(wildcard $(SRC_DIR)/*.c)

SRC_UTIL_DIR = $(SRC_DIR)/util

OUT_DIR = build
BIN_NAME = sunneed
PIP_NAME = pip

SUNNEED_BUILD_PIP ?= bq27441

UTIL_OBJS = $(patsubst %.c, %.o, $(wildcard $(SRC_UTIL_DIR)/*.c))

all: main pip util

main:
	gcc $(CFLAGS) $(SOURCES) $(CFLAGS_DEPS) $(OUT_DIR)/$(PIP_NAME).o -o $(OUT_DIR)/$(BIN_NAME)
 
pip: $(SRC_DIR)/pip/$(SUNNEED_BUILD_PIP).c
	gcc $(CFLAGS) $(CFLAGS_LIB) $(SRC_DIR)/pip/$(SUNNEED_BUILD_PIP).c -o $(OUT_DIR)/$(PIP_NAME).o

util: $(UTIL_OBJS)

$(SRC_UTIL_DIR)/%.o: $(SRC_UTIL_DIR)/%.c
	$(CC) $(CFLAGS) -o $(patsubst $(SRC_UTIL_DIR)/%.o, build/%, $@) $^ $(CFLAGS_DEPS)

.PHONY: all main pip util
