
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

#define __NR_cs3013_syscall2 378 //define the id of syscall2 for later use

//define the struct that was given to us for the assignment
struct ancestry {
    pid_t ancestors[10];
    pid_t siblings[100];
    pid_t children[100];
};

int main(int argc, char **argv){
    if (argc != 2 || (atoi(argv[1]) <= 0 && argv[1][0] != '0')){ //if you entered the wrong number of arguments and its not a number let the user know
        printf("please enter one paramater that is the number of a pid\n");
        return 1;
    }
    struct ancestry *anc = malloc(sizeof(struct ancestry)); //allocate space for the system call to edit
    int i = 0;
    while(i < 100){ //initialize all the values to -1
        if (i < 10) {
            anc->ancestors[i] = -1;
        }
        anc->siblings[i] = -1;
        anc->children[i] = -1;
        ++i;
    }
    unsigned short *target_pid = malloc(sizeof(unsigned short)); //initialize a pointer to the inputed pid
    *target_pid = atoi(argv[1]); //get the value of the input
    if(syscall(__NR_cs3013_syscall2, target_pid, anc) == -1) { //call the systemcall
        printf("the pid was not valid\n");
    }
    i = 0; //print out all the values that were stored from the system call
    while(anc->children[i] != -1 && i < 100){
        printf("child pid %d\n", anc->children[i]);
        ++i;
    }
    i = 0;
    while(anc->siblings[i] != -1 && i < 100){
        printf("sibling pid %d\n", anc->siblings[i]);
        ++i;
    }
    i = 0;
    while(anc->ancestors[i] != -1 && i < 10){
        printf("ancestor pid %d\n", anc->ancestors[i]);
        ++i;
    }
    return 0;
}
