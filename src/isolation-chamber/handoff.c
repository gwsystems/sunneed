#include "handoff.h"
 
static char child_stack[1048576];

//global variables used throughout
char **args;
char *tid;
char data_dir[100];
char new_mount_dir[100];
char child_fs[100];//if we have more than 10^4 tenants 50 could be a problem for char amount 
char old_dir[100];
char ipc_dir[100];
char child_home[100];
char executable[100]; //global path to executable
char hostname[33];
int  hn_len;


//wrapper function for pivot_root system call:
int pivot_root(char *a, char *b){
    return syscall(SYS_pivot_root,a,b);
}

/* 
 * Configure tenant namespaces
 * return:
 *      success:  0
 *      failure: -1
 */
int ns_config(){
    printf("Setting up new namespaces...\n");
    int err = 0; //keep error status
    //mount rprivate to ensure fs is private to processes outside mount namespace
    if(mount(NULL, "/", NULL, MS_REC | MS_PRIVATE, NULL) != 0){
        perror("error mounting private\n");
        err = -1;
    }
    //umount proc because we need to mount new /proc for new PID namespace
    if(umount2("/proc", MNT_DETACH) < 0){
        perror("error unmounting /proc\n");
        err = -1;
    }

    //self binding creates mount point
    if(mount(child_fs, child_fs,"bind", MS_BIND | MS_REC,"") < 0){
        perror("error creating child mount point\n");
        err = -1;
    }
    if(mkdir(old_dir,0755) < 0){
        perror("error mkdir: /.old/\n");
        err = -1;
    }


    //bind shared home directory to save tenant working directory
    if(mount(data_dir,data_dir,NULL,MS_BIND,NULL)){
        perror("error binding to /home directory");
        err = -1;
    }
    if(mount(NULL,data_dir,NULL,MS_SHARED,NULL)){
        perror("error marking shared mount (/home)");
        err = -1;
    }
    if(mount(data_dir,child_home,NULL,MS_BIND,NULL)){
        perror("error binding tenant /home to persistant saved /home directory");
        err = -1;
    }


    //most important piece of function
    //switches namespace's root / to child_fs
    if(pivot_root(child_fs, old_dir) < 0){
        perror("error pivoting root");
        err = -1;
    }
    
    //mounting important pieces of fs
    if(mount("tmpfs","/tmp", "tmpfs", 0, NULL) < 0){ //create temporary fs for tmp to be mounted on
        perror("error mounting tmpfs\n");
        err = -1;
    }
    if(mount("proc","/proc", "proc",0,NULL) < 0){//mount /proc to access PID namespace relevant info
        perror("error mounting /proc\n");
        err = -1;
    }
    if(mount("t","/sys","sysfs", 0, NULL) < 0){//mount /sys to have access to kernel abstractions
        perror("error mounting /sys\n");
        err = -1;
    }


    //bind sunneed's tenant_ipc directory to a read only mount on tenants fs:
    //NOTE - these will fail if sunneed as host doesn't have /tmp/tenant_ipc/ directory
    if(mount("/.old/tmp/"SUNNEED_TENANT_IPC_DIR,"/.old/tmp/"SUNNEED_TENANT_IPC_DIR,NULL,MS_BIND,NULL)){
        perror("error binding /tmp/tenant_ipc");
        err = -1;
    }
    if(mount(NULL,"/.old/tmp/"SUNNEED_TENANT_IPC_DIR,NULL,MS_SHARED,NULL)){
        perror("error marking shared mount .../tenant_ipc");
        err = -1;
    }
    if(mkdir("/tmp/"SUNNEED_TENANT_IPC_DIR,0777) < 0){
        perror("error mkdir: /tmp/tenant_ipc/\n");
        err = -1;
    }
    if(mount("/.old/tmp/"SUNNEED_TENANT_IPC_DIR,"/tmp/"SUNNEED_TENANT_IPC_DIR,NULL,MS_BIND,NULL)){
        perror("error binding sunneed's /tmp/tenant_ipc -> /overlay/tmp/ipc");
        err = -1;
    }
    if(mount("/.old/tmp/"SUNNEED_TENANT_IPC_DIR,"/tmp/"SUNNEED_TENANT_IPC_DIR,NULL,MS_REMOUNT | MS_BIND | MS_RDONLY,NULL)){
        perror("error remounting read-only");
        err = -1;
    }
    
    chdir("/");

    // detatch from host fs
    if(umount2("/.old", MNT_DETACH) < 0){
        perror("error unmounting old\n");
        err = -1;
    }
    if(rmdir("/.old") == -1){
        perror("rmdir");
        err = -1;
    }

    //set up uts ns
    sethostname(hostname, hn_len);

    return err;
}
 
/* inspiration from: https://blog.lizzie.io/linux-containers-in-500-loc/contained.c */
/*
 * Drop root capabilities
 * return:
 *      success:  0
 *      failure: -1
 */
