from tenant import os,sys,json,printR,printG,find_tid,delete_tenant

def test_filter():
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
	# test_simple.c just prints and exits
	# creating the seccomp filter based on this program
	# gives an ideal minimal whitelist for testing
	os.system('gcc -o ./test_containers/simple_test/progs/test_simple ./test_progs/test_simple.c')
	os.system('cp -rp ./test_containers/simple_test /root/isochamber/new_tenants/')
	os.system('python3 tenant_config.py simple_test')

	tid = find_tid("simple_test")
	tid_list.append(tid)
	
	os.system('python3 wlister.py ' + tid +' > output.txt')

	# test filter creation
	test_filter()
	
	# try running test_seccomp with this minimal filter
	# and expect to fail
	os.system('gcc -o ./test_containers/seccomp_test/progs/test_seccomp ./test_progs/test_seccomp.c')
	os.system('cp -rp ./test_containers/seccomp_test /root/isochamber/new_tenants/')
	os.system('python3 tenant_config.py seccomp_test')
	tid = find_tid("seccomp_test")
	tid_list.append(tid)
	
	os.system('python3 handoff.py '+tid+' > output.txt')

	outdump = open("output.txt", "r")
	dump = outdump.read()

	# "Assertion" will appear if an assertion failed
	if "child_exit_status: failed" not in dump:
		printR("--- Seccomp test failed: Process not killed ---")
		sys.exit(1)#error exit


def test_capabilities():
	# whitelist test_capabilities which has two system calls
	# that are potentially harmful, this will let the power 
	# of dropping capabilities for the process even if somehow
	# a malicious syscall was added to the whitelist
	os.system('gcc -o ./test_containers/cap_test/progs/test_capabilities ./test_progs/test_capabilities.c')
	os.system('cp -rp ./test_containers/cap_test /root/isochamber/new_tenants/')
	os.system('python3 tenant_config.py cap_test')
	tid = find_tid("cap_test")
	tid_list.append(tid)

	os.system('python3 wlister.py '+tid+' > output.txt')
	os.system('python3 handoff.py '+tid+' > output.txt')

	outdump = open("output.txt", "r")
	dump = outdump.read()

	# "Assertion" will appear if an assertion failed
	if "child_exit_status: failed" in dump:
		printR("--- Capabilities test failed: Assertion failed ---")
		sys.exit(1)#error exit


def test_ipc():
	os.system('cp -rp ./test_containers/ipc_test /root/isochamber/new_tenants/')
	os.system('python3 tenant_config.py ipc_test')
	tid = find_tid("ipc_test")
	tid_list.append(tid)

	os.system('python3 wlister.py '+tid+' > output.txt')
	os.system('python3 handoff.py '+tid+' > output.txt')

	outdump = open("output.txt", "r")
	dump = outdump.read()

	# "Assertion" will appear if an assertion failed
	if "Got file handle" not in dump:
		printR("--- IPC test failed: connection refused ---")
		sys.exit(1)#error exit



tid_list = []

test_seccomp()
test_capabilities()
test_ipc()

for t in tid_list:
	if delete_tenant(t) == -1:
		sys.exit(1)

os.system('sudo rm -f output.txt')

printG("--- Testing complete! No errors to report ---")
sys.exit(0)#clean exit

