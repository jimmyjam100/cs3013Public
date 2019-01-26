#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <string.h>

#define COMMAND_CHAR_LIMIT 1000
#define COMMAND_LIMIT 100

char *entries[COMMAND_LIMIT];

/*
run a user created command from the entries array at the index of id
*/
void userCreated(int id){
    int link[2];
    struct timeval t0;
    struct timeval t1;
    gettimeofday(&t0, NULL); //get the time of day before running the command
    struct rusage ru;
    pid_t pid = fork(); //create a child procces to run the actul command


    if (pid == 0) { //if I am the child
        dup2(link[1], STDOUT_FILENO);
        close(link[0]);
        close(link[1]);
        char *argv[100]; //alocate space you args
        int j = 0;
        char *split;
        char input[COMMAND_CHAR_LIMIT];
        strncpy(input, entries[id], COMMAND_CHAR_LIMIT); //copy the command to a local variable so we do not split it up
        split = strtok(input, " "); //split the command by spaces
        char *cmd = split; //set the command to be the first substring seperated by spaces
        while (split != NULL){ //for every substring seperated by spaces in the users command
            if (j == 99){ //if the user alread has too many args give the user an error message
                printf("ERROR: too many arguments the arg '%s' was not included\n", split);
            }
            else { //otherwise add the argument and split the command by the next space
                argv[j] = split;
                ++j;
                split = strtok(NULL, " ");
            }
        }
        argv[j] = NULL; //no more arguments
        execvp(cmd, argv); //exacute the command
    } else { //if I am the parent
        wait4(pid, 0, 0, &ru); //wait for the child to finish
    }
    gettimeofday(&t1, NULL); //get the time of day after the command finishes

    //calculate how long the command took to run
    unsigned long long jsEpocht0 = (unsigned long long)(t0.tv_sec) * 1000 + (unsigned long long)(t0.tv_usec) / 1000;
    unsigned long long jsEpocht1 = (unsigned long long)(t1.tv_sec) * 1000 + (unsigned long long)(t1.tv_usec) / 1000;
    unsigned long long elapsedMilliseconds = jsEpocht1 - jsEpocht0;

    //print out the stats
    printf("\n-- Statistics ---\n");
    printf("Elapsed time: %llu milliseconds\n", elapsedMilliseconds);
    printf("Page Faults: %ld\n", ru.ru_majflt);
    printf("Page Faults (reclaimed): %ld\n\n", ru.ru_minflt);
}


/*
a function that runs the "whoami" command and outputs the result
*/
void whoami(){
    int link[2];
    struct timeval t0;
    struct timeval t1;
    gettimeofday(&t0, 0); //get the time of day before it starts running so we can later tell how long it took
    struct rusage ru;
    pid_t pid = fork(); //create a seperate process that will be the one that runs the whoami command


    if (pid == 0) { //if I am the child
        dup2(link[1], STDOUT_FILENO);
        close(link[0]);
        close(link[1]);
        char *cmd = "whoami"; //set the command to be whoami
        char *argv[2];
        argv[0] = "whoami"; //set the first arg to be the command
        argv[1] = NULL; //no more additional args
        execvp(cmd, argv); //run the command
    } else { //if I am the parent
        wait4(pid, 0, 0, &ru); //wait for the child to finish
    }
    gettimeofday(&t1, NULL); //get the time of day after the command finishes

    //calculate how long the command took to run
    unsigned long long jsEpocht0 = (unsigned long long)(t0.tv_sec) * 1000 + (unsigned long long)(t0.tv_usec) / 1000;
    unsigned long long jsEpocht1 = (unsigned long long)(t1.tv_sec) * 1000 + (unsigned long long)(t1.tv_usec) / 1000;
    unsigned long long elapsedMilliseconds = jsEpocht1 - jsEpocht0;

    //print out the stats
    printf("\n-- Statistics ---\n");
    printf("Elapsed time: %llu milliseconds\n", elapsedMilliseconds);
    printf("Page Faults: %ld\n", ru.ru_majflt);
    printf("Page Faults (reclaimed): %ld\n\n", ru.ru_minflt);
}

