#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "unistd.h"
#include "syscall.h"
#include "dirent.h"
#include "wait.h"
bool doexit = false;
char path[128];
int pid,ppid;
void sh_parseCommand(char **argv,int argc);
bool execute(char *command);
int main(int argcf,char **argvf) {
    if (argcf < 1) return 0;
    //if (strcmp(argvf[1],"init")) return 0;
    pid = getpid(); // remember pid for waitpid
    ppid = getppid();
    FILE *f = fopen("/dev/tty","rw");
    char buff[100];
    char *argv[100];
    while(doexit != true) {
        int argc = 0;
       printf(pwd(path,128));
       printf(" $ ");
       fread(buff,1,100,f);
       argv[argc] = strtok(buff," ");
       while(argv[argc]) {
        argc++;
        argv[argc] = strtok(NULL," ");
       }
       if (argc > 0) {
        sh_parseCommand(argv,argc);
       }
    }
    return 0;
}
void sh_parseCommand(char **argv,int argc) {
    if (!strcmp(argv[0],"reboot")) {
        helin_syscall(14,0,0,0,0,0);
    } else if (!strcmp(argv[0],"poweroff")) {
        helin_syscall(15,0,0,0,0,0);
    } else if (!strcmp(argv[0],"clear")) { 
        printf("\033[2J"); // only test, must work
    } else if (!strcmp(argv[0],"exit")) {
        doexit = true;
    } else if (!strcmp(argv[0],"cd")) {
        if (argc > 1) {
            chdir(argv[1]);
        }
    } else if (!strcmp(argv[0],"ls")) {
        DIR *d = NULL;
        if (argc >1) {
            d = opendir(argv[1]);
        } else {
            d = opendir(path);
        }
        if (!d) return;
        struct dirent *di = NULL;
        while((di = readdir(d)) != 0) {
            printf("%s\n",di->name);
        }
        closedir(d);
    } else if (!strcmp(argv[0],"fault")) {
        int u = 1;
        int a = 0;
        int i = u/a;
    } else if (!strcmp(argv[0],"mpe")) {
        helin_syscall(21,0,0,0,0,0);
        waitpid(pid,NULL,0);
    } else if (argv[0][0] == '/') {
        int _pid = 0;
        if ((_pid = execv(argv[0],0,NULL)) > 0) {
            // wait
            //printf("shell: spawned process: %u\n",_pid);
            waitpid(_pid,NULL,0);
        } else {
            printf("Execution fail\n");
        }
    } else {
        if (!execute(argv[0])) {
            printf("Commmand %s not found\n",argv[0]);
        }
    }
}
bool execute(char *command) {
    char *run_path = "/bin";
    int _pid = 0;
    if (run_path == NULL) {
        printf("cannot find PATH enviroment variable!\n");
        return false;
    }
    char *buff = malloc(100);
    sprintf(buff,"%s/%s",run_path,command);
    if ((_pid = execv(buff,0,NULL)) > 0) {
        free(buff);
        waitpid(_pid,NULL,0);
        return true;
    } else {
        free(buff);
        return false;
    }
    return false;
}