int drop_caps(){

    printf("Dropping capabilities...\n");

    int caps[] = {
        CAP_AUDIT_CONTROL,
        CAP_AUDIT_READ,
        CAP_AUDIT_WRITE,
        CAP_BLOCK_SUSPEND,
        CAP_DAC_READ_SEARCH,
        CAP_FSETID,
        CAP_IPC_LOCK,
        CAP_MAC_ADMIN,
        CAP_MAC_OVERRIDE,
        CAP_MKNOD,
        CAP_SETFCAP,
        CAP_SYSLOG,
        CAP_SYS_ADMIN,
        CAP_SYS_BOOT,
        CAP_SYS_MODULE,
        CAP_SYS_NICE,
        CAP_SYS_RAWIO,
        CAP_SYS_RESOURCE,
        CAP_SYS_TIME,
        CAP_WAKE_ALARM,
        CAP_NET_ADMIN,
        CAP_NET_BIND_SERVICE,
        CAP_NET_RAW,
	    CAP_SETUID,	//consider removing if we add user ns
	    CAP_SETGID,	//consider removing if we add user ns
	    CAP_SYS_CHROOT
    };

    size_t ncaps = sizeof(caps) / sizeof(caps[1]);

    int i;

    for(i = 0; i < ncaps; i++){
        if(prctl(PR_CAPBSET_DROP, caps[i], 0, 0, 0)) {
            perror("Couldn't drop cap");
            return -1;
        }
    }

    cap_t cap = NULL;
    if (!(cap = cap_get_proc())
        || cap_set_flag(cap, CAP_INHERITABLE, ncaps, caps, CAP_CLEAR)
        || cap_set_proc(cap)) {
        fprintf(stderr, "failed: %m\n");
        if (cap) cap_free(cap);
        return -1;
    }
    cap_free(cap);

    return 0;
}

/*
 * cloned child process starts here
 * and execs tenant program
 * no return
 */
static int child_fn(){
    //isolation ns code:
    if(ns_config() != 0){
        printf("Failed to set up namespaces\n");
        exit(1);
    }

    if(drop_caps() != 0){
        printf("failed dropping capabilities\n");
        exit(1);
    }

    
    printf("Setting seccomp syscall filter...\n");
    //seccomp bpf filter code:
    /* process entering the restricted environment */
    if (prctl(PR_SET_NO_NEW_PRIVS, 1, 0, 0, 0)) {
        perror("Could not start seccomp:");
        exit(1);
    }
    if (prctl(PR_SET_SECCOMP, SECCOMP_MODE_FILTER, &filterprog) == -1) {
        perror("Could not start seccomp:");
        exit(1);
    }
    printf("exec tenant program\n\n");
    execv(executable, args);
    return 0;
}

/*
 * cloned child process starts here in configuration mode
 * and execs tenant program
 * no return
 */
static int child_config(){
    //isolation ns code:
    if(ns_config() != 0){
        printf("Failed to set up namespaces\n");
        exit(1);
    }
    execv(executable, args);
    return 0;
}

/*
 * Build directory path names used in
 * namespace setup
 * Parameter:
 *      char* - tenant executable passed in command line args
 */
int build_paths(char *prog){
    char tenants_fs[] = "/root/isochamber/tenants_fs/";
    char tenants_persist[] = "/root/isochamber/tenants_persist/";

    //build new_mount_dir path where child fs lies
    strcpy(new_mount_dir,tenants_fs);
    strcat(new_mount_dir,tid);
    strcat(new_mount_dir,"/overlay");

    //build path to tenants persistant data directory
    strcpy(data_dir,tenants_persist);
    strcat(data_dir,tid);
    strcat(data_dir,"/home");

    //create executable path w/ user input
    strcat(executable, prog);

    //save mount dir to global child_fs var
    int len = strlen(new_mount_dir);
    memcpy(child_fs, new_mount_dir, len + 1);//save path name globally


    //create name of 'old directory' which will be unmounted by tenant
    memcpy(old_dir, child_fs, len + 1);
    strcat(old_dir, "/.old");

    //create ipc dir to mount shared on sunneed's ipc folder in /tmp/tenant_ipc
    memcpy(ipc_dir, child_fs, len + 1);
    strcat(ipc_dir, "/tmp/"SUNNEED_TENANT_IPC_DIR);//this macro is defined in: ../shared/sunneed_ipc.h

    //this directory is where tenant data persists
    memcpy(child_home, child_fs, len + 1); 
    strcat(child_home, "/home");

    //create hostname for tenant (for UTS NS)
    strcat(hostname,tid);
    hn_len = strlen(hostname);

    return 0;
}

int main(int argc, char **argv) {
    int n = (sizeof argv) / (sizeof *argv);

    if(argc <= 2){
        printf("USAGE: sudo ./handoff <tid> <prog path>  \n");
        return 0;
    }else if(argc > 2){
        args = argv + 2;
    }
    tid = argv[1];
    build_paths(argv[2]);

    //host process mounts fs with rprivate for its own safety from untrusted tenant
    if(mount(NULL, "/", NULL, MS_REC | MS_PRIVATE, NULL) != 0){
        printf("error mounting private\n");
    }

    
#ifdef CONFIG 
#if CONFIG==1
    //clone child with new namespaces except NET to ease development & run @ child_config()
    pid_t child_pid = clone(child_config, child_stack+1048576, CLONE_NEWNS | CLONE_NEWPID | CLONE_NEWIPC | CLONE_NEWUTS | SIGCHLD, NULL);
#endif
#else
    //clone child into new namespaces and run @ child_fn()
    pid_t child_pid = clone(child_fn, child_stack+1048576, CLONE_NEWNS | CLONE_NEWPID | CLONE_NEWNET | CLONE_NEWIPC | CLONE_NEWUTS | SIGCHLD, NULL);
#endif

    if(child_pid < 0){
        printf("clone failed\n");
    }

    sleep(1);

    //find child exit status
    int child_status;
    waitpid(child_pid, &child_status, 0);
    int exit_status = WIFEXITED(child_status); //nonzero = normal exit
                                               //zero    = failed exit
    
    printf("\n\nback to handoff... \n");

    printf("child_exit_status: ");
    if(exit_status == 0){
        printf("failed\n");
    }else{
        printf("successful\n");
    }

    return 0;
}
