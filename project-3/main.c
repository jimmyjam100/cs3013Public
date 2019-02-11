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

unsigned int revenue;

pthread_mutex_t statistics_lock;
pthread_mutex_t door_lock; // enter/leave lock & for linked list lock

pthread_t *tids;

// Locked by enter / leave lock
enum team_status {Ready, Busy};
enum team_status *team_states;

unsigned long long *time_teams_busy_for;

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

struct visit_stats {
    unsigned long long visitTime;
    unsigned long long waitTime;
    struct visit_stats *next;
    
};

struct person_stats {
    int visits;
    int goldOwed;
    enum kind race;
    struct visit_stats *head;
};

struct person_stats **persons;



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
    if (cur_node == NULL) {
        return 0;
    }
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
    return ((unsigned long long)(t.tv_sec) * 1000 + (unsigned long long)(t.tv_usec) / 1000);
}

unsigned long long timeElapsed(struct node *n){
    struct timeval t1;
    gettimeofday(&t1, NULL);

    // you might wonder why we're calling it jsEpoch
    // that's because the normal unix epoch is based with seconds
    // while epochs in javascript are based in milliseconds
    // so if we just wrote epoch, I'd probably be wondering if it's seconds or millis
    // but this way, any web developer will instantly know the units we're talking about
    unsigned long long jsEpocht0 = n->start;//n->start * 1000 + n->start / 1000;
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
    struct person_stats *stats = malloc(sizeof(struct person_stats));
    stats->visits = 0;
    stats->goldOwed = 0;
    int nextVisit = 0;
    int hasEntered = 0;
    int costumingTeam;
    pid_t tid = syscall(SYS_gettid);
    printf("%d: Spawned\n", tid);
    double costumingTime;
    struct node *n;
    pthread_cond_t c = PTHREAD_COND_INITIALIZER;
    enum kind race = (enum kind) r;
    stats->race = race;
    // while true
    while (1) {
        hasEntered = 0;
        // get random amount of time that should sleep
        usleep(generateTimeBeforeArrival(race) * 1000 * 1000);

        // create node
        n = malloc(sizeof(struct node));
        n->start = getCurJSEpoch();
        n->next = NULL;
        n->type = race;
        n->cond = &c;
        // upon waking up
        // acquire enter/leave lock
        pthread_mutex_lock(&door_lock);
        while(hasEntered == 0){
            // check if story empty or checkPriority() == race or (checkPriority() == Neutral && everyone in store == race)
            printf("%d: %s: gonna check if can enter\n", tid, (race == Ninja) ? "Ninja" : "Pirate");
            if (canEnter() == race || canEnter() == Both) {
                unsigned long long waitTime = timeElapsed(n);
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
                // adding to revenue stats
                printf("%d: %s: acquiring statistics lock\n", tid, (race == Ninja) ? "Ninja" : "Pirate");
                pthread_mutex_lock(&statistics_lock);
                time_teams_busy_for[costumingTeam]+=costumingTime*1000*1000;
                if ((waitTime/1000) < 30){
                    revenue = revenue + ceil(costumingTime);
                    printf("%d: %s: Revenue is now %d\n", tid, (race == Ninja) ? "Ninja" : "Pirate", revenue);
                    stats->goldOwed = stats->goldOwed + ceil(costumingTime);
                }
                stats->visits = stats->visits + 1;
                struct visit_stats *new = malloc(sizeof(struct visit_stats));
                new->visitTime = costumingTime;
                new->waitTime = waitTime;
                new->next = NULL;

                if (stats->head == NULL){
                    stats->head = new;
                } else {
                    struct visit_stats *cur = stats->head;
                    while(cur->next != NULL){
                        cur = cur->next;
                    }
                    cur->next = new;
                }

                printf("%d: %s: releasing statistics lock\n", tid, (race == Ninja) ? "Ninja" : "Pirate");
                pthread_mutex_unlock(&statistics_lock);
                // acquire locklock
                printf("%d: %s: getting lock...\n", tid, (race == Ninja) ? "Ninja" : "Pirate");
                pthread_mutex_lock(&door_lock);
                printf("%d: %s: got the lock!\n", tid, (race == Ninja) ? "Ninja" : "Pirate");
                printf("%d: %s: leaving store\n", tid, (race == Ninja) ? "Ninja" : "Pirate");
                leaveStore(race);
                printf("%d: %s: left the store\n", tid, (race == Ninja) ? "Ninja" : "Pirate");
                releaseCostumingTeam(costumingTeam);
                printf("%d: %s: released costuming team\n", tid, (race == Ninja) ? "Ninja" : "Pirate");
                // write down stats
                printf("%d: %s: removing self from list\n", tid, (race == Ninja) ? "Ninja" : "Pirate");
                delete_node_by_pointer(n);
                printf("%d: %s: removed!\n", tid, (race == Ninja) ? "Ninja" : "Pirate");

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
                if(drand48() > 0.25 || nextVisit) {
                    pthread_mutex_lock(&statistics_lock);
                    int ittorator = 0;
                    while(persons[ittorator] != NULL){
                        ittorator++;
                    }
                    persons[ittorator] = stats;
                    pthread_mutex_unlock(&statistics_lock);
                    if (race == Ninja) {
                        printf("%d: Ninja: Bye bye!\n", tid);
                    } else {
                        printf("%d: Pirate: Bye bye!\n", tid);
                    }
                    return 0;
                }
                printf("%d: %s: Going to come back!\n", tid, (race == Ninja) ? "Ninja" : "Pirate");
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
    time_t startTime;
    startTime = time(NULL);
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

    persons = malloc(sizeof (struct persons_stats*)*(ninjas + pirates));
    time_teams_busy_for = malloc(sizeof(unsigned long long)*teams);
    for (i = 0; i < teams; i++){
        time_teams_busy_for[i] = 0;
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
    time_t endTime = time(NULL);
    long totalTime = endTime - startTime;

    /*
     * Loop through and summarize stats
     */
    int ninjaCounter = 0;
    int pirateCounter = 0;
    int numOfVisits = 0;
    unsigned long long totalWaitTime = 0;
    for(i = 0; i < pirates + ninjas; i++){
        if(persons[i]->race == Ninja){
            ninjaCounter++;
            printf("\n\nnow printing stats for Ninja #%d\n", ninjaCounter);
        }
        else {
            pirateCounter++;
            printf("\n\nnow printing stats for Pirate #%d\n", pirateCounter);
        }
        printf("\tgold owed: %d\n", persons[i]->goldOwed);
        printf("\tnumber of visits: %d\n", persons[i]->visits);
        int visitCounter = 0;
        struct visit_stats *cur_visit = persons[i]->head;
        while(cur_visit != NULL){
            numOfVisits++;
            visitCounter++;
            totalWaitTime += cur_visit->waitTime;
            printf("\tvisit #%d:\n", visitCounter);
            printf("\t\thow long the wait was: %llu secs\n", (((cur_visit->waitTime)*60)/1000));
            printf("\t\thow long the visit was: %llu secs\n",  (cur_visit->visitTime)*60);
            cur_visit = cur_visit->next;
        }
    }
    for(i = 0; i < teams; i++){
        printf("Team #%d\n", i+1);
        printf("\ttime spent busy: %llu secs\n", ((time_teams_busy_for[i]*60)/1000)/1000);
        printf("\ttime spent idle: %llu secs\n", (totalTime*60 - ((time_teams_busy_for[i]*60)/1000)/1000));
    }
    int days = ceil(((double)totalTime)/(60*24));
    int costs = days*5*teams;
    printf("spent a total of %d gold coins to pay for the %d teams over a course of %d days\n", costs, teams, days);
    printf("total ammount of gold earned: %d\n", revenue);
    double avgQueue = ((((double)totalWaitTime)/1000))/((double)totalTime);
    printf("average queue length: %f\n", avgQueue);
    printf("gold per visit: %d\n", revenue/numOfVisits);
    printf("total profits: %d\n", revenue-costs);


    /*
    struct person_stats *cur_person;
    for (i = 0; i < (ninjas + pirates); i++) {
        cur_person = persons[i];
        printf("%s: visited %d times, for ", (cur_person->race == Ninja) ? "Ninja" : "Pirate", cur_person->visits);
        struct visit_stats *cur_visit = cur_person->head;
        do {
            printf("%lld", cur_visit->visitTime);
            cur_visit = cur_visit->next;
        } while (cur_visit != NULL);
        printf("\n");
    }
     */
    return 0;
}
