import os
import sys
import uuid
import json

def printR(skk): print("\033[91m {}\033[00m" .format(skk)) 
def printG(skk): print("\033[92m {}\033[00m" .format(skk)) 
def printY(skk): print("\033[93m {}\033[00m" .format(skk)) 




#TENANT ONBOARD
# sudo tenant_config.py /path/to/tenant/program/files
# store these in /root/isochamber/new_tenants


	






argn = len(sys.argv)
if(argn < 2 ):
	printR("--- ERROR - Usage: \nsudo python3 tenant_config.py  <tenant uname>")
	sys.exit(1)

uname = sys.argv[1]
ten_dir = "/root/isochamber/new_tenants/"+uname

id = uuid.uuid1()

tid = id.hex

ten_config = ten_dir + "/config.json"

print(tid)

with open('/root/isochamber/containers.json', 'r') as file:
		containers_dict = json.load(file)

with open(ten_config, 'r') as file:
		config_dict = json.load(file)



config_dict['tid'] = tid


containers_dict.append(config_dict)
print(containers_dict)
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
	printR("--- Failed to build persistent /home dir 1 ---")
	sys.exit(1)

if( os.system('cp -rfp /root/isochamber/base_fs/home /root/isochamber/tenants_persist/'+tid+'/') != 0 ):
	printR("--- Failed to build persistent /home dir 2 ---")
	sys.exit(1)

if( os.system('cp -rfp '+ten_dir+'/progs/* /root/isochamber/tenants_persist/'+tid+'/home/ ') != 0 ):
	printR("--- Failed to build persistent /home dir 3 ---")
	sys.exit(1)

if( os.system('echo //--EndOfAllows-- > /root/isochamber/tenants_persist/'+tid+'/filter.gen.h') !=0 ):
	printR("--- Failed to add filter file ---")
	sys.exit(1)
# if( os.system('rm -rf '+ten_dir)):
# 	printR("--- Failed to remove " + ten_dir)
# 	sys.exit(1)


