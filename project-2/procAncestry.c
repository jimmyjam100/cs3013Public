
#include <sys/syscall.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <string.h>
#include <pthread.h>
#include <assert.h>

#define __NR_cs3013_syscall2 378

struct ancestry {
    pid_t ancestors[10];
    pid_t siblings[100];
    pid_t children[100];
};

int main(int argc, char **argv){
    if (argc != 2 || (atoi(argv[1]) == 0 && argv[1][0] != '0')){
        printf("please enter one paramater that is the number of a pid\n");
        return 1;
    }
    struct ancestry *anc = malloc(sizeof(struct ancestry));
    int i = 0;
    while(i < 100){
        if (i < 10) {
            anc->ancestors[i] = -1;
        }
        anc->siblings[i] = -1;
        anc->children[i] = -1;
        ++i;
    }
    unsigned short *target_pid = malloc(sizeof(unsigned short));
    *target_pid = atoi(argv[1]);
    syscall(__NR_cs3013_syscall2, target_pid, anc);
    i = 0;
    while(anc->children[i] != -1){
        printf("child pid %d\n", anc->children[i]);
        ++i;
    }
    return 0;
}
