# first program to run within isochamber
# - set up tenant filesystem

import os
import sys

def printR(skk): print("\033[91m {}\033[00m" .format(skk)) 
def printG(skk): print("\033[92m {}\033[00m" .format(skk)) 
def printY(skk): print("\033[93m {}\033[00m" .format(skk)) 

error = 0

printY("--- Downloading dependencies...")

# download libcap-dev which allows you to compile with
# -lcap flag to have access to #include <sys/capability.h>
if(os.system('sudo apt-get install libcap-dev') != 0):
	printR("--- Failed to install libcap-dev ---")
	sys.exit(1)#error exit

# download packages to set up tenant filesystem
if(os.system('sudo apt-get install debootstrap') != 0):
	printR("--- Failed to install debootstrap ---")
	sys.exit(1)#error exit

if(os.system('sudo apt-get install build-essential') != 0):
	printR("--- Failed to install build-essential ---")
	sys.exit(1)#error exit

	
printY("--- Installing Debian Buster to ./tenroot/ ...")
# install debian to ./tenroot/
# (can define architecture specifically but it will automatically choose 
# if dpkg is installed, which it usually is by default, specify if this breaks)
if(os.system('sudo debootstrap buster ./tenroot/ http://ftp.us.debian.org/debian') != 0):
	printR("--- Failed to install Debian to ./tenroot/ ---")
	sys.exit(1)#error exit


printG("--- Build Complete ---")
sys.exit(0)#clean exit