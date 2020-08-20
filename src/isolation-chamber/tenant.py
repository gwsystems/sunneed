import os
import sys
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
	c_init = c['init']
	return c_path,c_init

def umount_tenant(c_path, tid):
	os.system('umount -f '+c_path)
	os.system('rm -rf /root/isochamber/tenants_fs/'+tid+'/upper/*')
	os.system('rm -rf /root/isochamber/tenants_fs/'+tid+'/workdir/*')