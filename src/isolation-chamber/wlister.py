import re
import os
import sys
from time import sleep
import fileinput as fi
import json


def printR(skk): print("\033[91m {}\033[00m" .format(skk)) 
def printG(skk): print("\033[92m {}\033[00m" .format(skk)) 
def printY(skk): print("\033[93m {}\033[00m" .format(skk)) 


def find_tenant(tid):
	with open('/root/isochamber/containers.json', 'r') as file:
		containers_dict = json.load(file)

	for c in containers_dict:
		# print(c['tid'])
		if(c['tid'] == tid):
			return c

	return -1

def mount_tenant(tid):
	# create container c
	c = find_tenant(tid)
	if c == -1:
		printR("--- tenant not found, tenant must be onboarded ---")
		sys.exit(1)

	if( os.system('mount -t overlay -o lowerdir=/root/isochamber/base_fs,upperdir=/root/isochamber/tenants_fs/'+tid+'/upper,workdir=/root/isochamber/tenants_fs/'+tid+'/workdir none /root/isochamber/tenants_fs/'+tid+'/overlay') != 0 ):
		printR("--- Failed to mount overlay filesystem ---")
		sys.exit(1)

	c_path = "/root/isochamber/tenants_fs/"+tid+"/overlay"    #container path
	return c_path#,p_path

def umount_tenant(c_path, tid):
	os.system('umount -f '+c_path)
	os.system('rm -rf /root/isochamber/tenants_fs/'+tid+'/upper/*')
	os.system('rm -rf /root/isochamber/tenants_fs/'+tid+'/workdir/*')


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

def check_call(syscall):
	for i in blklist:
		x = i.strip()
		if syscall == x:
			printR("-- Warning! The " + syscall + " system call is potentially dangerous!")

def check_prog(obj):
	global tid
	printY("--- Compiling and strace-ing program...\n")
	os.system('make clean && make debug')
	print()

	c_path = mount_tenant(tid)

	sleep(1)
	os.system('(sudo strace -o procdump -f ./handoff '+tid+' '+ obj + ") &")
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
		allow_calls(obj,syscalls)
	else:
		printG("\n--- Program is fully white listed! Recompiling without debug mode...")
		os.system('make clean && make')
		if( os.system('cp -f ./filter.gen.h /root/isochamber/tenants_persist/'+tid+'/filter.gen.h') !=0 ):
			printR("--- Failed to copy tenant filter ---")
			sys.exit(1)





if(len(sys.argv) < 3 ):
	printR("--- ERROR - Usage: sudo python3 wlister.py <tid> <program name>")
	sys.exit(1)

tid      = sys.argv[1]
obj_name = sys.argv[2]



blkfile = open("default_docker_blacklist.txt", "r")
blklist = blkfile.readlines()


os.system('rm -f filter.gen.h && echo //--EndOfAllows-- > filter.gen.h')
os.system('cp ' + obj_name + ' tenroot/bin/' + obj_name)




check_prog(obj_name)
