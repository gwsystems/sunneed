CC = gcc
CFLAGS = -g -Wall -Werror
CFLAGS_LIB = -shared -fPIC

SO_NAME = libbq27441.so
TESTER_NAME = tester

SOURCES = bq27441.c
TESTER_SOURCES = tester.c

LINK_DEPS = -li2c
TESTER_DEPS = -lbq27441

lib:
	$(CC) $(LINK_DEPS) $(CFLAGS) $(CFLAGS_LIB) $(SOURCES) -o $(SO_NAME)

tester: lib
	$(CC) $(CFLAGS) $(LINK_DEPS) $(TESTER_DEPS) $(TESTER_SOURCES) $(SO_NAME) -o $(TESTER_NAME)
	@echo 'Running tester at ./$(TESTER_NAME)'	
	@LD_LIBRARY_PATH=$(shell pwd) ./$(TESTER_NAME)
