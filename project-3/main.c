#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#define TIMEMULT 1.2

int teams;

int ninjas;
int pirates;

int nAvgCostume;
int pAvgCostume;

int nAvgArrive;
int pAvgArrive;

enum kind {Ninja, Pirate, Both, None};

int ninjaIn = 0;
int pirateIn = 0;

struct node {
    enum kind type;
    pthread_cond_t *cond;
    unsigned long long start;
    struct node *next;
};

struct node *head = NULL;

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

int addNode(struct node *node){
    if(node == NULL){
        return 0;
    }
    if (head == NULL){
        head = node;
        return 1;
    }
    struct node *cur_node = head;
    while(cur_node->next != NULL){
        cur_node = cur_node->next;
    }
    cur_node->next = node;
    return 1;
}

int delete_node_by_pointer(struct node *node){
    if (head == node){
        head = head->next;
        free(node);
        return  1;
    }
    struct node *cur_node = head;
    while(cur_node->next != NULL){
        if(cur_node->next == node){
            cur_node->next = cur_node->next->next;
            free(node);
            return 1;
        }
        cur_node = cur_node->next;
    }
    return 0;
}

unsigned long long timeElapsed(struct node *n){
    struct timeval t1;
    gettimeofday(&t1, NULL);

    unsigned long long jsEpocht0 = n->start * 1000 + n->start / 1000;
    unsigned long long jsEpocht1 = (unsigned long long)(t1.tv_sec) * 1000 + (unsigned long long)(t1.tv_usec) / 1000;
    return jsEpocht1 - jsEpocht0;
}

enum kind canEnter(){
    if (ninjaIn == teams || pirateIn == teams){
        return None;
    }
    int currentlyIn = 0; //0 is for none, 1 is for ninjas, 2 is for pirates
    if  (ninjaIn > 0){
        currentlyIn = 1;
    }
    else if (pirateIn > 0){
        currentlyIn = 2;
    }
    struct node *cur_node = head;
    while (cur_node != NULL){
        if (cur_node->type == Pirate && (timeElapsed(cur_node) > 30000 - nAvgCostume*1000*TIMEMULT)){
            if(currentlyIn == 0 || currentlyIn == 2){
                return Pirate;
            }
            return None;
        }
        else if (cur_node->type == Ninja && (timeElapsed(cur_node) > 30000 - pAvgCostume*1000*TIMEMULT)){
            if(currentlyIn == 0 || currentlyIn == 1){
                return Ninja;
            }
            return None;
        }
    }
    if (currentlyIn == 0){
        return Both;
    } 
    else if (currentlyIn == 1){
        return Ninja;
    }
    return Pirate;
}

/*
 * Struct for stats
 */

void *thread(enum kind race) {
    // get random amount of time that should sleep
    // upon waking up
    // while true
        // acquire enter/leave lock
            // check if story empty or checkPriority() == race or (checkPriority() == Neutral && everyone in store == race)
                // enter store
                // remove self from linked list
                // release lock
                // sleep while we get help
                // acquire lock
                // write down stats
                // loop through linked list signaling everyone
                // release lock
                // return
            // else
                // release lock
                // sleep and get signaled
}


int main(int argc, char** argv) {
    if (argc != 8 || !( 
       (atoi(argv[1]) >= 2 && atoi(argv[1]) <= 4) && 
       (atoi(argv[2]) >= 10 && atoi(argv[2]) <= 50) && 
       (atoi(argv[3]) >= 10 && atoi(argv[3]) <= 50) &&
       (atoi(argv[4]) != 0 || argv[4][0] == '0') &&
       (atoi(argv[5]) != 0 || argv[5][0] == '0') &&
       (atoi(argv[6]) != 0 || argv[6][0] == '0') &&
       (atoi(argv[7]) != 0 || argv[7][0] == '0'))){
        printf("wrong args\n");
        return -1;
    }
    teams = atoi(argv[1]);
    ninjas = atoi(argv[2]);
    pirates = atoi(argv[3]);
    nAvgCostume = atoi(argv[4]);
    pAvgCostume = atoi(argv[5]);
    nAvgArrive = atoi(argv[6]);
    pAvgArrive = atoi(argv[7]);

    /*
     * Spawn all threads
     */

    /*
     * Wait for all threads to finish
     */

    /*
     * Loop through and summarize stats
     */
    
    printf("Hello, World!\n");
    return 0;
}
