# first program to run within isochamber
# - set up tenant filesystem
import os
import sys
from tenant import printR, printG, printY

def download_dependencies():
	printY("--- Downloading dependencies...")

	# download libcap-dev which allows you to compile with
	# -lcap flag to have access to #include <sys/capability.h>
	if(os.system('apt-get install libcap-dev') != 0):
		printR("--- Failed to install libcap-dev ---")
		sys.exit(1)#error exit

	# download packages to set up tenant filesystem
	if(os.system('apt-get install debootstrap') != 0):
		printR("--- Failed to install debootstrap ---")
		sys.exit(1)#error exit

	if(os.system('apt-get install build-essential') != 0):
		printR("--- Failed to install build-essential ---")
		sys.exit(1)#error exit

	
download_dependencies()


printY("--- install isochamber directory structure ---")
# set up container managementstructure
if( os.system('mkdir /root/isochamber && \
			   echo [] > /root/isochamber/containers.json && \
			   mkdir /root/isochamber/new_tenants && \
			   mkdir /root/isochamber/base_fs && \
			   mkdir /root/isochamber/tenants_fs && \
			   mkdir /root/isochamber/tenants_persist') != 0 ):
	printR("--- Failed to build /root/isochamber ---")
	sys.exit(1)


# install debian to /root/isochamber/base_fs
# (can define architecture specifically but it will automatically choose 
# if dpkg is installed, which it usually is by default, specify if this breaks)
printY("--- Installing Debian Buster to .../base_fs/ ...")
if(os.system('debootstrap buster /root/isochamber/base_fs/ http://ftp.us.debian.org/debian') != 0):
	printR("--- Failed to install Debian to /root/isochamber/base_fs/ ---")
	sys.exit(1)#error exit


printG("--- Build Complete ---")
sys.exit(0)#clean exit
