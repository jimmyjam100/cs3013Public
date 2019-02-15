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

int* quads; //ints values to represent if a quad is full or not
int driving_permits; //ints to represent how many people are on the road (3 means no one, 2 means one person ect.)
sem_t sem; //the semaphore that cars can take if they want to move

pthread_t tids[num_threads];

struct car { // struct for cars info
    unsigned long long spawn_time;
    pid_t tid;
    int spawn_location; // a road number
    int moves_left; // number of quadrant movements left
    int cur_location; // 0 means not in intersection
};

struct intents { //struct for linked list of cars wanting to move
    struct car *car;
    struct intents *next;
};

struct intents** quad_intents; //create a linked list for each quad


//returns true if the linked list contains the given car returns false otherwise
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

//adds a car to the linked list of the given quadId(listIndex)
//its position in the list is deturmined by how early it spawned (which is what we use for priority)
//(this also means that any car that arrives at an intersection can not go before another car that has arrived earlier)
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
//removes a car from a line (used if it just went to that quad)
int remove_car(struct car *car, int listIndex){
    if(quad_intents[listIndex] == NULL){
        return 0;
    }
    struct intents *cur = quad_intents[listIndex];
    if (cur->car == car){
        quad_intents[listIndex] = cur->next;
        free(cur);
        return 1;
    }
    while(cur->next != NULL){
        if(cur->next->car == car){
            struct intents *save = cur->next;
            cur->next = cur->next->next;
            free(save);
            return 1;
        }
        cur = cur->next;
    }
    return 0;
}
    

//see if a car can move if it can do the move
int move(struct car* car){
    sem_wait(&sem); //make it so no other car can move
    if(car->cur_location == 0){ //if the car is currently in lane
        if(!contains_car(car, quad_intents[car->spawn_location - 1])){ //add it to the line if it is not there already
            add_car(car, car->spawn_location - 1);
        }   
        if(quads[car->spawn_location -1] == 1 && driving_permits != 0 && (quad_intents[car->spawn_location - 1]->car->tid == car->tid) ){ //if there are less than 3 people driving and the quad that you want to drive into is not taken drive into it
            printf("car %d: moved from lane to intersection quadrent %d\n", car->tid, car->spawn_location);
            remove_car(car, car->spawn_location - 1);
            driving_permits--;
            car->cur_location = car->spawn_location;
            quads[car->cur_location - 1] = 0;
            sem_post(&sem);
            return 1;
        }
        //otherwise print out that you can not as well as some stats
        printf("car %d: could not move from lane into quadrent %d due to it being blocked or other cars waiting to go that spawned eairler\n", car->tid, car->spawn_location);
        if (driving_permits != 0){
            printf("\tfirst in line is car %d\n", quad_intents[car->spawn_location - 1]->car->tid);
            printf("\tis quad full: %s\n", quads[car->spawn_location -1] ? "empty" : "full");
        }
        else{
            printf("\t3 cars are already on the intersection\n");
        }
        sem_post(&sem);
        return 0;
    }
    if(car->moves_left == 1){ //if it is moving from a quad into a lane move automaticlly
        printf("car %d: leaving intersection from quadrent %d\n", car->tid, car->cur_location);
        driving_permits++;
        quads[car->cur_location - 1] = 1;
        car->cur_location = 0;
        car->moves_left--;
        sem_post(&sem);
        return 1;
    }
    //if you are moving from one quad to another
    if(!contains_car(car, quad_intents[(car->cur_location)%4])){ //if it is not already in line put it in line
        add_car(car, (car->cur_location)%4);
    }
    if(quads[(car->cur_location)%4] == 1 && (quad_intents[(car->cur_location)%4]->car->tid == car->tid || driving_permits == 0)){ //if the quad is free and you are the first in line (or there are 3 people on the road already indicating that this is the only elligable move possible for any of the cars) move the car to the quad he wants to go to
        printf("car %d: moved from quadrent %d to quadrent %d\n", car->tid, car->cur_location, (car->cur_location)%4 + 1);
        remove_car(car, (car->cur_location)%4);
        quads[car->cur_location - 1] = 1;
        car->cur_location = (car->cur_location)%4 +1;
        quads[car->cur_location - 1] = 0;
        car->moves_left--;
        sem_post(&sem);
        return 1;
    }
    //if he cannot print it out along with some stats
    printf("car %d: could not move from quadrent %d into quadrent %d due to it being blocked or other cars waiting to go that spawned eairler\n", car->tid, car->cur_location, (car->cur_location)%4 + 1);
    printf("\tfirst in line is car %d\n", quad_intents[(car->cur_location)%4]->car->tid);
    printf("\tis quad full: %s\n", quads[(car->cur_location)%4] ? "empty" : "full");
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
//generates as random int from min to max
int generateRandomInt(int min, int max) {
    int ret = (lrand48() % (max - min + 1)) + min;
    return ret;
}
//gerates a random spawn point
int generateSpawnPoint() {
    return generateRandomInt(1, 4);
}

//the thread for each car
void* thread(void *args) {
    pid_t tid = syscall(SYS_gettid);
    struct car* this_vehicle = malloc(sizeof(struct car)); // create the car struct and start filling out its info
    this_vehicle->tid = tid;
    while (1) { // each time a car gets to the begginging of this loop it signifys it being a new car
        this_vehicle->spawn_time = getCurJSEpoch(); //get the spawn time so we know how much priority it should get
        this_vehicle->cur_location = 0; //set its current locaation to the lane
        this_vehicle->spawn_location = generateSpawnPoint(); //set the quad that it will move into first
        this_vehicle->moves_left = generateRandomInt(1, 3); //set if it will make a right turn (1), no turn (2), or a left turn (3)
        printf("car %d: Spawned in lane %d and is making %s turn\n", tid, this_vehicle->spawn_location, this_vehicle->moves_left == 1 ? "a right" : this_vehicle->moves_left == 2 ? "no" : "a left"); //print out that a car has spawned
        while(this_vehicle->moves_left > 0){ //while this vehicle as moves left to make
            move(this_vehicle); //make a move if it can and then sleep
            sleep(1);
        }
    }
}


int main() {
    printf("here is some ascci art for refrence when we talk about lane and quad 1, 2, 3, and 4\n");
    printf("    North (2)\n");
    printf("   --------------\n");
    printf("W |      |       | E\n");
    printf("E |   2  |   1   | A\n");
    printf("S |      |       | S\n");
    printf("T  --------------  T\n");
    printf("( |      |       | (\n");
    printf("3 |  3   |   4   | 1\n");
    printf(") |      |       | )\n");
    printf("   --------------\n");
    printf("    South (4)\n\n\n");
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

    for (i = 0; i < num_threads; i++) {
        if (tids[i] != 0) {
            pthread_join(tids[i], NULL);
        }
    }
    return 0;
}
