#define _GNU_SOURCE
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/utsname.h>
#include <sys/wait.h>
#include <unistd.h>

#include <stddef.h>
#include <string.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/prctl.h>
#include <sys/syscall.h>
#include <sys/socket.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <time.h>

#include <linux/filter.h>
#include <linux/seccomp.h>
#include <linux/audit.h>





#define ArchField offsetof(struct seccomp_data, arch)

#define Allow(syscall) \
    BPF_JUMP(BPF_JMP+BPF_JEQ+BPF_K, SYS_##syscall, 0, 1), \
    BPF_STMT(BPF_RET+BPF_K, SECCOMP_RET_ALLOW)

struct sock_filter filter[] = {
    /* validate arch */
    BPF_STMT(BPF_LD+BPF_W+BPF_ABS, ArchField),
    BPF_JUMP( BPF_JMP+BPF_JEQ+BPF_K, AUDIT_ARCH_X86_64, 1, 0),
    BPF_STMT(BPF_RET+BPF_K, SECCOMP_RET_KILL),

    /* load syscall */
    BPF_STMT(BPF_LD+BPF_W+BPF_ABS, offsetof(struct seccomp_data, nr)),

    /* list of allowed syscalls */
    Allow(exit_group),  /* exits a processs */
    Allow(brk),     /* for malloc(), inside libc */
    Allow(mmap),        /* also for malloc() */
    Allow(munmap),      /* for free(), inside libc */
    Allow(write),       /* called by printf */
    Allow(fstat),       /* called by printf */
    Allow(getpid),       /* called by parent & child */
    Allow(execve),      /* called by parent to create child */
    Allow(arch_prctl),
    Allow(access),
    Allow(openat),
    Allow(close),
    Allow(read),
    Allow(pread64),
    Allow(mprotect),
    Allow(uname),
    Allow(set_tid_address),//ls...
    Allow(set_robust_list),
    Allow(rt_sigaction),
    Allow(rt_sigprocmask),
    Allow(prlimit64),
    Allow(statfs),
    Allow(ioctl),
    Allow(getdents64),
    Allow(getuid),//sh...
    Allow(getgid),
    Allow(getppid),
    Allow(geteuid),
    Allow(getegid),
    Allow(getcwd),
    Allow(getpgrp),
    Allow(lstat),
    Allow(setuid),
    Allow(setgid),
    Allow(setpgid),
    Allow(stat),
    Allow(fcntl),
    Allow(kill),
    Allow(wait4),



    

    /* and if we don't match above, die */
    BPF_STMT(BPF_RET+BPF_K, SECCOMP_RET_TRAP),
};
struct sock_fprog filterprog = {
    .len = sizeof(filter)/sizeof(filter[0]),
    .filter = filter
};

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
    printf("pivot setup good\n");

    return syscall(SYS_pivot_root,a,b);
}


static int child_fn(){
    //isolation ns code:
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
    printf("New UTS namespace nodename: ");
    print_nodename();

    printf("Changing nodename inside new UTS namespace\n");
    sethostname("tenant", 6);

    printf("New UTS namespace nodename: ");
    print_nodename();

    //seccomp bpf filter code:
    /* set up the restricted environment */
    if (prctl(PR_SET_NO_NEW_PRIVS, 1, 0, 0, 0)) {
        perror("Could not start seccomp:");
        exit(1);
    }
    if (prctl(PR_SET_SECCOMP, SECCOMP_MODE_FILTER, &filterprog) == -1) {
        perror("Could not start seccomp:");
        exit(1);
    }

    /* printf only writes to stdout, but for some reason it stats it. */
    printf("hello from handoff.c\n");
    printf("PID: %d\n", getpid());

    
    //char *args[] = {"Hello", "Sandbox", NULL};
    // char *arg[] = {NULL};
    execv(executable, args);
    //execv("/bin/sh",args);
    printf("back to handoff.c\n");

    return 0;
}


int main(int argc, char **argv) {
    printf("argc: %d\n", argc);
    int n = (sizeof argv) / (sizeof *argv);
    printf("argn: %d\n", n);
    if(argc <= 1){
        printf("no executable specified - exiting\n");
        return 0;
    }else if(argc == 2){
        char *tempargs[] = {argv[0],NULL};
        args = tempargs;


    }else if(argc > 2){
        //printf("second argument: %s\n",argv[2]);
        args = argv + 1;
        // printf("first/second argument: %s\n",args[1]);
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

    printf("Original UTS namespace nodename: ");
    print_nodename();

    

    return 0;
}