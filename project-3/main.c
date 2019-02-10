#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <pthread.h>
#include <assert.h>
#include <math.h>
#include <zconf.h>
#include <syscall.h>

#define TIMEMULT 1.2


int teams;

int ninjas;
int pirates;

int nAvgCostume;
int pAvgCostume;

int nAvgArrive;
int pAvgArrive;

enum kind {Ninja, Pirate, Both, None};

pthread_mutex_t statistics_lock;
pthread_mutex_t door_lock; // enter/leave lock & for linked list lock

pthread_t *tids;

// Locked by enter / leave lock
enum team_status {Ready, Busy};
enum team_status *team_states;

int ninjaIn = 0;
int pirateIn = 0;

struct thread_stats {

};

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

int contains_node(struct node *node){
    if (head == node){
        return  1;
    }
    if (head == NULL){
        return 0;
    }
    struct node *cur_node = head;
    while(cur_node->next != NULL){
        if(cur_node->next == node){
            return 1;
        }
        cur_node = cur_node->next;
    }
    return 0;
}

unsigned long long getCurJSEpoch() {
    struct timeval t;
    gettimeofday(&t, NULL);
    return (unsigned long long)(t.tv_sec) * 1000 + (unsigned long long)(t.tv_usec) / 1000;
}

unsigned long long timeElapsed(struct node *n){
    struct timeval t1;
    gettimeofday(&t1, NULL);

    // you might wonder why we're calling it jsEpoch
    // that's because the normal unix epoch is based with seconds
    // while epochs in javascript are based in milliseconds
    // so if we just wrote epoch, I'd probably be wondering if it's seconds or millis
    // but this way, any web developer will instantly know the units we're talking about
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

// returns seconds of time as a double
double generateTimeBeforeArrival(enum kind race) {
    double ret = 0;
    double a = drand48();
    double b = drand48();
    double z = sqrt(-2 * log(a)) * cos(2 * M_PI * b); // Box-Muller transform
    if (race == Ninja) {
        ret = z*sqrt(nAvgCostume) + nAvgArrive;
    } else {
        ret = z*sqrt(pAvgCostume) + pAvgArrive;
    }
    if (ret < 0){
        return 0;
    }
    return ret;
}

// returns seconds of time as a double
double generateCostumingTime(enum kind race) {
    double ret = 0;
    double a = drand48();
    //printf("generated a a val of %f\n", a);
    double b = drand48();
    //printf("generated a b val of %f\n", b);
    double z = sqrt(-2 * log(a)) * cos(2 * M_PI * b); // Box-Muller transform
    //printf("generated a z val of: %f\n", z);
    if (race == Ninja) {
        ret = z*sqrt(nAvgCostume) + nAvgCostume;
    } else {
        ret = z*sqrt(pAvgCostume) + pAvgCostume;
    }
    if (ret < 0){
        return 0;
    }
    return ret;
}

/*
 * Struct for stats
 */

void enterStore(enum kind race) {
    if (race == Ninja) {
        ninjaIn++;
    } else {
        pirateIn++;
    }
}

void leaveStore(enum kind race) {
    if (race == Ninja){
        ninjaIn--;
    }
    else{
        pirateIn--;
    }
}

int getCostumingTeam() {
    int costumingTeam;
    for (costumingTeam = 0; costumingTeam < teams; costumingTeam++) {
        if (team_states[costumingTeam] == Ready) {
            team_states[costumingTeam] = Busy;
            break;
        }
    }
    return costumingTeam;
}

void releaseCostumingTeam(int costumingTeam) {
    team_states[costumingTeam] = Ready;
}

/**
 *
 * @param race of type enum kind
 * @return
 */
void *thread(void *r) {
    int nextVisit = 0;
    int hasEntered = 0;
    int costumingTeam;
    pid_t tid = syscall(SYS_gettid);
    printf("%d: Spawned\n", tid);
    double costumingTime;
    struct node *n;
    pthread_cond_t c = PTHREAD_COND_INITIALIZER;
    enum kind race = (enum kind) r;
    // while true
    while (1) {
        // get random amount of time that should sleep
        usleep(generateTimeBeforeArrival(race) * 1000 * 1000);
        // upon waking up
        // acquire enter/leave lock
        pthread_mutex_lock(&door_lock);
        // create node
        n = malloc(sizeof(struct node));
        n->start = getCurJSEpoch();
        n->next = NULL;
        n->type = race;
        n->cond = &c;
        while(hasEntered == 0){
            // check if story empty or checkPriority() == race or (checkPriority() == Neutral && everyone in store == race)
            printf("%d: %s: gonna check if can enter\n", tid, (race == Ninja) ? "Ninja" : "Pirate");
            if (canEnter() == race || canEnter() == Both) {
                printf("%d: %s: I can enter!\n", tid, (race == Ninja) ? "Ninja" : "Pirate");
                // enter store
                hasEntered = 1;
                enterStore(race);
                printf("%d: %s: entered store!\n", tid, (race == Ninja) ? "Ninja" : "Pirate");
                // get costuming team
                printf("%d: %s: getting costuming team\n", tid, (race == Ninja) ? "Ninja" : "Pirate");
                costumingTeam = getCostumingTeam();
                printf("%d: %s: got costuming team\n", tid, (race == Ninja) ? "Ninja" : "Pirate");
                // release lock
                printf("%d: %s: releasing lock\n", tid, (race == Ninja) ? "Ninja" : "Pirate");
                pthread_mutex_unlock(&door_lock);
                printf("%d: %s: released lock\n", tid, (race == Ninja) ? "Ninja" : "Pirate");
                if(n->next != NULL) {
                    printf("%d: %s: signaling\n", tid, (race == Ninja) ? "Ninja" : "Pirate");
                    pthread_cond_signal(n->next->cond);
                    printf("%d: %s: done signaling\n", tid, (race == Ninja) ? "Ninja" : "Pirate");
                }
                // sleep while we get help
                costumingTime = generateCostumingTime(race);
                printf("%d: %s: about to sleep for %f\n", tid, (race == Ninja) ? "Ninja" : "Pirate", costumingTime);
                usleep(costumingTime * 1000 * 1000);
                printf("%d: %s: woke up after costuming\n", tid, (race == Ninja) ? "Ninja" : "Pirate");
                // acquire lock
                printf("%d: %s: getting lock...\n", tid, (race == Ninja) ? "Ninja" : "Pirate");
                pthread_mutex_lock(&door_lock);
                printf("%d: %s: got the lock!\n", tid, (race == Ninja) ? "Ninja" : "Pirate");
                printf("%d: %s: leaving store\n", tid, (race == Ninja) ? "Ninja" : "Pirate");
                leaveStore(race);
                printf("%d: %s: left the store\n", tid, (race == Ninja) ? "Ninja" : "Pirate");
                releaseCostumingTeam(costumingTeam);
                printf("%d: %s: released costuming team\n", tid, (race == Ninja) ? "Ninja" : "Pirate");
                // write down stats

                // loop through linked list signaling everyone
                printf("%d: %s: checking if head exists\n", tid, (race == Ninja) ? "Ninja" : "Pirate");
                if(head != NULL){
                    printf("%d: %s: head exists.. gonna signal\n", tid, (race == Ninja) ? "Ninja" : "Pirate");
                    pthread_cond_signal(head->cond);
                    printf("%d: %s: head exists.. done signaling\n", tid, (race == Ninja) ? "Ninja" : "Pirate");
                }

                // release lock
                printf("%d: %s: releasing lock...\n", tid, (race == Ninja) ? "Ninja" : "Pirate");
                pthread_mutex_unlock(&door_lock);
                printf("%d: %s: released lock!\n", tid, (race == Ninja) ? "Ninja" : "Pirate");
                // return / check if should come back
                if(1) {//drand48() > 0.25 || nextVisit){
                    if (race == Ninja) {
                        printf("%d: Ninja: Bye bye!\n", tid);
                    } else {
                        printf("%d: Pirate: Bye bye!\n", tid);
                    }
                    return 0;
                }
                nextVisit = 1;
            // else
            } else {
                printf("%d: %s: I can't enter :(\n", tid, (race == Ninja) ? "Ninja" : "Pirate");
                if(!contains_node(n)){
                    printf("%d: %s: added my node to list\n", tid, (race == Ninja) ? "Ninja" : "Pirate");
                    addNode(n);
                }

                if(n->next != NULL) {
                    printf("%d: %s: signaling next\n", tid, (race == Ninja) ? "Ninja" : "Pirate");
                    pthread_cond_signal(n->next->cond);
                    printf("%d: %s: done signaling next\n", tid, (race == Ninja) ? "Ninja" : "Pirate");
                }
                printf("%d: %s: waiting to be signaled\n", tid, (race == Ninja) ? "Ninja" : "Pirate");
                pthread_cond_wait(&c, &door_lock);
                printf("%d: %s: got signaled!\n", tid, (race == Ninja) ? "Ninja" : "Pirate");
            }
            // release lock
            // sleep and get signaled
        }
        return 0;
    }
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
