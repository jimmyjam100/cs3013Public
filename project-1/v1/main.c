#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <string.h>

#define OPTIONS 2

char *entries[100];

void userCreated(int id){
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
        char *argv[100];
        int j = 0;
        char *split;
        char input[100];
        strncpy(input, entries[id], 100);
        split = strtok(input, " ");
        char *cmd = split;
        while (split != NULL){
            argv[j] = split;
            ++j;
            split = strtok(NULL, " ");
        }
        argv[j] = NULL;
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

void changeDir(){
    char arg[100];
    char dir[100];
    printf("New Directory?: ");
    int i = 0;
    scanf("%c", &arg[i]);
    while(i == 0 || (i < 98 && arg[i-1] != '\n')){
        scanf("%c", &arg[i]);
        ++i;
    }
    arg[i-1] = '\0';
    chdir(arg);
    printf("\n");
}

void pwd() {
    int link[2];
    pid_t pid = fork();
    if (pid == 0) {
        dup2(link[1], STDOUT_FILENO);
        close(link[0]);
        close(link[1]);
        char *cmd = "pwd";
        char *argv[2];
        argv[0] = "pwd";
        argv[1] = NULL;
        printf("Directory: ");
        fflush(stdout); // Why flush? Print is buffered until newline
                        // and since replace ourselves with a new process,
                        // we will never print unless we [force?] flush
        execvp(cmd, argv);
    } else {
        wait(NULL);
        printf("\n");
    }
}

int add_entry(char e[]) {
    int i = 0;
    for (i = 0; i < 100; i++) {
        if (entries[i] == NULL) {
            char *temp = malloc(100);
            strncpy(temp, e, 100);
            entries[i] = temp;
            entries[i + 1] = NULL;
            return i;
        }
    }
}

int valid_selection(const char *c) {
    if (!strcmp(c, "a") || !strcmp(c, "c") || !strcmp(c, "e") || !strcmp(c, "p")){
        return 1;
    }
    int i = 0;
    char convert[3];
    for(i = 0; entries[i]  != NULL; i++){
        sprintf(convert, "%d", i);
        if (!strcmp(c, convert)){
            return 1;
        }
    }
    return 0;
}



int main() {
    add_entry("whoami");
    add_entry("last");
    add_entry("ls");
    printf("===== Mid-Day Commander, v0 =====\n");
    while (1) {
        printf("G’day, Commander! What command would you like to run?\n");
        char selection[3] = "z";
        while (!valid_selection(selection)) {
            printf("\t0. whoami : Prints out the result of the whoamicommand\n");
            printf("\t1. last : Prints out the result of the last command\n");
            printf("\t2. ls : Prints out the result of a listing on a user-specified path\n");
            int i = 3;
            for(i = 3; entries[i] != NULL; i++){
                printf("\t%d. %s : User created command\n", i, entries[i]);
            }
            printf("\ta. add command : Adds a new command to the menu.\n");
            printf("\tc. change directory : Changes process working directory\n");
            printf("\te. exit : Leave Mid-Day Commander\n");
            printf("\tp. pwd : Prints working directory\n");
            printf("Option?: ");
            scanf("%s", selection);
            if (!valid_selection(selection)) {
                printf("\nSorry, that's not a valid option. Choose one of the following:\n\n");
            }
        }
        if (!strcmp(selection, "0")) {
            printf("\n-- Who Am I? --\n");
            whoami();
        } else if (!strcmp(selection, "1")) {
            printf("\n-- Last Logins --\n");
            last();
        } else if (!strcmp(selection, "2")) {
            printf("\n-- Directory Listing --\n");
            ls();
        } else if (!strcmp(selection, "e")) {
            printf("Logging you out, Commander.\n");
            return 0;
        } else if (!strcmp(selection, "c")) {
            printf("\n-- Change Directory --\n");
            changeDir();
        } else if (!strcmp(selection, "p")) {
            printf("\n-- Current Directory --\n");
            pwd();
        } else if (!strcmp(selection, "a")) {
            printf("\n-- Add a command --\n");
            printf("Command to add?: ");
            char cmd[100];
            int i = 0;
            scanf("%c", &cmd[i]);
            while(i == 0 || (i < 98 && cmd[i-1] != '\n')){
                scanf("%c", &cmd[i]);
                ++i;
            }
            cmd[i-1] = '\0';
            int id = add_entry(cmd);
            printf("Okay, added with ID %d!\n\n", id);
        } else {
            userCreated(atoi(selection));
            //printf("Sorry, that command isn't supported yet\n\n");
        }
    }
    return 0;
}
