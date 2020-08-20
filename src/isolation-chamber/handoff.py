import os
import sys
from tenant import mount_tenant,umount_tenant,printR

if(len(sys.argv) < 2):
	printR("--- ERROR - Usage: sudo python3 handoff.py <tenant_id(tid)> ---")
	sys.exit(1)

tid      = sys.argv[1]
# obj_name = sys.argv[2]



if( os.system('cp -f /root/isochamber/tenants_persist/'+tid+'/filter.gen.h ./filter.gen.h') !=0 ):
	printR("--- Failed to copy tenant filter ---")
	sys.exit(1)

if( os.system('make clean && make') !=0 ):
	printR("--- Failed to compile handoff.c ---")
	sys.exit(1)


c_path,c_init= mount_tenant(tid)


if( os.system('./handoff '+tid+" "+c_init) != 0 ):
	printR("--- handoff.c failed ---")
	# sys.exit(1)

umount_tenant(c_path,tid)
