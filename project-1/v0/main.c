#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <string.h>

//create constants that will be used throughout the program
#define OPTIONS 2
#define COMMAND_CHAR_LIMIT 1000

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
    gettimeofday(&t1, 0); // get the time after the whoami has run
    long elapsed = (t1.tv_usec - t0.tv_usec) / 1000; //get the change in time

    //print out the stats
    printf("\n-- Statistics ---\n");
    printf("Elapsed time: %ld milliseconds\n", elapsed);
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
    gettimeofday(&t1, 0); //get the time after the command was run
    long elapsed = (t1.tv_usec - t0.tv_usec) / 1000; //calculate how much time elapsed

    //print out the stats
    printf("\n-- Statistics ---\n");
    printf("Elapsed time: %ld milliseconds\n", elapsed);
    printf("Page Faults: %ld\n", ru.ru_majflt);
    printf("Page Faults (reclaimed): %ld\n\n", ru.ru_minflt);
}


int main() {
    printf("===== Mid-Day Commander, v0 =====\n");
    while (1) { //while in the application
        printf("G’day, Commander! What command would you like to run?\n");
        int selection = -2;
        while (selection < 0 || selection > OPTIONS) {
            //let the user know what thier options are
            printf("\t0. whoami : Prints out the result of the whoamicommand\n");
            printf("\t1. last : Prints out the result of the last command\n");
            printf("\t2. ls : Prints out the result of a listing on a user-specified path\n");
            printf("Option?: ");
            scanf("%d", &selection);
            if (selection < 0 || selection > OPTIONS) { //if they did not chose a valid option let the user know
                printf("\nSorry, that's not a valid option. Choose one of the following:\n\n");
            }
        }
        printf("\n");
        //proccess the user's input
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