/*
a function that runs the "last" command and outputs the result
*/
void last(){
    int link[2];
    struct timeval t0;
    struct timeval t1;
    gettimeofday(&t0, 0); //get the time of day before it starts running so we can later tell how long it took
    struct rusage ru;
    pid_t pid = fork(); //create a seperate process that will be the one that runs the command


    if (pid == 0) { //if I am the child
        dup2(link[1], STDOUT_FILENO);
        close(link[0]);
        close(link[1]);
        char *cmd = "last"; //set the command to be last
        char *argv[2];
        argv[0] = "last"; //set the first arg to be the command
        argv[1] = NULL; //no more args
        execvp(cmd, argv); //run the command
    } else { //if I am the parent
        wait4(pid, 0, 0, &ru); //wait for the child to finish
    }
    gettimeofday(&t1, 0); //get the time after the command has run
    long elapsed = (t1.tv_usec - t0.tv_usec) / 1000; //get the change in time

    //print out the stats
    printf("\n-- Statistics ---\n");
    printf("Elapsed time: %ld milliseconds\n", elapsed);
    printf("Page Faults: %ld\n", ru.ru_majflt);
    printf("Page Faults (reclaimed): %ld\n\n", ru.ru_minflt);
}

/*
asks the user for a directory and additional args and then runs the ls command with the given directory and args
*/
void ls(){
    char arg[COMMAND_CHAR_LIMIT]; //allocate space for the user to type args
    char dir[COMMAND_CHAR_LIMIT]; //allocate space for the user to type the directory
    printf("Arguments?: ");
    int i = 0;
    scanf("%c", &arg[i]); //get rid of any unwanted lingering endlines from previus scanfs
    while(i == 0 || (i < COMMAND_CHAR_LIMIT && arg[i-1] != '\n')){ //while there is room in the array and the user does not hit enter keep on recording what the user types
        scanf("%c", &arg[i]);
        ++i;
        if (i == COMMAND_CHAR_LIMIT){ //if the user tries to type too many chars prompt the user, but let the user continue with what they have
            printf("ERROR: too many chars");
        }
    }
    arg[i-1] = '\0'; // append a null terminator
    i = 0;
    printf("Path?: ");
    while((i < COMMAND_CHAR_LIMIT - 1 && dir[i-1] != '\n')){ // do all the same thing but for the dir
        scanf("%c", &dir[i]);
        ++i;
        if (i == COMMAND_CHAR_LIMIT){
            printf("ERROR: too many chars");
        }
    }
    dir[i-1] = '\0';
    if (i == 1){ // if the user did not type anything make it so the cwd will be used
        dir[0] = '.';
        dir[1] = '\0';
    }

    int link[2];
    struct timeval t0;
    struct timeval t1;
    gettimeofday(&t0, 0); //get the time before the command is run
    struct rusage ru;
    pid_t pid = fork(); //fork the proccess so one of them can run the command


    if (pid == 0) { //if I am the child
        printf("\n");
        fflush(stdout);
        dup2(link[1], STDOUT_FILENO);
        close(link[0]);
        close(link[1]);
        char *cmd = "ls"; //set the command to be ls
        char *argv[100]; //set a max of 99 args
        argv[0] = "ls"; //set the first arg to be the command
        if (arg[0] == '\0'){ //if the user did not input any additional args just set the remaining args to be the dir
            argv[1] = dir;
            argv[2] = NULL;
        }
        else{ //if the user did input additional args split the args up by spaces
            int j = 1;
            char *split;
            split = strtok(arg, " "); //split the args by space
            while (split != NULL){ //while there are still more in the string
                if (j == 98){ //if the user has already inputed 98 args, dont add it and notify the user
                    printf("ERROR: too many arguments the arg '%s' was not included", split);
                }
                else { //otherwise add the arg to the array
                    argv[j] = split;
                    ++j;
                    split = strtok(NULL, " ");
                }
            }
            argv[j] = dir; //set the last arg to the dir
            argv[j+1] = NULL; //no more args
        }
        execvp(cmd, argv); //run the command
    } else { //if I am the parent
        wait4(pid, 0, 0, &ru); //wait for the child to finsih running
    }
    gettimeofday(&t1, NULL); //get the time of day after the command finishes

    //calculate how long the command took to run
    unsigned long long jsEpocht0 = (unsigned long long)(t0.tv_sec) * 1000 + (unsigned long long)(t0.tv_usec) / 1000;
    unsigned long long jsEpocht1 = (unsigned long long)(t1.tv_sec) * 1000 + (unsigned long long)(t1.tv_usec) / 1000;
    unsigned long long elapsedMilliseconds = jsEpocht1 - jsEpocht0;

    //print out the stats
    printf("\n-- Statistics ---\n");
    printf("Elapsed time: %llu milliseconds\n", elapsedMilliseconds);
    printf("Page Faults: %ld\n", ru.ru_majflt);
    printf("Page Faults (reclaimed): %ld\n\n", ru.ru_minflt);
}

