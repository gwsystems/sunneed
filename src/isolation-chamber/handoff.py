import os
import sys
import json

def printR(skk): print("\033[91m {}\033[00m" .format(skk)) 

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


if(len(sys.argv) < 3):
	printR("--- ERROR - Usage: sudo python3 handoff.py <tenant_id(tid)> <program name> ---")
	sys.exit(1)

tid      = sys.argv[1]
obj_name = sys.argv[2]



if( os.system('cp -f /root/isochamber/tenants_persist/'+tid+'/filter.gen.h ./filter.gen.h') !=0 ):
	printR("--- Failed to copy tenant filter ---")
	sys.exit(1)

if( os.system('make clean && make') !=0 ):
	printR("--- Failed to compile handoff.c ---")
	sys.exit(1)


c_path= mount_tenant(tid)


if( os.system('./handoff '+tid+" "+obj_name) != 0 ):
	printR("--- handoff.c failed ---")
	# sys.exit(1)

umount_tenant(c_path,tid)
