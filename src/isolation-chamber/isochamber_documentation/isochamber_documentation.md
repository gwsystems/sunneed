# Documentation:

This documentation is meant to help understand how each program is working. To understand how to use them, see the [README.md](../README.md). To get a high level understanding of the Isochamber structure, see the [system overview](./isochamber_system_overview.md).

## tenant.py -

>  This file holds python functions used by the rest of the python programs.

```python
def printR(skk): print("\033[91m {}\033[00m" .format(skk)) 
def printG(skk): print("\033[92m {}\033[00m" .format(skk)) 
def printY(skk): print("\033[93m {}\033[00m" .format(skk)) 
```

​		These functions provide useful colorization of output throughout different scripts.

```python
# function takes tid and verifies that it is configured
# return:
# 		success: tenant's respective json entry
#			failure: -1
def find_tenant(tid):
```

​		`find_tenant()` attempts to find the tenant's entry in `containers.json` and returns that dictionary if the tenant has been configured. If the tenant hasn't been configured, return -1.

```python
# function takes container name and returns tid
# return:
#			success: tid
#			failure: none - exit with error
def find_tid(cname):
```

​		`find_tid()` takes the container name and returns the `tid` if the tenant has been configured and exits with an error if it hasn't.		

```python
# This function is important as it is what mounts the tenant's
# root filesystem (.../overlay/)
# return:
# 		success: c_path, c_init
#			failure: none - exit with error
def mount_tenant(tid):
```

​		`mount_tenant()` is responsible for mounting the tenant's individual filesystem. It uses an OverlayFS implementation of a union mount. This lets each tenant use `isochamber/base_fs/` as the base of their unique filesystem. This function will exit if the mounting doesn't work. 

`c_path`: path to root of tenant's filesystem (`isochamber/tenants_fs/<tid>/overlay/`)

`c_init`: path to program executable specified in `containers.json`.

```python
# This function only differs from above in that
# it returns the location of the dependencies 
# script rather than the init program
# return:
# 		success: c_path, c_init
#			failure: none - exit with error
def mount_tenant_config(tid):
```

​		`mount_tenant_config()` is responsible for mounting the tenant's filesystem and returning the container path as well as the location of the installation script so that dependencies can be installed into the tenant fs. This is used during execution of `tenant_config.py`.

`c_path`: path to root of tenant's filesystem (`isochamber/tenants_fs/<tid>/overlay/`)

`c_init`: path to dependency installation script specified in `containers.json`.

```python
# unmount the tenant filesystem
# return:
# 		no return value
def umount_tenant(c_path, tid):
```

​		`umount_tenant()` is responsible for unmounting the overlay filesystem after the container is done running. 

`c_path`: path to root of tenant's filesystem (`isochamber/tenants_fs/<tid>/overlay/`)

Right now, this method is very simple and contains only one line that unmounts the union mount. During initial design, the entire tenant filesystem was deleted except for the `/home/` directory to keep persistent data overhead to a minimum. This causes errors with keeping dependencies downloaded accross multiple runs of a container. You can see commented out lines that accomplish this. If at any point, the decision is made to get rid of filesystem overhead during the umount process, this is the place to add this logic.

```python
# Uninstall a tenant container and remove all records of it
# return:
# 		success:  0
#			failure: -1
def delete_tenant(tid):
```

​		`delete_tenant()` attempts to find a tenant's container using `containers.json`. If the tenant is not found in this json file, the method returns -1 in error. If it finds the record of the tenant container, it removes that dictionary entry and updates `containers.json` not to include it. The tenant entries `~/isochamber/tenants_fs/<tid>/` and  `~/isochamber/tenants_persist/<tid>/` are then deleted. 

## build.py -

> This script prepares the host system to install, maintain, and run tenant containers. This program requires no extra command line arguments.

* `download_dependencies()` - function downloads necessary packages for Isochamber including `libcap-dev`, `debootstrap`, and `build-essential`. 
* Create directory `/root/isochamber/` and subdirectories `new_tenants/`, `base_fs/`, `tenants_fs/`, and `tenants_persist/`. Also create blank log of containers by `echo`ing `[]` to `isochamber/containers.json`.
* Use debootstrap to populate `base_fs/` with a debian filesystem.
* Copy shared objects necessary for IPC (`libsunneedclient.so` & `sunneed_overlay_testing.so`) to `base_fs/lib/` so they can be located within tenant containers.

## tenant_config.py -

> This script installs a new tenant container to the system. This program requires the container's name to be specified as a command line argument.

