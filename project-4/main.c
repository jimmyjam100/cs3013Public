#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#define NOTALLOC 4
#define SWAPPED -2

#define EMPTY 0
#define FULL 1

#define N_PROCESSES 4

struct table_entry{
    uint8_t alloc:1;
    uint8_t valid:1;
    uint8_t protection:1;
    uint8_t frame:2;
    uint8_t swapspace:8;
    uint8_t buffer:3;
};

struct page {
    char data[16];
};

unsigned char memory[64];

enum SWAP_STATES {
    FREE,
    CATALONIA // Occupied
};
enum SWAP_STATES swap_states[N_PROCESSES + N_PROCESSES * 4];
struct page swap[N_PROCESSES + N_PROCESSES * 4];

int page_table_start[4];

int free_list[4];

int swapSize = 0;
int nextSwap = 0;

// the different types of instructions
enum instructions {
    load,
    store,
    map
};

// a struct used to store parsed user input
struct user_input {
    int valid;
    int pid;
    enum instructions instruction;
    int virtual_address;
    int value;
};

int get_num_pages_in_swap() {
    int counter = 0, i = 0;
    for (i = 0; i < (N_PROCESSES + N_PROCESSES * 4); i++) {
        if (swap_states[i] == CATALONIA) {
            counter += 1;
        }
        // above could be simplified to counter += swap_states[i] // but we're not a mad lad
    }
    return counter;
}

struct page get_page_from_swap(int index) {
    return swap[index];
}

int append_page_to_swap(struct page page) {
    int counter = 0, i = 0;
    for (i = 0; i < (N_PROCESSES + N_PROCESSES * 4); i++) {
        if (swap_states[i] == FREE) {
            swap[i] = page;
            swap_states[i] = CATALONIA;
            return i;
        }
    }
}

void write_to_swap_location(struct page page, int index) {
    swap_states[index] = CATALONIA;
    swap[index] = page;
}

struct page copyPage(struct page *page){
    struct page ret;
    for(int i = 0; i < 16; i++){
        ret.data[i] = page->data[i];
    }
    return ret;
}

int indexToSwap(int index){
    return -(index+1);
}

int swapToIndex(int swap){
    return -(swap+1);
}

