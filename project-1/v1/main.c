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
    printf("Arguments: ");
    int i = 0;
    scanf("%c", &arg[i]);
    while(i == 0 || (i < 98 && arg[i-1] != '\n')){
        scanf("%c", &arg[i]);
        ++i;
    }
    arg[i-1] = '\0';
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

char *entries[100];

void add_entry(char e[]) {
    int i = 0;
    for (i = 0; i < 100; i++) {
        if (entries[i] == NULL) {
            char *temp = malloc(100);
            strncpy(temp, e, 100);
            entries[i] = temp;
            entries[i + 1] = NULL;
            return;
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

    /*const char valids[] = "012acep";
    char ch[2];
    ch[0] = c;
    ch[1] = 0;
    int r = (strchr(valids, c) != NULL);
    return r;*/
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
        printf("\n");
        if (!strcmp(selection, "0")) {
            printf("-- Who Am I? --\n");
            whoami();
        } else if (!strcmp(selection, "1")) {
            printf("-- Last Logins --\n");
            last();
        } else if (!strcmp(selection, "2")) {
            printf("-- Directory Listing --\n");
            ls();
        } else {
            printf("Sorry, that command isn't supported yet\n\n");
        }
    }
    return 0;
}
