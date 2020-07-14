import re
import os
import sys
from time import sleep
import fileinput as fi

def printR(skk): print("\033[91m {}\033[00m" .format(skk)) 
def printG(skk): print("\033[92m {}\033[00m" .format(skk)) 
def printY(skk): print("\033[93m {}\033[00m" .format(skk)) 


def allow_calls(obj,syscalls):
	printY("\n--- White listing system calls...\n")
	allowstring = ""

	for i in syscalls:
		allowstring = allowstring + i

	fn = './filter.gen.h'

	for line in fi.FileInput(fn, inplace = 1):
		if '//--EndOfAllows--' in line:
			line = line.replace(line, allowstring + line)

		print(line, end='')

	check_prog(obj)



def check_prog(obj):
	printY("--- Compiling and strace-ing program...\n")
	os.system('make clean && make')
	print()
	os.system('(sudo strace -o procdump -f ./handoff ' + obj + ") &")

	sleep(5)

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
			else:
				syscalls.add("\tALLOW_ARM(" + armresult.group(1) + "),\n")

	if trapflag == True:
		allow_calls(obj,syscalls)
	else:
		printG("\n--- Program is fully white listed!")






# print('n arguments: ', len(sys.argv))
# print('args: ', sys.argv[1])
if(len(sys.argv) < 2 ):
	printR("--- ERROR - Usage: python3 wlister.py <program name>")
	sys.exit(0)

obj_name = sys.argv[1]
# print(obj_name)


check_prog(obj_name)