void swapPage(int swapSpace, int pageIndex){
    printf("Swapped frame %d and disk swapslot %d\n", pageIndex, swapSpace);
    struct page swapIn = get_page_from_swap(swapSpace);
    struct page swapOut = copyPage((struct page*) (&memory[pageIndex*16]));
    //strncpy((char *)(&swapOut), &memory[pageIndex*16], sizeof(struct page));
    for (int i = 0; i < 4; i++){
        if(page_table_start[i] >= 0 && page_table_start[i] <= 3){
            if (pageIndex == page_table_start[i]){
                //printf("SWAPPED SOMETHING IN FOR A PAGE TABLE\n");
                for (int k = 0; k < 4; k++){
                    if(indexToSwap(page_table_start[k]) == swapSpace){
                        page_table_start[k] = pageIndex;
                    }
                }
                page_table_start[i] = swapToIndex(swapSpace);
                write_to_swap_location(swapOut, swapSpace);
                *((struct page *)(&memory[pageIndex*16])) = copyPage(&swapIn);
                //strncpy(&memory[pageIndex*16], (char *)(&swapIn), sizeof(struct page));
                nextSwap = (nextSwap+1)%4;
                return;    
            }
            else{
                for(int j = 0; j < 4; j++){
                    struct table_entry* entry = (struct table_entry*)(&memory[page_table_start[i]*16 + j*2]);
                    if (entry->valid == 1 && entry->alloc == 1 && entry->frame == pageIndex){
                        //printf("SWAPED AN ENTRY OUT WHILE PAGE TABLE %d IS IN MEMORY IN INDEX %d\n", i, page_table_start[i]);
                        entry->valid = 0;
                        entry->swapspace = swapSpace;
                        write_to_swap_location(swapOut, swapSpace);
                        *((struct page *)(&memory[pageIndex*16])) = copyPage(&swapIn);
                        //strncpy(&memory[pageIndex*16], (char *)(&swapIn), sizeof(struct page));
                        for (int k = 0; k < 4; k++){
                            if(indexToSwap(page_table_start[k]) == swapSpace){
                                page_table_start[k] = pageIndex;
                            }
                        }
                        nextSwap = (nextSwap+1)%4;
                        return;
                    }
                }
            }
        }
        else{
            if(indexToSwap(page_table_start[i]) == swapSpace){
                for(int j = 0; j < 4; j++){
                    struct table_entry* entry = (struct table_entry*)(&(swapIn.data[j*2]));
                    if(entry->valid == 1 && entry->alloc == 1 && entry->frame == pageIndex){
                        //printf("SWAPED AN ENTRY WITH ITS PAGE TABLE\n");
                        write_to_swap_location(swapOut, swapSpace);
                        *((struct page *)(&memory[pageIndex*16])) = copyPage(&swapIn);
                        //strncpy(&memory[pageIndex*16], (char *)(&swapIn), sizeof(struct page));
                        entry = (struct table_entry*)(&memory[pageIndex*16 + j*2]);
                        entry->valid  = 0;
                        entry->swapspace = swapSpace;
                        page_table_start[i] = pageIndex;
                        nextSwap = (nextSwap+1)%4;
                        return;
                    }
                }
            
            
            }
            struct page possibleTable =  get_page_from_swap(indexToSwap(page_table_start[i]));
            for(int j = 0; j < 4; j++){
                struct table_entry* entry = (struct table_entry*)(&(possibleTable.data[j*2]));
                if(entry->valid == 1 && entry->alloc == 1 && entry->frame == pageIndex){

                    //printf("SWAPED A ENTRY WITH NOT ITS PAGE TABLE I THINK ITS PAGE TABLE IS IN SWAPSPACE %d\n", indexToSwap(page_table_start[i]));
                    write_to_swap_location(swapOut, swapSpace);
                    *((struct page *)(&memory[pageIndex*16])) = copyPage(&swapIn);
                    //strncpy(&memory[pageIndex*16], (char *)(&swapIn), sizeof(struct page));
                    for (int k = 0; k < 4; k++){
                        if(indexToSwap(page_table_start[k]) == swapSpace){
                            page_table_start[k] = pageIndex;
                        }
                    }
                    
                    swapPage(indexToSwap(page_table_start[i]), (pageIndex + 1)%4);
                    entry = (struct table_entry*)(&(memory[page_table_start[i]*16 + j*2]));
                    entry->valid = 0;
                    entry->swapspace = swapSpace;
                    nextSwap = (nextSwap+1)%4;
                    return;                   
                }
            }
        }
    }
    printf("ERROR: DID NOT SWAP FOR SOME REASON\n");                   
}