/*
prompts the user for a new directory and changes the current working dir to what the user inputs
*/
void changeDir(){
    char arg[COMMAND_CHAR_LIMIT]; //alocate space for what the user inputs
    printf("New Directory?: ");
    int i = 0;
    scanf("%c", &arg[i]); //remove any lingering new line input
    while(i == 0 || (i < COMMAND_CHAR_LIMIT && arg[i-1] != '\n')){ //as long as there is space in the string keep on adding what the user types to the string
        scanf("%c", &arg[i]);
        ++i;
        if (i == COMMAND_CHAR_LIMIT) { //but if they type one char too many let the user know
            printf("ERROR: too many chars\n");
        }
    }
    arg[i-1] = '\0'; //add a null terminator
    chdir(arg); //change the cwd
    printf("\n");
}

/*
lets the user know what the cwd is by running the pwd command
*/
void pwd() {
    int link[2];
    pid_t pid = fork(); //create a child proccess to run the command
    if (pid == 0) { //if I am the child
        dup2(link[1], STDOUT_FILENO);
        close(link[0]);
        close(link[1]);
        char *cmd = "pwd"; //set the command to be pwd
        char *argv[2];
        argv[0] = "pwd"; //set the first arg to be the command
        argv[1] = NULL; //no more args
        printf("Directory: ");
        fflush(stdout);
        execvp(cmd, argv); //run the command
    } else { //if I am the parent
        wait(NULL); //wait for the child to finish
        printf("\n");
    }
}

/*
adds a user created command
*/
int add_entry(char e[]) {
    int i = 0;
    for (i = 0; i < COMMAND_LIMIT; i++) { //ittorate through each command untill you find an empty one or run out of space
        if (entries[i] == NULL) { //if you find an empty one fill it with the user created command
            char *temp = malloc(COMMAND_CHAR_LIMIT);
            strncpy(temp, e, COMMAND_CHAR_LIMIT);
            entries[i] = temp;
            entries[i + 1] = NULL;
            return i; //return the index that it was added
        }
    }
    printf("ERROR: too many commands\n"); //let the user know that they have too many commands
    return -1; //return an error response
}

/*
check to see if the input the user types is a valid selection
*/
int valid_selection(const char *c) {
    if (!strcmp(c, "a") || !strcmp(c, "c") || !strcmp(c, "e") || !strcmp(c, "p")){ //check to see if it is one of the valid letter selections
        return 1;
    }
    //check to see if it is a valid number selection
    int i = 0;
    char convert[3];
    for(i = 0; entries[i]  != NULL && i < COMMAND_LIMIT; i++){
        sprintf(convert, "%d", i);
        if (!strcmp(c, convert)){
            return 1;
        }
    }
    return 0;
}



int main() {
    add_entry("whoami"); //add the base commands
    add_entry("last");
    add_entry("ls");
    printf("===== Mid-Day Commander, v1 =====\n");
    while (1) { //while in the application
        printf("G’day, Commander! What command would you like to run?\n");
        char selection[3] = "z";
        while (!valid_selection(selection)) {
            //let the user know what their options are
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
            if (scanf("%s", selection) == EOF) {
                printf("\nLogging you out, Commander.\n");
                return 0;
            }
            if (!valid_selection(selection)) { //if they did not chose a valid selection let the user know
                printf("\nSorry, that's not a valid option. Choose one of the following:\n\n");
            }
        }
        //proccess the input
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
            char cmd[COMMAND_CHAR_LIMIT];
            int i = 0;
            scanf("%c", &cmd[i]);
            while(i == 0 || (i < COMMAND_CHAR_LIMIT && cmd[i-1] != '\n')){
                scanf("%c", &cmd[i]);
                ++i;
                if (i == COMMAND_CHAR_LIMIT) {
                    printf("ERROR: too many chars\n");
                }
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
