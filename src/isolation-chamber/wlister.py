import re
import os
import sys
from time import sleep
import fileinput as fi
from tenant import mount_tenant, umount_tenant, printR, printG, printY

# allow calls by adding system calls to filter file
# return:
#		no return value
def allow_calls(syscalls):
	printY("\n--- White listing system calls...\n")
	allowstring = ""

	for i in syscalls:
		allowstring = allowstring + i

	fn = './filter.gen.h'

	for line in fi.FileInput(fn, inplace = 1):
		if '//--EndOfAllows--' in line:
			line = line.replace(line, allowstring + line)

		print(line, end='')

	check_prog() # recursively keep checking program

# check to see if program is malicious by comparing
# against default Docker blacklist
# (Right now this only prints a warning but fail/error
# logic could be added here)
# return:
# 		no return value
def check_call(syscall):
	for i in blklist:
		x = i.strip()
		if syscall == x:
			printR("-- Warning! The " + syscall + " system call is potentially dangerous!")

# function recursively compiles and straces program
# until the program stops trapping and it is fully
# whitelisted
# return:
#		no return value
def check_prog():
	global tid
	printY("--- Compiling and strace-ing program...\n")
	os.system('make clean && make debug')
	print()

	c_path,c_init = mount_tenant(tid)

	sleep(1)
	os.system('(sudo strace -o procdump -f ./handoff '+tid+' '+ c_init + ") &")
	sleep(5)

	umount_tenant(c_path,tid)

	printY("--- Attempt to killall strace processes incase they are hung")
	os.system('sudo killall -9 strace')

	pdump  = open("procdump","r")

	syssearch = re.compile(r'si_syscall=__NR_([a-zA-Z0-9_]*),')
	syssearch_arm = re.compile(r'si_syscall=__ARM_NR_([a-zA-Z0-9_]*),')

	syscalls = set() #use set to avoid duplicate syscalls

	contents = pdump.readlines()

	trapflag = False

	for i in contents:
		if re.search("si_signo=SIGSYS", i):
			trapflag = True
			searchresult = syssearch.search(i)
			armresult    = syssearch_arm.search(i)
			if searchresult != None:
				syscalls.add("\tALLOW(" + searchresult.group(1) + "),\n")
				check_call(searchresult.group(1))
			else:
				syscalls.add("\tALLOW_ARM(" + armresult.group(1) + "),\n")

	if trapflag == True:
		allow_calls(syscalls) # this is essentially recursion (allow_calls calls check_prog)
	else:
		printG("\n--- Program is fully white listed! Recompiling without debug mode...")
		os.system('make clean && make')
		if( os.system('cp -f ./filter.gen.h /root/isochamber/tenants_persist/'+tid+'/filter.gen.h') !=0 ):
			printR("--- Failed to copy tenant filter ---")
			sys.exit(1)
		# TODO: change this hard copy filter process to one that serializes different tenant filters
		# 		into a file to avoid recompilation during handoff.py

# --------------------------------------------------------------

# PROGRAM START
if(len(sys.argv) < 2 ):
	printR("--- ERROR - Usage: sudo python3 wlister.py <tid>")
	sys.exit(1)

tid      = sys.argv[1]

blkfile = open("default_docker_blacklist.txt", "r")
blklist = blkfile.readlines()

os.system('rm -f filter.gen.h && echo //--EndOfAllows-- > filter.gen.h')

check_prog()