void mapInst(int pid, int virtual_address, int protection){
    if(!(page_table_start[pid] >= 0 && page_table_start[pid] <=3)){
        int i = 0;
        for(i = 0; i < 4 && free_list[i] == FULL; i++){}
        if(i == 4){
            int newPage = 0;
            if(page_table_start[pid] == NOTALLOC){
                newPage = 1;
                struct page newPage;
                for(int j = 0; j < 16; j++){
                    newPage.data[j] = 0;
                }
                append_page_to_swap(newPage);
                page_table_start[pid] = swapToIndex(get_num_pages_in_swap() - 1);
                //printf("num of pages %d\n", get_num_pages_in_swap());
            }
            int oldSwap = nextSwap;
            swapPage(indexToSwap(page_table_start[pid]), nextSwap);
            //page_table_start[pid] = oldSwap;
            if(newPage){
                 printf("Put page table for PID %d into physical frame %d\n", pid, oldSwap);
            }
        }
        else{
            page_table_start[pid] = i;
            free_list[i] = FULL;
            printf("Put page table for PID %d into physical frame %d\n", pid, i);
        }
    }
    struct table_entry* entry = (struct table_entry*)(&memory[page_table_start[pid]*16 + (virtual_address>>4)*2]);
    if(entry->alloc == 0){
        printf("allocating new page\n");
    }
    int oldAlloc = entry->alloc;
    int oldProt = entry->protection;
    if(entry->alloc == 0 || entry->valid == 0){
        int i = 0;
        for(i = 0; i < 4 && free_list[i] == FULL; i++){}
        if(i == 4){
            if(nextSwap == page_table_start[pid]){
                nextSwap = (nextSwap + 1)%4;
            }
            //printf("error too full to create page\n");
            int oldAlloc = entry->alloc;
            int oldSwap = nextSwap;
            if (oldAlloc == 0){
                struct page newPage;
                for(int j = 0; j < 16; j++){
                    newPage.data[j] = 0;
                }
                append_page_to_swap(newPage);
                entry->swapspace = get_num_pages_in_swap() - 1;
            }
            swapPage(entry->swapspace, nextSwap);
            entry->alloc = 1;
            entry->valid = 1;
            entry->protection = protection;
            entry->frame = oldSwap;
            //return;
        }
        else{
            printf("Mapped virtual_address %d (page %d) into physical frame %d\n", virtual_address, (virtual_address>>4), i);
            free_list[i] = FULL;
            entry->alloc = 1;
            entry->valid = 1;
            entry->protection = protection;
            entry->frame = i;
        }
    }
    if(oldAlloc == 1){
        if(oldProt == protection){
            printf("Error: virtual page %d is already mapped with rw_bit=%d\n", virtual_address>>4, protection);
        }
        else{
            printf("page already allocated, updating protection\n");
            entry->protection = protection;
        }   
    }

}

void storeInst(int pid, int virtual_address, int value){
    if (page_table_start[pid] != NOTALLOC){
        if(page_table_start[pid] < 0 || page_table_start[pid] > 3){
            int oldSwap = nextSwap;
            swapPage(indexToSwap(page_table_start[pid]), nextSwap);
            //page_table_start[pid] = oldSwap;
        }
        struct table_entry* entry = (struct table_entry*)(&memory[page_table_start[pid]*16 + (virtual_address>>4)*2]);
        if(entry->alloc == 0){
            printf("Error: space not allocated yet\n");
            return;
        }
        if(entry->protection == 0){
            printf("Error: space is read only\n");
            return;
        }
        if(entry->valid == 0){
            if(nextSwap == page_table_start[pid]){
                nextSwap = (nextSwap + 1)%4;
            }
            int oldSwap = nextSwap;
            swapPage(entry->swapspace, nextSwap);
            entry->valid = 1;
            entry->frame = nextSwap;
        }
        int newAddress = ((virtual_address)&(0xf)) + (entry->frame << 4);
        printf("Stored value %d at virtual address %d (physical address %d)\n", value, virtual_address, newAddress);
        memory[newAddress] = value;
        return;
    }
    printf("Error: space not allocated yet\n");
}

void loadInst(int pid, int virtual_address) {
    if (page_table_start[pid] != NOTALLOC){
        if(page_table_start[pid] < 0 || page_table_start[pid] > 3){
            int oldSwap = nextSwap;
            swapPage(indexToSwap(page_table_start[pid]), nextSwap);
            //page_table_start[pid] = oldSwap;
        }
        struct table_entry* entry = (struct table_entry*)(&memory[page_table_start[pid]*16 + (virtual_address>>4)*2]);
        if(entry->alloc == 0){
            printf("Error: space not allocated yet\n");
            return;
        }
        if(entry->valid == 0){
            if(nextSwap == page_table_start[pid]){
                nextSwap = (nextSwap + 1)%4;
            }
            int oldSwap = nextSwap;
            swapPage(entry->swapspace, nextSwap);
            entry->valid = 1;
            entry->frame = oldSwap;
        }
        int newAddress = ((virtual_address)&(0xf)) + (entry->frame << 4);
        printf("The value %d is virtual address %d (physical address %d)\n", memory[newAddress], virtual_address, newAddress);
        return;
    }
    printf("Error: space not allocated yet\n");

}