* Grab container name (`cname`) from command line arguments and use that to identify the tenant package located at `/root/isochamber/new_tenants/<cname>/`.
* Using `uuid` python module, and from it, the `uuid1()` function, we assign the tenant a unique id, and store this as a 32 character hexidecimal value that is from here on referred to as `tid` for tenant id. 
* Open the tenant's `config.json`, place `tid` into the blank `tid` field, and then add the whole `config.json` as an entry into `/root/isochamber/containers.json` so that the host has all the information it needs about the container.
* Create an entry for the tenant under `tenants_fs/<tid>/`. Populate this entry with subdirectories used for tenant's OverlayFS mount: `upper/`,`workdir/`, and `overlay/`. See the [system overview](./isochamber_system_overview.md) for more information about what these are for. 
* Create entry for tenant under `tenants_persist/<tid>/`. Populate this with a blank `home/` folder copied from `base_fs/`. This is the tenant's persistent `home/` directory. Populate this with the entire contents of `~/new_tenants/<tenant_package>/progs/*`. This step is what allows tenant programs to be run from within the container. This copy command uses `cp -rfp ... ...`. The `-p` flag specifies that all file permissions remain the same for the copy.
* Create a blank default whitelist with `echo //--EndOfAllows-- > /root/isochamber/tenants_persist/<tid>/filter.gen.h`. This is what `wlister.py` expects to see as a blank whitelist. Keeping it under `tenants_persist/` allows the unique filter to be saved per tenant.
* Compile `handoff.c` in configuration mode
* Call `mount_tenant_config(tid)` (located in `tenant.py`) to grab two paths from `isochamber/containers.json`. The first path is the path to the root of the tenant's filesystem and is found at `~/tenants_fs/<tid>/overlay/`. The second path returned is the container relative path to the dependency installation script, which by default is `/home/install_deps.sh`. 
* Run `handoff.c` passing the respective `tid` and executable path. This spawns the container with a network connection and will download any dependencies specified in `install_deps.sh`. 
* Call `umount_tenant()` (located in `tenant.py`) to unmount the process from the overlay mount.

## wlister.py -

> This program generates a unique system call whitelist for a tenant container specified by a `tid` command line argument. It does this by compiling `handoff.c` in debug mode so that the seccomp filter traps the process instead of killing it. This allows the program to be straced to see which system calls have trapped the program. By recursively compiling, strace-ing, and whitelisting, the tenant program keeps running until all syscalls have been whitelisted.
>
> TODO: Change recompilation loop to serializing tenant filters to a file. This will let the program run different tenants without recompiling `handoff.c` to get the proper filter every time.

```python
# function recursively compiles and straces program
# until the program stops trapping and it is fully
# whitelisted
# return:
# 		no return value
def check_prog():
```

*  `check_prog()` compiles `handoff.c` in debug mode, mounts the tenant's filesystem (using `mount_tenant()`), and straces the execution of `handoff.c`. 
* The program straces `handoff.c` in a process moved to the background using `&`. This is because sometimes strace hangs and needs to be killed. Without running this command in the background, `wlister.py` itself will hang as well as it waits for strace. Therefore we direct strace's output to a text file and `sleep()` while we wait it to finish. 
* After sleeping, unmount the tenant fs using `umount_tenant()` and kill all running strace processes.
* The output of strace is then examined. The method uses two regex patterns (one is incase this is being compiled on ARM) to find trapped syscalls. These calls are then checked against Docker's default blacklist to see if the call is potentially malicious (using `check_prog()`). 
* If the program trapped and syscalls are found, call `allow_calls()` to whitelist said syscalls. This is where the program is recursive because `allow_calls()` again calls `check_prog()` until the program doesn't trap and...
* ... the program prints a success message, recompiles the program in production mode, and saves the generated whitelist filter to `tenants_persist/<tid>/filter.gen.h`.

```python
# check to see if program is malicious by comparing
# against default Docker blacklist
# return:
# 		no return value
def check_call(syscall):
```

​		`check_call()` takes a list of system calls and checks to see whether or not any of them are present in Docker's default blacklist (located in `default_docker_blacklist.txt`). Right now a warning is just output for the developer but logic to report an error or exit could be added here.

```python
# allow calls by adding system calls to filter file
# return:
# 		no return value
def allow_calls(syscalls):
```

​		`allow_calls()` takes a list of system calls and appends each one to tenant's `filter.gen.h` file. Once the syscalls have been added to the filter file, recurse and call `check_prog()`.

## handoff.py -

