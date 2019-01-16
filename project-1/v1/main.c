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
    char* arg = malloc(100);
    char* dir = malloc(100);
    printf("Arguments: ");
    scanf("%s", arg);
    printf("Directory: ");
    scanf("%s", dir);

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
        char *cmd = "ls";
        char *argv[4];
        argv[0] = "ls";
        argv[1] = arg;
        argv[2] = dir;
        argv[4] = NULL;
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

int valid_selection(const char c) {
    const char valids[] = "012acep";
    char ch[2];
    ch[0] = c;
    ch[1] = 0;
    int r = (strchr(valids, c) != NULL);
    return r;
}



int main() {
    printf("===== Mid-Day Commander, v0 =====\n");
    while (1) {
        printf("G’day, Commander! What command would you like to run?\n");
        char selection = 'z';
        while (!valid_selection(selection)) {
            printf("\t0. whoami : Prints out the result of the whoamicommand\n");
            printf("\t1. last : Prints out the result of the last command\n");
            printf("\t2. ls : Prints out the result of a listing on a user-specified path\n");
            printf("\ta. add command : Adds a new command to the menu.\n");
            printf("\tc. change directory : Changes process working directory\n");
            printf("\te. exit : Leave Mid-Day Commander\n");
            printf("\tp. pwd : Prints working directory\n");
            printf("Option?: ");
            scanf(" %c", &selection);
            if (!valid_selection(selection)) {
                printf("\nSorry, that's not a valid option. Choose one of the following:\n\n");
            }
        }
        printf("\n");
        if (selection == '0') {
            printf("-- Who Am I? --\n");
            whoami();
        } else if (selection == '1') {
            printf("-- Last Logins --\n");
            last();
        } else if (selection == '2') {
            printf("-- Directory Listing --\n");
            ls();
        } else {
            printf("Sorry, that command isn't supported yet\n\n");
        }
    }
    return 0;
}