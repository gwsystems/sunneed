# Builds the main sunneed executable.

CC = gcc
CFLAGS = -Wall
CFLAGS_LIB = -c

SRC_DIR = src
SOURCES = $(wildcard $(SRC_DIR)/*.c)

OUT_DIR = build
BIN_NAME = sunneed
PIP_NAME = pip

SUNNEED_BUILD_PIP ?= bq27441

all: pip
	gcc $(CFLAGS) $(SOURCES) $(OUT_DIR)/$(PIP_NAME).o -o $(OUT_DIR)/$(BIN_NAME)
 
pip:
	gcc $(CFLAGS) $(CFLAGS_LIB) $(SRC_DIR)/pip/$(SUNNEED_BUILD_PIP).c -o $(OUT_DIR)/$(PIP_NAME).o