> `handoff.py` is a python wrapper script for `handoff.c`. It takes care of mounting and unmounting the tenant filesystem before and after execution of `handoff.c`. 

`handoff.py` requires a command line argument specifying the tenant's tid. There is also an optional flag that will run the container in configuration mode. This optional flag is `-configure`. If it is used, then `handoff_shell()` is called. Otherwise, during normal use, `handoff_tenant()` is called. The tenant filesystem is unmounted (using `umount_tenant()`) after either method is called.

```python
# function runs to handoff a tenant in normal operation
# NOTE: First line manually hard copies tenant's filter
# to filter.gen.h within sunneed/isochamber/ and this
# is what requires recompilation during handoff. This
# is less than ideal and should be changed so that 
# tenant filters are serialized so that handoff.h doesn't
# need to be recompiled with the correct filter
# return:
# 		success: c_path
#			failure: none - exit with error
def handoff_tenant():
```

* Hard copy tenant filter from persistant directory into `sunneed/src/isolation-chamber/` so that we have the correct whitelist to be used in the seccomp filter. 
* Recompile `handoff.c` and `handoff.h` so that `handoff.h`, which includes `"filter.gen.h"` will now include the correct filter file.
* Mount tenant filesystem with `mount_tenant()`
* Execute `./handoff` passing the respective `tid` and path to the program to be run.

```python
# function hands off a tenant in configuration mode 
# this is used for development as it allows you to
# enter a shell in the container with direct internet 
# acccess for more complicated debugging, testing,
# and downloading needs... (i.e. only way so far to 
# download a git repo within container)
# return:
# 		success: c_path
#			failure: none - exit with error
def handoff_shell():
```

* No whitelist is needed for this step as running `handoff.c` in configuration mode skips installing the seccomp filter.
* Recompile `handoff.c` in configuration mode so that the container has all root capabilities, no syscall filter, and a network connection.
* Mount tenant filesystem with `mount_tenant()`. We don't need to use `mount_tenant_config()` because we aren't trying to run the dependency installation script, we are merely using the convenient configuration mode to run a bash within the container so...
* ... Execute `./handoff` passing the respective `tid` and `/bin/bash` as the program to be exec-ed by `handoff.c`

## delete_tenant.py -

> This is a very simple script designed to ease container management by the developer. Use this program to completely uninstall a container given a tenant id.

`delete_tenant.py` simply takes the `tid` from the command line arguments and returns an exits if no `tid` is specified. The script then just calls `delete_tenant()` from `tenant.py`, exits if it returns an error, and prints a success message if the tenant container is properly uninstalled.

----------------------------------------

## handoff.h -

