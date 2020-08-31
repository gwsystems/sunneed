import os
import sys
import json

# functions useful for colorized output
def printR(skk): print("\033[91m {}\033[00m" .format(skk)) 
def printG(skk): print("\033[92m {}\033[00m" .format(skk)) 
def printY(skk): print("\033[93m {}\033[00m" .format(skk)) 

# function takes tid and verifies that it is configured
# return:
# 	success: tenant's respective json entry
#	failure: -1
def find_tenant(tid):
	with open('/root/isochamber/containers.json', 'r') as file:
		containers_dict = json.load(file)

	for c in containers_dict:
		if(c['tid'] == tid):
			return c

	return -1

# function takes container name and returns tid
# return:
#		success: tid
# 		failure: none - exit with error
def find_tid(cname):
	with open('/root/isochamber/containers.json', 'r') as file:
		containers_dict = json.load(file)

	for c in containers_dict:
		if(c['cname'] == cname):
			return c['tid']
	printR("tenant wasn't configured properly")
	sys.exit(1)


# This function is important as it is what mounts the tenant's
# root filesystem (.../overlay/)
# return:
#		success: c_path, c_init
# 		failure: none - exit with error
def mount_tenant(tid):
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


# This function only differs from above in that
# it returns the location of the dependencies 
# script rather than the init program
# return:
#		success: c_path, c_init
# 		failure: none - exit with error
def mount_tenant_config(tid):
	c = find_tenant(tid)
	if c == -1:
		printR("--- tenant not found, tenant must be onboarded ---")
		sys.exit(1)

	if( os.system('mount -t overlay -o lowerdir=/root/isochamber/base_fs,upperdir=/root/isochamber/tenants_fs/'+tid+'/upper,workdir=/root/isochamber/tenants_fs/'+tid+'/workdir none /root/isochamber/tenants_fs/'+tid+'/overlay') != 0 ):
		printR("--- Failed to mount overlay filesystem ---")
		sys.exit(1)

	c_path = "/root/isochamber/tenants_fs/"+tid+"/overlay"    #container path
	c_init = c['dependencies']
	return c_path,c_init


# unmount the tenant filesystem
# return:
# 		no return value
def umount_tenant(c_path, tid):
	os.system('umount -f '+c_path)
	# NOTE: below commented out lines were for deleting contents
	# of tenant fs on umount, however due to dependencies needing
	# to be downloaded, we are just keeping the full tenant fs
	# persistant for now. If future design changes, this logic 
	# should go here. 
	# os.system('rm -rf /root/isochamber/tenants_fs/'+tid+'/upper/*')
	# os.system('rm -rf /root/isochamber/tenants_fs/'+tid+'/workdir/*')


# uninstall tenant container and all records of it
# return:
# 		success:  0
# 		failure: -1
def delete_tenant(tid):
	with open('/root/isochamber/containers.json', 'r') as file:
		containers_dict = json.load(file)

	deleted = 0
	for c in containers_dict:
		if(c['tid'] == tid):
			containers_dict.remove(c)
			update = open('/root/isochamber/containers.json', 'w') 
			json.dump(containers_dict, update, indent=4)
			update.close()

			deleted = 1
			continue
	if deleted == 0:
		printR("--- tenant not found, cannot delete ---")
		return -1

	os.system('rm -rf /root/isochamber/tenants_fs/'+tid)
	os.system('rm -rf /root/isochamber/tenants_persist/'+tid)

	return 0