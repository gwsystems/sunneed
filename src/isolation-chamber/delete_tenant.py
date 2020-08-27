from tenant import sys,printG,printR,delete_tenant

argn = len(sys.argv)
if(argn < 2 ):
	printR("--- ERROR - Usage: \nsudo python3 delete_tenant.py  <tenant's id>")
	sys.exit(1)

tid = sys.argv[1]

if delete_tenant(tid) == -1:
	sys.exit(1)
else:
	printG("--- Success! Tenant deleted ---")