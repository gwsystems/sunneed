import os
import sys
import json

def printR(skk): print("\033[91m {}\033[00m" .format(skk)) 

def find_tenant(tid):
	with open('containers.json', 'r') as file:
		containers_dict = json.load(file)

	for c in containers_dict:
		if(c['tid'] == tid):
			return c

	return -1

def mount_tenant(tid):
	# create container c
	c = find_tenant(tid)
	if c == -1:
		printR("--- tenant not found, tenant must be onboarded ---")
		sys.exit(1)

	if( os.system('mount -t overlay -o lowerdir=/opt/isochamber/base_fs,upperdir=/opt/isochamber/tenants_fs/'+tid+'/upper,workdir=/opt/isochamber/tenants_fs/'+tid+'/workdir none /opt/isochamber/tenants_fs/'+tid+'/overlay') != 0 ):
		printR("--- Failed to mount overlay filesystem ---")
		sys.exit(1)

	c_path = "/opt/isochamber/tenants_fs/"+tid+"/overlay"    #container path
	#p_path = "/opt/isochamber/tenants_persist/"+tid+"/home"	 #persistent data path
	return c_path#,p_path

def umount_tenant(c_path, tid):
	os.system('umount -f '+c_path)
	os.system('rm -rf /opt/isochamber/tenants_fs/'+tid+'/upper/*')
	os.system('rm -rf /opt/isochamber/tenants_fs/'+tid+'/workdir/*')


if(len(sys.argv) < 3):
	printR("--- ERROR - Usage: sudo python3 handoff.py <tenant_id(tid)> <program name> ---")
	sys.exit(1)

tid      = sys.argv[1]
obj_name = sys.argv[2]




# set up overlay fs - MAYBE THIS SHOULD BE WITH ONBOARRDING 
#						this way whole fs is persistent 
#						instead of limiting it to /home
# if( os.system('mkdir /opt/isochamber/'+tid+'/ && \
# 			   mkdir /opt/isochamber/'+tid+'/upper && \
# 			   mkdir /opt/isochamber/'+tid+'/workdir && \
# 			   mkdir /opt/isochamber/'+tid+'/overlay') != 0 ):
# 	printR("--- Failed to build overlay filesystem ---")
# 	sys.exit(1)

c_path= mount_tenant(tid)
# os.system('sudo cp -rfp /opt/isochamber/tenants_persist/'+tid+'/home/ '+c_path+"/")

if( os.system('./handoff '+tid+" "+obj_name) != 0 ):
	printR("--- handoff.c failed ---")
	# sys.exit(1)

umount_tenant(c_path,tid)
