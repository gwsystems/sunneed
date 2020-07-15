import os

error = 0
errCount = 0


def printR(skk): print("\033[91m {}\033[00m" .format(skk)) 
def printG(skk): print("\033[92m {}\033[00m" .format(skk)) 
def printY(skk): print("\033[93m {}\033[00m" .format(skk)) 

def test_seccomp():
	global error
	global errCount
	# test_simple.c just prints and exits
	# creating the seccomp filter based on this program
	# gives an ideal minimal whitelist for testing
	os.system('gcc -o testsimple test_simple.c')
	os.system('python3 wlister.py testsimple > output.txt')

	# try running test_seccomp with this minimal filter
	# and expect to fail
	os.system('gcc -o testsec test_seccomp.c')
	os.system('sudo cp testsec tenroot/bin/testsec')
	# os.system('sudo strace -o procdump_testsec -e trace=socket ./handoff testsec > output1.txt')

	# pdump   = open("procdump_testsec", "r")
	# dump = pdump.read()

	# if "CLD_KILLED" not in dump:
	# 	printR("--- Seccomp test failed: Process not killed ---")
	# 	error = 1
	# 	errCount += 1
	os.system('sudo ./handoff testsec > output.txt')

	outdump = open("output.txt", "r")
	dump = outdump.read()

	# "Assertion" will appear if an assertion failed
	if "child_exit_status: 0" not in dump:
		printR("--- Seccomp test failed: Process not killed ---")
		error = 1
		errCount += 1


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
		error = 1
		errCount += 1


test_seccomp()
test_capabilities()


if error:
	printR("--- Testing complete... " + str(errCount) + " errors found ---")
else:
	printG("--- Testing complete! No errors to report ---")

os.system('sudo rm -f output.txt')