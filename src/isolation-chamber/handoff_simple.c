#define _GNU_SOURCE
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/utsname.h>
#include <sys/wait.h>
#include <unistd.h>

// #include <stdlib.h>
// #include <stdio.h>
#include <stddef.h>
#include <string.h>
// #include <unistd.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/prctl.h>
#include <sys/syscall.h>
#include <sys/socket.h>

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

    /* and if we don't match above, die */
    BPF_STMT(BPF_RET+BPF_K, SECCOMP_RET_TRAP),
};
struct sock_fprog filterprog = {
    .len = sizeof(filter)/sizeof(filter[0]),
    .filter = filter
};

static char child_stack[1048576];

static void print_nodename() {
  struct utsname utsname;
  uname(&utsname);
  printf("%s\n", utsname.nodename);
}




static int child_fn(){

    printf("New UTS namespace nodename: ");
    print_nodename();

    printf("Changing nodename inside new UTS namespace\n");
    sethostname("tenant", 6);

    printf("New UTS namespace nodename: ");
    print_nodename();


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






    char *args[] = {"Hello", "Sandbox", NULL};
    execv("./hellosandbox", args);
    printf("back to handoff.c\n");



    return 0;
}


int main(int argc, char **argv) {
    // char buf[1024];
    printf("Original UTS namespace nodename: ");
    print_nodename();

    printf("Original PID: %d\n", getpid());

    pid_t child_pid = clone(child_fn, child_stack+1048576, CLONE_NEWNS | CLONE_NEWPID | CLONE_NEWUTS | SIGCHLD, NULL);

    sleep(1);

    printf("Original UTS namespace nodename: ");
    print_nodename();

    waitpid(child_pid, NULL, 0);
    


    return 0;
}