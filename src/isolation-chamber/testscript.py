import os
import sys


def printR(skk): print("\033[91m {}\033[00m" .format(skk)) 
def printG(skk): print("\033[92m {}\033[00m" .format(skk)) 
def printY(skk): print("\033[93m {}\033[00m" .format(skk)) 


def test_filter():
	global error
	global errCount
	default_filter   = open("simplefilter.txt", "r")
	generated_filter = open("filter.gen.h"    , "r")

	default_list = default_filter.readlines()
	gen_list     = generated_filter.readlines()

	if len(default_list) != len(gen_list):
		printR("--- Filter test failed: Generated filter is the wrong length ---")
		sys.exit(1)#error exit

	# whitelist should have same syscalls but not necessarily in same order so
	# we can't just check for equality of the files
	for i in default_list:
		if i not in gen_list:
			printR("--- Filter test failed: Missing syscall from filter ---")
			printR(i)
			sys.exit(1)#error exit



def test_seccomp():
	global error
	global errCount
	# test_simple.c just prints and exits
	# creating the seccomp filter based on this program
	# gives an ideal minimal whitelist for testing
	os.system('gcc -o testsimple test_simple.c')
	os.system('python3 wlister.py testsimple > output.txt')

	# test filter creation
	test_filter()

	# try running test_seccomp with this minimal filter
	# and expect to fail
	os.system('gcc -o testsec test_seccomp.c')
	os.system('sudo cp testsec tenroot/bin/testsec')
	os.system('sudo ./handoff testsec > output.txt')

	outdump = open("output.txt", "r")
	dump = outdump.read()

	# "Assertion" will appear if an assertion failed
	if "child_exit_status: 0" not in dump:
		printR("--- Seccomp test failed: Process not killed ---")
		sys.exit(1)#error exit


def test_capabilities():
	global error
	global errCount
	# whitelist test_capabilities which has two system calls
	# that are potentially harmful, this will let the power 
	# of dropping capabilities for the process even if somehow
	# a malicious syscall was added to the whitelist
	os.system('gcc -o testcap test_capabilities.c')
	os.system('python3 wlister.py testcap > output.txt')
	os.system('sudo ./handoff testcap > output.txt')

	outdump = open("output.txt", "r")
	dump = outdump.read()

	# "Assertion" will appear if an assertion failed
	if "child_exit_status: 0" in dump:
		printR("--- Capabilities test failed: Assertion failed ---")
		sys.exit(1)#error exit


test_seccomp()
test_capabilities()

os.system('sudo rm -f output.txt')

printG("--- Testing complete! No errors to report ---")
sys.exit(0)#clean exit

