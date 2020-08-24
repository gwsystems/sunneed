import os
import sys
from tenant import mount_tenant,umount_tenant,printR

# function runs to handoff a tenant in normal operation
# NOTE: First line manually hard copies tenant's filter
# to filter.gen.h within sunneed/isochamber/ and this
# is what requires recompilation during handoff. This
# is less than ideal and should be changed so that 
# tenant filters are serialized so that handoff.h doesn't
# need to be recompiled with the correct filter
def handoff_tenant():
	if( os.system('cp -f /root/isochamber/tenants_persist/'+tid+'/filter.gen.h ./filter.gen.h') !=0 ):
		printR("--- Failed to copy tenant filter ---")
		sys.exit(1)

	if( os.system('make clean && make') !=0 ):
		printR("--- Failed to compile handoff.c ---")
		sys.exit(1)

	c_path,c_init= mount_tenant(tid)

	if( os.system('./handoff '+tid+" "+c_init) != 0 ):
		printR("--- handoff.c failed ---")

	return c_path

# function hands off a tenant in configuration mode 
# this is used for development as it allows you to
# enter a shell in the container with direct internet 
# acccess for more complicated debugging, testing,
# and downloading needs... (i.e. only way so far to 
# download a git repo within container)
def handoff_shell():
	if( os.system('make clean && make config') !=0 ):
		printR("--- Failed to compile handoff.c ---")
		sys.exit(1)

	c_path,c_init= mount_tenant(tid)

	if( os.system('./handoff '+tid+" /bin/sh") != 0 ):
		printR("--- handoff.c failed ---")

	return c_path
# --------------------------------------------------------------

# PROGRAM START
if(len(sys.argv) < 2) or (len(sys.argv) > 3):
	printR("--- ERROR - Usage: sudo python3 handoff.py <tenant_id(tid)> <optional flag: -configure>---")
	sys.exit(1)

tid      = sys.argv[1]
c_path = ""

# see if user is trying to run in configure mode
if len(sys.argv) == 3:
	configure = sys.argv[2]
	if(configure == "-configure"):
		c_path = handoff_shell()
	else:
		printR("--- ERROR - Usage: sudo python3 handoff.py <tenant_id(tid)> <optional flag: -configure>---")
		sys.exit(1)
else:
	c_path = handoff_tenant()

# umount tenant process from container fs
umount_tenant(c_path,tid)