void process(int pid, enum instructions instruction, int virtual_address, int value) {
    // basically a switch statement determining which function to call based off of the instruction enum that was parsed from the input
    if(instruction == map){
        mapInst(pid, virtual_address, value);
    }
    else if(instruction == store){
        storeInst(pid, virtual_address, value);
    }
    else if(instruction == load){
        loadInst(pid, virtual_address);
    }
}


struct user_input preProcess(char *input) {
    // Create structure that will be returned to invoker
    struct user_input ret;
    // Set the valid byte to zero. The invoker will check for a value of 1 to see if
    // the input was able to be correctly parsed, else there was an error.
    ret.valid = 0;

    int init_size = strlen(input);
    char delim[] = ",";

    char *ptr = strtok(input, delim);
    // set the value of unprocessed_pid to the pointer obtained from strstok
    // it is called unprocessed_pid because it is just a char*, and has not
    // yet been parsed into its proper data type
    char *unprocessed_pid = ptr;

    ptr = strtok(NULL, delim);
    if (ptr == NULL) return ret;
    char *unprocessed_instruction= ptr;

    ptr = strtok(NULL, delim);
    if (ptr == NULL) return ret;
    char *unprocessed_virtual_address = ptr;

    ptr = strtok(NULL, delim);
    if (ptr == NULL) return ret;
    char *unprocessed_value = ptr;

    // fill up the struct

    // get pid value
    ret.pid = atoi(unprocessed_pid);

    // get instruction
    if (strcmp(unprocessed_instruction, "store") == 0) {
        ret.instruction = store;
    } else if (strcmp(unprocessed_instruction, "load") == 0) {
        ret.instruction = load;
    } else if (strcmp(unprocessed_instruction, "map") == 0) {
        ret.instruction = map;
    } else {
        ret.valid = -1;
    }

    // get the virtual address
    ret.virtual_address = atoi(unprocessed_virtual_address);

    // get the value
    ret.value = atoi(unprocessed_value);

    // if no further issues were raised, then set the valid byte to 1
    if (ret.valid == 0) ret.valid = 1;

    // return the struct
    return ret;
}

int main() {
    // Initializing memory valuess
    int i = 0, j = 0;
    for (i = 0; i < 64; i++){
        memory[i] = 0;
    }
    for (i = 0; i < (N_PROCESSES + N_PROCESSES * 4); i++) {
        swap_states[i] = FREE;
        struct page new;
        for (j = 0; j < 16; j++) {
            new.data[j] = 0;
        }
        swap[i] = new;
    }
    // Initializing free list
    for (i = 0; i < 4; i++){
        free_list[i] = EMPTY;
        page_table_start[i] = NOTALLOC;
    }
    // Loop through to get user input
    while (1) {
        char instruction[100];
        printf("Instruction? ");
        // If end of file detceted, stop the program
        // Store input into the instruction array
        if (scanf("%s", instruction) == EOF) {
            printf("\nEnd of program\n");
            return 0;
        }
        // do stuff with instructions
        // print out the input received
        printf("Input received: %s\n", instruction);
        // parse the input into a struct with the correct data types
        struct user_input ui = preProcess(instruction);
        // ensure the input was valid / no errors while parsing
        if (ui.valid != 1) {
            printf("Invalid user input\n");
        }
        // call the process function with our input
        process(ui.pid, ui.instruction, ui.virtual_address, ui.value);
    }
    return 0;
}
