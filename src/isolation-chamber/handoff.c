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
    }else if(argc >= 2){
        args = argv + 1;
        //printf("first argument: %s\n",args[0]);

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
