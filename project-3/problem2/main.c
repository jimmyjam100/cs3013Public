#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <semaphore.h>
#include <zconf.h>
#include <syscall.h>
#include <malloc.h>
#include <sys/time.h>
#include <stdlib.h>

#define num_threads 20

int* quads;
int driving_permits;
sem_t sem;

pthread_t tids[num_threads];

struct car {
    unsigned long long spawn_time;
    pid_t tid;
    int spawn_location; // a road number
    int moves_left; // number of quadrant movements left
    int cur_location; // 0 means not in intersection
};

struct intents {
    struct car *car;
    struct intents *next;
};

struct intents** quad_intents;

int contains_car(struct car *car, struct intents *intents){
    if(intents == NULL){
        return 0;
    }
    struct intents *cur = intents;
    while(cur != NULL){
        if(cur->car == car){
            return 1;
        }
        cur = cur->next;
    }
    return 0;
}

int add_car(struct car *car, int listIndex){
    struct intents *new = malloc(sizeof(struct intents));
    new->car = car;
    if(quad_intents[listIndex] == NULL){
        new->next = NULL;
        quad_intents[listIndex] = new;
        return 1;
    }
    if(quad_intents[listIndex]->car->spawn_time > car->spawn_time){
        new->next = quad_intents[listIndex];
        quad_intents[listIndex] = new;
        return 1;
    }
    struct intents *cur = quad_intents[listIndex];
    while(cur->next != NULL && cur->next->car->spawn_time < car->spawn_time){
        cur = cur->next;
    }
    new->next = cur->next;
    cur->next = new;
    return 1;
}

int remove_top(int listIndex){
    if(quad_intents[listIndex] == NULL){
        return 0;
    }
    struct intents *save = quad_intents[listIndex];
    quad_intents[listIndex] = quad_intents[listIndex]->next;
    free(save);
    return 1;
}
    


int move(struct car* car){
    sem_wait(&sem);
    if(car->cur_location == 0){
        if(!contains_car(car, quad_intents[car->spawn_location - 1])){
            add_car(car, car->spawn_location - 1);
        }
        if(quads[car->spawn_location -1] == 1 && quad_intents[car->spawn_location - 1]->car == car && driving_permits != 0){
            printf("car %d: moving from lane to intersection quadrent %d", car->tid, car->spawn_location);
            car->cur_location = car->spawn_location;
            car->moves_left = car->moves_left - 1;
            quads[cur_location - 1] = 0;
            sem_post(&sem);
            return 1;
        }
        printf("car %d: could not move into intersection due to it being blocked or other cars wanting to go first", car->tid);
        sem_post(&sem);
        return 0;
    }

    sem_post(&sem);
    return 0;
        
}

/*
 * Quadrant Map:
 *     North (2)
 *    --------------
 * W |      |       | E
 * E |   2  |   1   | A
 * S |      |       | S
 * T  --------------  T
 * ( |      |       | (
 * 3 |  3   |   4   | 1
 * ) |      |       | )
 *    --------------
 *     South (4)
 */

// returns milliseconds since the epoch
unsigned long long getCurJSEpoch() {
    struct timeval t;
    gettimeofday(&t, NULL);
    return ((unsigned long long)(t.tv_sec) * 1000 + (unsigned long long)(t.tv_usec) / 1000);
}

int generateRandomInt(int min, int max) {
    int ret = lrand48() % max + min;
    return ret;
}

int generateSpawnPoint() {
    return generateRandomInt(1, 4);
}

int generateDestination(int starting) {
    if (starting == 1) {
        return generateRandomInt(2, 4);
    } else if (starting == 2) {
        int x = generateRandomInt(3, 5) % 4;
        return x == 0 ? 4 : x;
    } else if (starting == 3) {
        int x = generateRandomInt(4, 6) % 4;
        return x == 0 ? 4 : x;
    } else if (starting == 4) {
        return generateRandomInt(1, 3);
    }
}


void* thread(void *args) {
    pid_t tid = syscall(SYS_gettid);
    struct car* this_vehicle = malloc(sizeof(struct car));
    this_vehicle->tid = tid;
    while (1) {
        this_vehicle->spawn_time = getCurJSEpoch();
        this_vehicle->cur_location = generateSpawnPoint();
        this_vehicle->moves_left =
        printf("%d: Spawned\n", tid);
    }
}


int main() {
    srand(time(NULL));
    int err, i;
    quad_intents = malloc(sizeof(struct intents*)*4);
    for (i = 0; i < 4; i++){
        quad_intents[i] = NULL;
    }
    quads = malloc(sizeof(int)*4);
    for (i = 0; i < 4; i++){
        quads[i] = 1;
    }
    driving_permits = 3;
    err = sem_init(&sem, 0, 1);
    if (err) { // error intializing semaphores
        return 1;
    }
    printf("Spawning threads\n");
    for (i = 0; i < num_threads; i++) {
        if (pthread_create(&tids[i], NULL, thread, (void *) 0)){

        }
    }
    return 0;
}
