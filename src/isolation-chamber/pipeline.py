import os
import sys

if(os.system('python3 build.py') !=0):
	sys.exit(1)
if(os.system('python3 testscript.py') !=0):
	sys.exit(1)

sys.exit(0)