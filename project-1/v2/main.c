#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <string.h>
#include <pthread.h>
#include <assert.h>

#define COMMAND_CHAR_LIMIT 1000
#define COMMAND_LIMIT 100

char *entries[COMMAND_LIMIT];

struct node{
    pid_t pid;
    struct node *next;
};

struct node *head;

pthread_mutex_t lock;

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
        char input[COMMAND_CHAR_LIMIT];
        strncpy(input, entries[id], COMMAND_CHAR_LIMIT);
        split = strtok(input, " ");
        char *cmd = split;
        while (split != NULL){
            if (j == 99){
                printf("ERROR: too many arguments the arg '%s' was not included\n", split);
            }
            else {
                argv[j] = split;
                ++j;
                split = strtok(NULL, " ");
            }
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

void *threaded_whoami(void *tid) {
    //printf("\nthread\n");
    printf("\n");
    whoami();
    pthread_exit(NULL);
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
    char arg[COMMAND_CHAR_LIMIT];
    char dir[COMMAND_CHAR_LIMIT];
    printf("Arguments?: ");
    int i = 0;
    scanf("%c", &arg[i]);
    while(i == 0 || (i < COMMAND_CHAR_LIMIT && arg[i-1] != '\n')){
        scanf("%c", &arg[i]);
        ++i;
        if (i == COMMAND_CHAR_LIMIT) {
            printf("ERROR: too many chars\n");
        }
    }
    arg[i-1] = '\0';
    i = 0;
    printf("Path?: ");
    while((i < COMMAND_CHAR_LIMIT && dir[i-1] != '\n')){
        scanf("%c", &dir[i]);
        ++i;
        if (i == COMMAND_CHAR_LIMIT) {
            printf("ERROR: too many chars\n");
        }
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
                if (j == 98){
                    printf("ERROR: too many arguments the arg '%s' was not included\n", split);
                }
                else {
                    argv[j] = split;
                    ++j;
                    split = strtok(NULL, " ");
                }
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
    char arg[COMMAND_CHAR_LIMIT];
    printf("New Directory?: ");
    int i = 0;
    scanf("%c", &arg[i]);
    while(i == 0 || (i < COMMAND_CHAR_LIMIT && arg[i-1] != '\n')){
        scanf("%c", &arg[i]);
        ++i;
        if (i == COMMAND_CHAR_LIMIT) {
            printf("ERROR: too many chars\n");
        }
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

void list_processes() {
    pthread_mutex_lock(&lock);
    int i = 0;
    struct node * cur_node = head;
    while (cur_node != NULL){
        printf("[%d] %d\n", i, cur_node->pid);
        cur_node = cur_node->next;
        i++;
    }
    pthread_mutex_unlock(&lock);
    printf("\n");
}

int add_entry(char e[]) {
    int i = 0;
    for (i = 0; i < COMMAND_LIMIT; i++) {
        if (entries[i] == NULL) {
            char *temp = malloc(COMMAND_CHAR_LIMIT);
            strncpy(temp, e, COMMAND_CHAR_LIMIT);
            entries[i] = temp;
            entries[i + 1] = NULL;
            return i;
        }
    }
    printf("ERROR: too many commands\n");
    return -1;
}

int valid_selection(const char *c) {
    if (!strcmp(c, "a") || !strcmp(c, "c") || !strcmp(c, "e") || !strcmp(c, "p")){
        return 1;
    }
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

int str_ends_with_ampersand(char *s) {
    int i;
    char last = s[0];
    for (i = 0; s[i] != '\0'; i++){
        if (s[i] != ' ') {
            last = s[i];
        }
    }
    return last == '&';
}

int create_node(pid_t pid) {
    struct node *new_node = (struct node *) malloc(sizeof(struct node));
    new_node->pid = pid;
    new_node->next = NULL;
    struct node *cur_node = head;
    if (head == NULL){
        head = new_node;
        return 0;
    }
    int i = 0;
    while (cur_node->next != NULL){
        cur_node = cur_node->next;
        i++;
    }
    cur_node->next = new_node;
    return i+1;
}

int delete_node(int index){
    int i = 0;
    struct node *cur_node = head;
    if (head == NULL){
        return 0;
    }
    if (index == 0){
        struct node *save = head;
        head = head->next;
        free(save);
        return 1;
    }
    for(i = 0; i < index - 1 && cur_node != NULL; i++){
        cur_node = cur_node->next;
    }
    if(cur_node == NULL || cur_node->next == NULL){
        return 0;
    }
    struct node *save = cur_node->next;
    cur_node->next = cur_node->next->next;
    free(save);
    return 1;
}

int delete_node_by_pid_t(pid_t pid) {
    struct node *cur_node = head;
    if (head == NULL){
        return 0;
    }
    if (head->pid == pid){
        struct node *save = head;
        head = head->next;
        free(save);
        return 1;
    }
    while(cur_node->next != NULL && cur_node->next->pid != pid){
        cur_node = cur_node->next;
    }
    if(cur_node == NULL || cur_node->next == NULL){
        return 0;
    }
    struct node *save = cur_node->next;
    cur_node->next = cur_node->next->next;
    free(save);
    return 1;

}

int get_index_of_node(pid_t pid){
    int i = 0;
    struct node *cur_node = head;
    while(cur_node != NULL){
        if (cur_node->pid == pid){
            return i;
        }
        cur_node = cur_node->next;
        i++;
    }
    return -1;
}

void *threaded_user_created(void *cmd2){
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
        char input[COMMAND_CHAR_LIMIT];
        strncpy(input, cmd2, COMMAND_CHAR_LIMIT);
        split = strtok(input, " ");
        char *cmd = split;
        while (split != NULL){
            if (j == 99){
                printf("ERROR: too many arguments the arg '%s' was not included\n", split);
            }
            else {
                argv[j] = split;
                ++j;
                split = strtok(NULL, " ");
            }
        }
        argv[j - 1] = NULL;
        execvp(cmd, argv);
    } else {
        pthread_mutex_lock(&lock);
        printf("test print 1\n");
        create_node(pid);
        pthread_mutex_unlock(&lock);


        wait4(pid, 0, 0, &ru);


        pthread_mutex_lock(&lock);
        printf("test print 2\n"); //TODO print stuff
        delete_node_by_pid_t(pid);
        pthread_mutex_unlock(&lock);
    }
    gettimeofday(&t1, 0);
    long elapsed = (t1.tv_usec - t0.tv_usec) / 1000;

    printf("\n-- Statistics ---\n");
    printf("Elapsed time: %ld milliseconds\n", elapsed);
    printf("Page Faults: %ld\n", ru.ru_majflt);
    printf("Page Faults (reclaimed): %ld\n\n", ru.ru_minflt);
}

int main() {
    assert(pthread_mutex_init(&lock, NULL) == 0);
    add_entry("whoami");
    add_entry("last");
    add_entry("ls");
    printf("===== Mid-Day Commander, v2 =====\n");
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
            printf("\tr. running processes : Prints list of running processes\n");
            printf("Option?: ");
            if (scanf("%s", selection) == EOF) {
                printf("\nLogging you out, Commander.\n");
                return 0;
            }
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
