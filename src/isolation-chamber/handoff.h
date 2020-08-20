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

#include <linux/capability.h>
#include <sys/capability.h>



#define ARCHFIELD offsetof(struct seccomp_data, arch)

#define ALLOW(syscall) \
    BPF_JUMP(BPF_JMP+BPF_JEQ+BPF_K, __NR_##syscall, 0, 1), \
    BPF_STMT(BPF_RET+BPF_K, SECCOMP_RET_ALLOW)

#define ALLOW_ARM(syscall) \
    BPF_JUMP(BPF_JMP+BPF_JEQ+BPF_K, __ARM_NR_##syscall, 0, 1), \
    BPF_STMT(BPF_RET+BPF_K, SECCOMP_RET_ALLOW)

struct sock_filter filter[] = {
    /* validate arch */
    BPF_STMT(BPF_LD+BPF_W+BPF_ABS, ARCHFIELD),

#if __x86_64__    
	BPF_JUMP( BPF_JMP+BPF_JEQ+BPF_K, AUDIT_ARCH_X86_64, 1, 0),
#elif __arm__
	BPF_JUMP( BPF_JMP+BPF_JEQ+BPF_K, AUDIT_ARCH_ARM, 1, 0),
#endif

    BPF_STMT(BPF_RET+BPF_K, SECCOMP_RET_KILL),

    /* load syscall */
    BPF_STMT(BPF_LD+BPF_W+BPF_ABS, offsetof(struct seccomp_data, nr)),

    /* list of allowed syscalls */
    ALLOW(exit_group),  /* exits a processs */
//    ALLOW(brk),     /* for malloc(), inside libc */
//    ALLOW(mmap2),        /* also for malloc() */
//    ALLOW(munmap),      /* for free(), inside libc */
//    ALLOW(write),       /* called by printf */
//    ALLOW(fstat),
    ALLOW(execve),      /* called by parent to create child */
    #include "filter.gen.h"

    /* and if we don't match above, die (trap or kill)*/
    #ifdef DEBUG
    #if DEBUG==1
        BPF_STMT(BPF_RET+BPF_K, SECCOMP_RET_TRAP),
    #endif
    #else
        BPF_STMT(BPF_RET+BPF_K, SECCOMP_RET_KILL),
    #endif

};
struct sock_fprog filterprog = {
    .len = sizeof(filter)/sizeof(filter[0]),
    .filter = filter
};
