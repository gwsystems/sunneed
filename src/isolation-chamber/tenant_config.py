import os
import sys
import uuid
import json
from tenant import printR, printG, printY, mount_tenant_config, umount_tenant

# CONFIGURE TENANT CONTAINER

argn = len(sys.argv)
if(argn < 2 ):
	printR("--- ERROR - Usage: \nsudo python3 tenant_config.py  <tenant's cname>")
	sys.exit(1)

cname = sys.argv[1]
ten_dir = "/root/isochamber/new_tenants/"+cname

id = uuid.uuid1()
tid = id.hex

ten_config = ten_dir + "/config.json"



with open('/root/isochamber/containers.json', 'r') as file:
		containers_dict = json.load(file)

with open(ten_config, 'r') as file:
		config_dict = json.load(file)

config_dict['tid'] = tid

# tenant's config entry to containers.json
containers_dict.append(config_dict)
update = open('/root/isochamber/containers.json', 'w') 
json.dump(containers_dict, update, indent=4)
update.close()


# set up overlay fs 
if( os.system('mkdir /root/isochamber/tenants_fs/'+tid+'/ && \
			   mkdir /root/isochamber/tenants_fs/'+tid+'/upper && \
			   mkdir /root/isochamber/tenants_fs/'+tid+'/workdir && \
			   mkdir /root/isochamber/tenants_fs/'+tid+'/overlay') != 0 ):
	printR("--- Failed to build overlay filesystem ---")
	sys.exit(1)

if( os.system('mkdir /root/isochamber/tenants_persist/'+tid+'/') != 0 ):
	printR("--- Failed to build tenant's persistent directory ---")
	sys.exit(1)

if( os.system('cp -rfp /root/isochamber/base_fs/home /root/isochamber/tenants_persist/'+tid+'/') != 0 ):
	printR("--- Failed to create tenant's persistent home/ dir ---")
	sys.exit(1)

if( os.system('cp -rfp '+ten_dir+'/progs/* /root/isochamber/tenants_persist/'+tid+'/home/ ') != 0 ):
	printR("--- Failed to copy tenant progs/ to /home/ ---")
	sys.exit(1)

if( os.system('echo //--EndOfAllows-- > /root/isochamber/tenants_persist/'+tid+'/filter.gen.h') !=0 ):
	printR("--- Failed to add filter file ---")
	sys.exit(1)

if( os.system('make clean && make config') !=0 ):
	printR("--- Failed to compile handoff.c in configuration mode ---")
	sys.exit(1)

c_path,c_init= mount_tenant_config(tid)

if c_init != "":
	if( os.system('./handoff '+tid+" "+c_init) != 0 ):
		printR("--- handoff.c failed ---")
		sys.exit(1)

umount_tenant(c_path,tid)

printG("--- Tenant Configured! ---")
printG("tid: " + tid)