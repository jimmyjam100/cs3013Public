#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <string.h>
#include <pthread.h>
#include <assert.h>

//set the cammand char limit and the ammount of commands that the user can create
#define COMMAND_CHAR_LIMIT 1000
#define COMMAND_LIMIT 100

char *entries[COMMAND_LIMIT]; //create the array for user created commands

struct node{ //create the struct for the linked list of active commands being run on another thread
    pid_t pid;
    struct node *next;
    char *cmd;
};

struct node *head; //create a pointer for the first value of the linked list

pthread_mutex_t lock; //create a lock for the linked list so two threads can't change it at once

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
    gettimeofday(&t1, 0); // get the time after the whoami has run
    long elapsed = (t1.tv_usec - t0.tv_usec) / 1000; //get the change in time

    //print out the stats
    printf("\n-- Statistics ---\n");
    printf("Elapsed time: %ld milliseconds\n", elapsed);
    printf("Page Faults: %ld\n", ru.ru_majflt);
    printf("Page Faults (reclaimed): %ld\n\n", ru.ru_minflt);
}

void *threaded_whoami(void *tid) {
    //printf("\nthread\n");
    printf("\n");
    whoami();
    pthread_exit(NULL);
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
    fflush(stdout);
    int i = 0;
    scanf("%c", &arg[i]); //get rid of any unwanted lingering endlines from previus scanfs
    while(i == 0 || (i < COMMAND_CHAR_LIMIT && arg[i-1] != '\n')){ //while there is room in the array and the user does not hit enter keep on recording what the user types
        scanf("%c", &arg[i]);
        ++i;
        if (i == COMMAND_CHAR_LIMIT){ //if the user tries to type too many chars prompt the user, but let the user continue with what they have
            printf("ERROR: too many chars");
            fflush(stdout);
        }
    }
    arg[i-1] = '\0'; // append a null terminator
    i = 0;
    printf("Path?: ");
    fflush(stdout);
    while((i < COMMAND_CHAR_LIMIT - 1 && dir[i-1] != '\n')){ // do all the same thing but for the dir
        scanf("%c", &dir[i]);
        ++i;
        if (i == COMMAND_CHAR_LIMIT){
            printf("ERROR: too many chars");
            fflush(stdout);
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
        //dup2(link[1], STDOUT_FILENO);
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
                    fflush(stdout);
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
lists all the processes that are currently running in another thread
*/
void list_processes() {
    pthread_mutex_lock(&lock); //lock the linked list so nothing can edit it while this is running
    int i = 0;
    struct node * cur_node = head; //get a pointer to the head to ittorate through the linked list
    while (cur_node != NULL){ //while there are still processes
        printf("[%d] %d (%s) \n", i, cur_node->pid, cur_node->cmd); //print out the details about that process
        cur_node = cur_node->next; //ittorate
        i++;
    }
    pthread_mutex_unlock(&lock); //unlock the lock
    printf("\n");
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
    if (!strcmp(c, "a") || !strcmp(c, "c") || !strcmp(c, "e") || !strcmp(c, "p") || !strcmp(c, "r")){ //check to see if it is one of the valid letter selections
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

/*
check to see if a string ends with &
*/
int str_ends_with_ampersand(char *s) {
    int i;
    char last = s[0];
    //get the last non space char
    for (i = 0; s[i] != '\0'; i++){
        if (s[i] != ' ') {
            last = s[i];
        }
    }
    return last == '&'; //return 1 if the last non space char is &
}

/*
create a new entry in the linked list and return its index
*/
int create_node(pid_t pid, char *cmd) {
    struct node *new_node = (struct node *) malloc(sizeof(struct node)); //allocate space for a new node
    new_node->pid = pid; //set its value
    new_node->next = NULL; //it will be the last in the list
    new_node->cmd = cmd; // sets the command value
    struct node *cur_node = head; //create an ittorator
    if (head == NULL){ //if there are no entries set the first entry to be the head
        head = new_node;
        return 0;
    }
    int i = 0;
    while (cur_node->next != NULL){ //whiel there are still more nodes ittorate through
        cur_node = cur_node->next;
        i++;
    }
    cur_node->next = new_node; //set the next node to be the new node
    return i+1;
}

/*
delete a node at a given index
*/
int delete_node(int index){
    int i = 0;
    struct node *cur_node = head; //create an ittorator
    if (head == NULL){ //if there are no entries return an error
        return 0;
    }
    if (index == 0){ //if they want to delete the head take care of that edge case
        struct node *save = head;
        head = head->next;
        free(save);
        return 1;
    }
    for(i = 0; i < index - 1 && cur_node != NULL; i++){ //itorate through the node untill the index is reached
        cur_node = cur_node->next;
    }
    if(cur_node == NULL || cur_node->next == NULL){ //if the index is not in the linked list return error
        return 0;
    }
    //actully delete it here
    struct node *save = cur_node->next;
    cur_node->next = cur_node->next->next;
    free(save);
    return 1;
}

/*
delete node by pid
*/
int delete_node_by_pid_t(pid_t pid) {
    struct node *cur_node = head; //create an ittorator
    if (head == NULL){ //if there are no entries return an error
        return 0;
    }
    if (head->pid == pid){ //if they want to delete the head take care of that edge case
        struct node *save = head;
        head = head->next;
        free(save);
        return 1;
    }
    while(cur_node->next != NULL && cur_node->next->pid != pid){ //ittorate through the nodes untill the pid is reached
        cur_node = cur_node->next;
    }
    if(cur_node == NULL || cur_node->next == NULL){ //if the pid is not in the linked list return error
        return 0;
    }
    //delete the node
    struct node *save = cur_node->next;
    cur_node->next = cur_node->next->next;
    free(save);
    return 1;

}

/*
gets the index of a node given you input its pid
*/
int get_index_of_node(pid_t pid){
    int i = 0;
    struct node *cur_node = head; //create an ittorator
    while(cur_node != NULL){ //while there are still entries
        if (cur_node->pid == pid){ //if the cur_node is the one we are looking for return its index
            return i;
        }
        cur_node = cur_node->next; //ittorate
        i++;
    }
    return -1; //return error
}
/*
the command to run a user created command but instead make it a seperate thread
*/
void *threaded_user_created(void *cmd2){
    int link[2];
    struct timeval t0;
    struct timeval t1;
    gettimeofday(&t0, 0); //get the time of day before the command was run
    struct rusage ru;
    pid_t pid = fork(); //create a child process to run the command


    if (pid == 0) { //if I am the child
        dup2(link[1], STDOUT_FILENO);
        close(link[0]);
        close(link[1]);
        char *argv[100];
        int j = 0;
        char *split;
        char input[COMMAND_CHAR_LIMIT];
        strncpy(input, cmd2, COMMAND_CHAR_LIMIT); //copy the value of the command so we dont end up editing the actull string
        split = strtok(input, " "); //split the string up by spaces to get the args
        char *cmd = split; //set the command to be the first substring
        while (split != NULL){  //for every substring
            if (j == 99){ //if the user inputs too many args tell the user there was an error
                printf("ERROR: too many arguments the arg '%s' was not included\n", split);
            }
            else { //otherwise add the substring as an arg
                argv[j] = split;
                ++j;
                split = strtok(NULL, " ");
            }
        }
        argv[j - 1] = NULL; //replace the & with a NULL terminator
        execvp(cmd, argv); //run the command
    } else { //if I am the parent
        char input[COMMAND_CHAR_LIMIT];
        strncpy(input, cmd2, COMMAND_CHAR_LIMIT); //copy the value of the command so we dont end up editing the actull string
        pthread_mutex_lock(&lock); //lock the linked list so only I can edit it
        create_node(pid, input); //create the node in the linked list to keep track of it
        pthread_mutex_unlock(&lock); //unlock the linked list while we wait for the child to finish


        wait4(pid, 0, 0, &ru); //wait for the child to finish


        pthread_mutex_lock(&lock); //lock the linked list so only I can edit it
        delete_node_by_pid_t(pid); //delete the node
        pthread_mutex_unlock(&lock); //unlock the linked list
    }
    gettimeofday(&t1, 0); //get the time of day after the command was run

    //calculate how much time it took to run the command
    unsigned long long jsEpocht0 = (unsigned long long)(t0.tv_sec) * 1000 + (unsigned long long)(t0.tv_usec) / 1000;
    unsigned long long jsEpocht1 = (unsigned long long)(t1.tv_sec) * 1000 + (unsigned long long)(t1.tv_usec) / 1000;
    unsigned long long elapsedMilliseconds = jsEpocht1 - jsEpocht0;

    //print out the stats
    printf("\n-- Statistics ---\n");
    printf("Elapsed time: %llu milliseconds\n", elapsedMilliseconds);
    printf("Page Faults: %ld\n", ru.ru_majflt);
    printf("Page Faults (reclaimed): %ld\n\n", ru.ru_minflt);
}

int main() {
    assert(pthread_mutex_init(&lock, NULL) == 0); //initialize the linked list lock
    add_entry("whoami"); //add the 3 base commands to the command list
    add_entry("last");
    add_entry("ls");
    printf("===== Mid-Day Commander, v2 =====\n");
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
            printf("\tr. running processes : Prints list of running processes\n");
            printf("Option?: ");
            if (scanf("%s", selection) == EOF) {
                printf("\nLogging you out, Commander.\n");
                return 0;
            }
            if (!valid_selection(selection)) { //make sure the user chooses an vailid choice
                printf("\nSorry, that's not a valid option. Choose one of the following:\n\n");
            }
        }
        //processes the result
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
        } else if (!strcmp(selection, "r")) {
            printf("\n-- Background Processes --\n");
            list_processes();
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
            char* cmd = entries[atoi(selection)];
            if (str_ends_with_ampersand(cmd)) {
                pthread_t thread;
                int status, i;
                i = 0;
                status = pthread_create(&thread, NULL, threaded_user_created, (void *)cmd);
                if (status != 0) {
                    printf("Error creating thread\n");
                }
                usleep(2000);
            } else {
                userCreated(atoi(selection));
            }
            //printf("Sorry, that command isn't supported yet\n\n");
        }
    }
    return 0;
}
