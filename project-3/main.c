#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <assert.h>

int teams;

int ninjas;
int pirates;

int nAvgCostume;
int pAvgCostume;

int nAvgArrive;
int pAvgArrive;

pthread_mutex_t statistics_lock;

pthread_t *tids;

// Locked by enter / leave lock
enum team_status {Ready, Busy};
enum team_status *team_states;

enum kind {Ninja, Pirate, Neutral};
enum kind priority = Neutral;

/*
 * Struct for stats
 */

struct thread_stats {

};

struct node {

};

/*
 * Linked list struct
 */

/*
 * Linked List add node
 */

/*
 * Linked List remove node
 */

/*
 * Lock for list list
 */

/**
 *
 * @param race of type enum kind
 * @return
 */
void *thread(void *r) {
    enum kind race = (enum kind) r;
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
    int i;
    int j;
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
     * Initialization
     */
    assert(pthread_mutex_init(&statistics_lock, NULL) == 0);
    tids = malloc(sizeof (pthread_t) * (ninjas + pirates));
    for (i = 0; i < (ninjas + pirates); i++) {
        tids[i] = 0;
    }
    // Initialize team states as Ready
    team_states = malloc(sizeof (enum team_status) * teams);
    for (i = 0; i < teams; i++) {
        team_states[i] = Ready;
    }

    /*
     * Spawn all threads
     */
    int status;
    for (i = 0; i < ninjas; i++) {
        status = pthread_create(&tids[i], NULL, thread, (void *) Ninja);
        if (status != 0) {
            printf("Error Creating PThread\n");
        }
    }
    for (j = 0; j < pirates; j++) {
        status = pthread_create(&tids[i + j], NULL, thread, (void *) Pirate);
        if (status != 0) {
            printf("Error Creating PThread\n");
        }
    }

    /*
     * Wait for all threads to finish
     */
    for (i = 0; i < (ninjas + pirates); i++) {
        if (tids[i] != 0) {
            pthread_join(tids[i], NULL);
        }
    }

    /*
     * Loop through and summarize stats
     */
    printf("Hello, World!\n");
    return 0;
}