> This `.h` file is included from `handoff.c` and itself includes all the required libraries that `handoff.c` needs to run. It also holds the seccomp filter that is attached to the tenant process during `handoff.c` execution. Seccomp (SECure COMPuting), in combination with a BPF (Berkley Packet Filter) filter allows a process to filter incoming system calls. We define this filter to be a whitelist so any system calls not present in the filter will cause the process to trap out. See the following sets of documentation to learn more: [eigenstate.org](https://eigenstate.org/notes/seccomp.html) & [kernel.org](https://www.kernel.org/doc/html/v4.16/userspace-api/seccomp_filter.html).

`handoff.h` holds a `struct sock_fprog` that is used within `handoff.c` when placing the process in seccomp mode. A `sock_fprog` holds an array of `struct sock_filter`s called `filter[]`. This is the filter mentioned above. Each entry in `filter[]` is a BPF instruction. BPF code is an assembly-like language that doesn't allow loops in order to ensure an infinite loop isn't possible. This is because BPF code runs in the kernel itself. Each instruction is made up of an opcode, offsets, and an operand. To ease development, the kernel has macros available (BPF_JMP & BPF_STMT) to avoid having to hard code BPF instructions. These are explained in depth in the eigenstate.org article linked above. Below, I will explain the macro and code bits that aren't covered in either of the articles linked above.

```c#
#define ALLOW_ARM(syscall) \
    BPF_JUMP(BPF_JMP+BPF_JEQ+BPF_K, __ARM_NR_##syscall, 0, 1), \
    BPF_STMT(BPF_RET+BPF_K, SECCOMP_RET_ALLOW)
```

​		The macro `ALLOW(syscall)` is explained in the eigenstate.org article. This differs from that in one way. Notice the `__ARM_NR_##syscall`. This is the form that ARM specific system calls have so when compiling this program on ARM, `wlister.py` will whitelist system calls using `ALLOW_ARM` instead of `ALLOW`.

```c#
#if __x86_64__    
    	BPF_JUMP( BPF_JMP+BPF_JEQ+BPF_K, AUDIT_ARCH_X86_64, 1, 0),
#elif __arm__
    	BPF_JUMP( BPF_JMP+BPF_JEQ+BPF_K, AUDIT_ARCH_ARM, 1, 0),
#endif
```

​		This code snippet is within `filter[]`. These two lines set up the filter to audit system calls against either x86_64 or ARM respectively. This allows the program to be compiled on either architecture without having to change anything.

```c#
/* list of allowed syscalls */
ALLOW(exit_group),  /* exits a processs */
ALLOW(execve),      /* called by parent to create child */

#include "filter.gen.h" //this holds the tenant's whitelisted syscalls
```

​		This section of `filter[]` holds the whitelisted system calls. `exit_group` is always needed so a process can exit normally. `execve` is needed by default to allow `handoff.c` to exec the tenant process. The include `"filter.gen.h"` line allows any BPF instructions present in `filter.gen.h` to be inserted into this filter. See further `filter.gen.h` documentation below.

```c#
#ifdef DEBUG
#if DEBUG==1
  BPF_STMT(BPF_RET+BPF_K, SECCOMP_RET_TRAP), //used for debugging with strace
#endif
#else
  BPF_STMT(BPF_RET+BPF_K, SECCOMP_RET_KILL), //used in production for tenant oppression
#endif
```

​		This snippet is at the bottom of `filter[]`. By adding the compilation flag `-DDEBUG=1` (used when you run `make debug`), you can compile this program in debug mode. This dictates whether or not to trap or kill the process calling an unlisted system call. Debug mode is used during in `wlister.py` for the whitelisting implementation.

## filter.gen.h -

> This file holds whitelisted system calls for a tenant container. This is included from within a struct located in `handoff.h` so that the filter can easily be switched out for different tenants.

This is one thing that should most likely be redesigned. It would be far more efficient to figure out how to serialize different tenant's filters into a file so that the respective one could be installed during `handoff.c` runtime and avoid the need to recompile the whole program.

As discussed above in the `handoff.h` section, BPF filters are basically an array of BPF instructions. Useful macros like `ALLOW()` can easily string together multiple BPF instructions. Thus, this file is just filled with as many `ALLOW()` macros as the tenant program needs (and or `ALLOW_ARM()` if running on ARM architecture). This allows as many system calls as needed be dynamically added while the rest of `filter[]` remains the same accross different tenant containers.

Below is an example of a `filter.gen.h` file that you would get when whitelisting `test_progs/test_simple.c`:

```c#
  ALLOW(close),
	ALLOW(brk),
	ALLOW(write),
	ALLOW(writev),
	ALLOW(access),
	ALLOW(fstat),
	ALLOW(openat),
	ALLOW(read),
	ALLOW(mmap),
	ALLOW(arch_prctl),
	ALLOW(mprotect),
	ALLOW(munmap),
//--EndOfAllows--
```



## handoff.c -

> `handoff.c` is the big cheese of this whole operation. This is the program that takes a `tid` and an executable path, and spawns an isolated container envionment to run the respective tenant's program.

`handoff.c` can be compiled in multiple modes. There is standard mode which is accomplished with `make`. In standard mode, `handoff.c` sets up the tenant process namespaces, drops root capabilities, and attaches a seccomp filter before exec-ing the tenant program within the sandbox. There is debug mode which can be done with `make debug` (as discussed in the `handoff.h` section above). And there is configuration mode which can be compiled by running `make config`. When running the program in configuration mode, root capabilities are not dropped, and the seccomp filter is not installed. The only thing it does is set up the tenant namespaces and exec the sandboxed process. Before moving on to function documentation, I will include a snippet from the main method to show how configuration mode works:

```c#
#ifdef CONFIG 
#if CONFIG==1
    //clone child with new namespaces except NET to ease development & run @ child_config()
    pid_t child_pid = clone(child_config, child_stack+1048576, CLONE_NEWNS | CLONE_NEWPID |
                            									CLONE_NEWIPC | CLONE_NEWUTS | SIGCHLD, NULL);
#endif
#else
    //clone child into new namespaces and run @ child_fn()
    pid_t child_pid = clone(child_fn, child_stack+1048576, CLONE_NEWNS | CLONE_NEWPID |
                            CLONE_NEWNET | CLONE_NEWIPC | CLONE_NEWUTS | SIGCHLD, NULL);
#endif
```

​		Notice that when compiled with `-DCONFIG=1` (as is done when running `make config`), there is one less `CLONE` flag. The dropped flag is `CLONE_NEWNET`. This prevents isolating the process from the network as is done in standard compilation mode. This is useful for downloading library dependencies. The other difference is starting the cloned process in `child_config()` instead of `child_fn()`. Find out more by reading the method documentation below.

```c#
/* 
 * cloned child process starts here
 * and execs tenant program
 * no return 
 */
static int child_fn(){
```

​		This function does four things. It calls `ns_config()` followed by `drop_caps()`. It then uses two `prctl()` calls to set the seccomp filter for the process (these are explained in the eigenstate article listed in the `handoff.h` section). The last thing this function does is `execv()` the tenant program. Therefore the `child_fn()` function will not return as exec functions only return if an error occurs.

```c#
/* 
 * cloned child process starts here in configuration mode
 * and execs tenant program
 * no return
 */
static int child_config(){
```

​		This function is used when `handoff.c` is compiled in configuration mode. It differs from `child_fn()` in two ways. First, root capabilities are not dropped. Second, the seccomp filter isn't installed for the process.

```c#
/*
 * Configure tenant namespaces
 * return:
 * 		success:  0
 * 		failure: -1
 */
int ns_config(){
```

​		This function sets up child namespaces. The majority of the method deals with filesystem isolation (the mount namespace). This function mounts the directory `/root/isochamber/tenants_fs/<tid>/overlay`. The `pivot_root()` system call is then used to switch the process's root to the `~/overlay/` directory and then save the old filesystem into a subdirectory of the new root. The old filesystem is then unmounted and deleted. Before the old filesystem is removed, two shared mounts are created. The tenant's `/home/` directory is binded to `/root/isochamber/tenants_persist/<tid>/home/`. The tenant then creates a shared, read-only mount binded to Sunneed's `/tmp/tenant_ipc/`. Learn more about why in the [system overview](./isochamber_system_overview.md). 

​		The hostname is set as the `tid` for this processes UTS namespace. 

​		Note: Currently the majority of namespace configuration is done for the mount namespace. By placing the process in all of these namespaces (NS (mount), PID, NET, IPC, UTS), these resources are isolated from the host and other tenants. However, some more configuration, such as network connection, may need to be done as Sunneed is developed. Also consider adding user-namespace isolation. This can provide a lot of fine grain isolation but has been known to have a fair amount of bugs in the past as it is a fairly recent namespace and changes a lot of kernel functionality.

```c#
/*
 * Drop root capabilities
 * return:
 * 		success:  0
 * 		failure: -1
 */
int drop_caps(){
```

​		Linux divides root privileges, capabilities, into separate units that can be separately removed for a user. This [blog post](https://blog.lizzie.io/linux-containers-in-500-loc.html) does a great job of highlighting the power of different capabilities and the implication of dropping/keeping it with different namespace isolation. This method removes capabilities from the tenant process.

--------------------------------

## Next Steps:

* Whitelisting process:

  Right now whitelist filters are copied from the tenant's persistent directory into isolation-chamber directory to be used by `handoff.c`. `handoff.c` must be recompiled to make use of the new seccomp filter. This should be changed so that the filters are serialized into a file and can be chosen at runtime.

  During whitelisting, if a malicious system call is whitelisted, a warning is output for the developer. Maybe develop some way to require human intervention to decide yes or no or at least some other check on this. This is a bit confusing as this whole process might have to take place on the cloud because embedded computers won't always have a compiler on them which is required for the whitelisting process. 

* Tenant onboarding:

  Develop some way for tenants to obtain a sample tenant package to fill out and turn in. Give them some way to upload their tenant package and have that be automatically configured on the system. 

* Testing:

  Add more unit testing as this is added into production. Extensive automated testing hasn't been done as it was undergoing constant  changes throughout this project.

* Look into:

  * [cgroups](https://www.kernel.org/doc/Documentation/cgroup-v1/cgroups.txt) - a kernel feature that allows limitation, accounting, and resource isolation for groups of processes (i.e. all tenants or a subsection of tenants)
  * LSMs - [Linux Security Modules](https://www.kernel.org/doc/html/v4.15/admin-guide/LSM/index.html) - Framework provides comprehensive security checks and allows Mandatory Access Control (MAC) to provide and define security policies (Can be more comprehensive than just relying on capabilities)
  * Look into guarding against LD preload workarounds. It can be possible to get around LD preload. One way is making direct system calls. This should be covered by the seccomp filter. Just make sure there aren't any trivial ways to get around LD preload and bypass Sunneed's transparent interposition layer.