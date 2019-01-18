#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <string.h>

#define OPTIONS 2

void whoami(){
    int link[2];
    struct timeval t0;
    struct timeval t1;
    gettimeofday(&t0, 0);
    struct rusage ru;
    pid_t pid = fork();


    if (pid == 0) {
        dup2(link[1], STDOUT_FILENO);
        close(link[0]);
        close(link[1]);
        char *cmd = "whoami";
        char *argv[2];
        argv[0] = "whoami";
        argv[1] = NULL;
        execvp(cmd, argv);
    } else {
        wait4(pid, 0, 0, &ru);
    }
    gettimeofday(&t1, 0);
    long elapsed = (t1.tv_usec - t0.tv_usec) / 1000;

    printf("\n-- Statistics ---\n");
    printf("Elapsed time: %ld milliseconds\n", elapsed);
    printf("Page Faults: %ld\n", ru.ru_majflt);
    printf("Page Faults (reclaimed): %ld\n\n", ru.ru_minflt);
}

void last(){
    int link[2];
    struct timeval t0;
    struct timeval t1;
    gettimeofday(&t0, 0);
    struct rusage ru;
    pid_t pid = fork();


    if (pid == 0) {
        dup2(link[1], STDOUT_FILENO);
        close(link[0]);
        close(link[1]);
        char *cmd = "last";
        char *argv[2];
        argv[0] = "last";
        argv[1] = NULL;
        execvp(cmd, argv);
    } else {
        wait4(pid, 0, 0, &ru);
    }
    gettimeofday(&t1, 0);
    long elapsed = (t1.tv_usec - t0.tv_usec) / 1000;

    printf("\n-- Statistics ---\n");
    printf("Elapsed time: %ld milliseconds\n", elapsed);
    printf("Page Faults: %ld\n", ru.ru_majflt);
    printf("Page Faults (reclaimed): %ld\n\n", ru.ru_minflt);
}

void ls(){
    char arg[100];
    char dir[100];
    printf("Arguments?: ");
    int i = 0;
    scanf("%c", &arg[i]);
    while(i == 0 || (i < 98 && arg[i-1] != '\n')){
        scanf("%c", &arg[i]);
        ++i;
    }
    arg[i-1] = '\0';
    i = 0;
    printf("Path?: ");
    while((i < 98 && dir[i-1] != '\n')){
        scanf("%c", &dir[i]);
        ++i;
    }
    dir[i-1] = '\0';
    if (i == 1){
        dir[0] = '.';
        dir[1] = '\0';
    }

    int link[2];
    struct timeval t0;
    struct timeval t1;
    gettimeofday(&t0, 0);
    struct rusage ru;
    pid_t pid = fork();


    if (pid == 0) {
        printf("\n");
        fflush(stdout);
        dup2(link[1], STDOUT_FILENO);
        close(link[0]);
        close(link[1]);
        char *cmd = "ls";
        char *argv[100];
        argv[0] = "ls";
        if (arg[0] == '\0'){
            argv[1] = dir;
            argv[2] = NULL;
        }
        else{
            int j = 1;
            char *split;
            split = strtok(arg, " ");
            while (split != NULL){
                argv[j] = split;
                ++j;
                split = strtok(NULL, " /n");
            }
            argv[j] = dir;
            argv[j+1] = NULL;
        }
        execvp(cmd, argv);
    } else {
        wait4(pid, 0, 0, &ru);
    }
    gettimeofday(&t1, 0);
    long elapsed = (t1.tv_usec - t0.tv_usec) / 1000;

    printf("\n-- Statistics ---\n");
    printf("Elapsed time: %ld milliseconds\n", elapsed);
    printf("Page Faults: %ld\n", ru.ru_majflt);
    printf("Page Faults (reclaimed): %ld\n\n", ru.ru_minflt);
}


int main() {
    printf("===== Mid-Day Commander, v0 =====\n");
    while (1) {
        printf("G’day, Commander! What command would you like to run?\n");
        int selection = -2;
        while (selection < 0 || selection > OPTIONS) {
            printf("\t0. whoami : Prints out the result of the whoamicommand\n");
            printf("\t1. last : Prints out the result of the last command\n");
            printf("\t2. ls : Prints out the result of a listing on a user-specified path\n");
            printf("Option?: ");
            scanf("%d", &selection);
            if (selection < 0 || selection > OPTIONS) {
                printf("\nSorry, that's not a valid option. Choose one of the following:\n\n");
            }
        }
        printf("\n");
        if (selection == 0) {
            printf("-- Who Am I? --\n");
            whoami();
        } else if (selection == 1) {
            printf("-- Last Logins --\n");
            last();
        } else if (selection == 2) {
            printf("-- Directory Listing --\n");
            ls();
        }
    }
    return 0;
}
