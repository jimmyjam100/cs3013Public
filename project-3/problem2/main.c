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
        if(driving_permits == 0 && (quad_intents[car->spawn_location - 1]->car->tid == car->tid)){
            remove_top(car->spawn_location - 1);
        }
            
        if(quads[car->spawn_location -1] == 1 && driving_permits != 0 && (quad_intents[car->spawn_location - 1]->car->tid == car->tid) ){
            printf("car %d: moved from lane to intersection quadrent %d\n", car->tid, car->spawn_location);
            remove_top(car->spawn_location - 1);
            driving_permits--;
            car->cur_location = car->spawn_location;
            quads[car->cur_location - 1] = 0;
            sem_post(&sem);
            return 1;
        }
        printf("car %d: could not move from lane into quadrent %d due to it being blocked or other cars waiting to go that spawned eairler\n", car->tid, car->spawn_location);
        if (quad_intents[car->spawn_location - 1] !=  NULL){
            printf("\tfirst in line is car %d\n", quad_intents[car->spawn_location - 1]->car->tid);
            printf("\tis lane full: %s\n", quads[car->spawn_location -1] ? "empty" : "full");
        }
        else{
            printf("\t3 cars are already on the intersection\n");
        }
        sem_post(&sem);
        return 0;
    }
    if(car->moves_left == 1){
        printf("car %d: leaving intersection from quadrent %d\n", car->tid, car->cur_location);
        driving_permits++;
        quads[car->cur_location - 1] = 1;
        car->cur_location = 0;
        car->moves_left--;
        sem_post(&sem);
        return 1;
    }
    if(!contains_car(car, quad_intents[(car->cur_location)%4])){
        add_car(car, (car->cur_location)%4);
    }
    if(quads[(car->cur_location)%4] == 1 && (quad_intents[(car->cur_location)%4]->car->tid == car->tid)){
        printf("car %d: moved from quadrent %d to quadrent %d\n", car->tid, car->cur_location, (car->cur_location)%4 + 1);
        remove_top((car->cur_location)%4);
        quads[car->cur_location - 1] = 1;
        car->cur_location = (car->cur_location)%4 +1;
        quads[car->cur_location - 1] = 0;
        car->moves_left--;
        sem_post(&sem);
        return 1;
    }
    printf("car %d: could not move from quadrent %d into quadrent %d due to it being blocked or other cars waiting to go that spawned eairler\n", car->tid, car->cur_location, (car->cur_location)%4 + 1);
    printf("\tfirst in line is car %d\n", quad_intents[(car->cur_location)%4]->car->tid);
    printf("\tis quad full: %s\n", quads[(car->cur_location)%4] ? "empty" : "full");
    printf("\tis %d and %d equal: %s\n", car->tid, quad_intents[(car->cur_location)%4]->car->tid, (quad_intents[(car->cur_location)%4]->car->tid == car->tid) ? "yes" : "no");
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
    int ret = (lrand48() % (max - min + 1)) + min;
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
        this_vehicle->cur_location = 0;
        this_vehicle->spawn_location = generateSpawnPoint();
        this_vehicle->moves_left = generateRandomInt(1, 3);
        printf("car %d: Spawned in lane %d and is making %s turn\n", tid, this_vehicle->spawn_location, this_vehicle->moves_left == 1 ? "a right" : this_vehicle->moves_left == 2 ? "no" : "a left");
        while(this_vehicle->moves_left > 0){
            while(move(this_vehicle) == 0){
                sleep(1);
            }
            sleep(1);
        }
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
    sleep(50);
    return 0;
}
