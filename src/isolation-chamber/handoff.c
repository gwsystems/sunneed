#include "handoff.h"


static char child_stack[1048576];

char executable[100]; //global path to executable
char **args;

static void print_nodename() {
  struct utsname utsname;
  uname(&utsname);
  printf("%s\n", utsname.nodename);
}

//wrapper function for pivot_root system call:
int pivot_root(char *a, char *b){
    if(mount(a,a,"bind", MS_BIND | MS_REC,"") < 0){
        printf("error mounting in pivot_root\n");
    }
    if(mkdir(b,0755) < 0){
        printf("error mkdir\n");
    }
//    printf("pivot setup good\n");

    return syscall(SYS_pivot_root,a,b);
}

int ns_config(){
    printf("Setting up new namespaces...\n");
    //new mount ns:
    if(unshare(CLONE_NEWNS) < 0){
        printf("can't create new mount ns");
    }
    if(umount2("/proc", MNT_DETACH) < 0){
        printf("error unmounting /proc\n");
    }


    if(pivot_root("./tenroot","./tenroot/.old") < 0){
        perror("error pivoting root");
    }
    
    if(mount("tmpfs","/dev", "tmpfs", MS_NOSUID | MS_STRICTATIME, NULL) < 0){
        printf("error mounting tmpfs\n");
    }
    if(mount("proc","/proc", "proc",0,NULL) < 0){
        printf("error mounting /proc\n");
    }
    if(mount("t","/sys","sysfs", 0, NULL) < 0){
        printf("erroor mounting 't'?\n");
    }

    chdir("/");
    

    if(umount2("/.old", MNT_DETACH) < 0){
        printf("error unmounting old\n");
    }
    if(rmdir("/.old") == -1){
        perror("rmdir");
    }

    //set up uts ns
    sethostname("tenant", 6);

    return 0;
}

//inspiration from: https://blog.lizzie.io/linux-containers-in-500-loc/contained.c
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
        return 1;
    }
    cap_free(cap);

    return 0;
}


static int child_fn(){
    //isolation ns code:
    ns_config();

    drop_caps();

    //seccomp bpf filter code:
    /* set up the restricted environment */
    printf("Setting seccomp syscall filter...\n");

    if (prctl(PR_SET_NO_NEW_PRIVS, 1, 0, 0, 0)) {
        perror("Could not start seccomp:");
        exit(1);
    }
    if (prctl(PR_SET_SECCOMP, SECCOMP_MODE_FILTER, &filterprog) == -1) {
        perror("Could not start seccomp:");
        exit(1);
    }
    printf("exec tenant program:\n\n");
    execv(executable, args);
    return 0;
}


int main(int argc, char **argv) {
    int n = (sizeof argv) / (sizeof *argv);
    if(argc <= 1){
        printf("no executable specified - exiting\n");
        return 0;
    }else if(argc >= 2){
        args = argv + 1;
    }

    printf("Original UTS namespace nodename: ");
    print_nodename();

    printf("Original PID: %d\n", getpid());

    // system("mount --make-rprivate /");//make root mount private
    if(mount(NULL, "/", NULL, MS_REC | MS_PRIVATE, NULL) != 0){
        printf("error mounting private\n");
    }

    //create executable path w/ user input
    strcpy(executable,"/bin/");
    strcat(executable, argv[1]);

    //clone child into new namespaces and run @ child_fn()
    pid_t child_pid = clone(child_fn, child_stack+1048576, CLONE_NEWPID | CLONE_NEWNET | CLONE_NEWUTS | SIGCHLD, NULL);

    if(child_pid < 0){
        printf("clone failed\n");
    }

    sleep(1);

    waitpid(child_pid, NULL, 0);

    printf("\n\nback to handoff... done\n\n");

    return 0;
}
