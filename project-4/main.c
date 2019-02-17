#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#define NOTALLOC -1
#define SWAPPED -2

#define EMPTY 0
#define FULL 1

char memory[64];

struct table_entry{
    uint8_t alloc:1;
    uint8_t valid:1;
    uint8_t protection:1;
    uint8_t frame:2;
    uint8_t buff:3;
};

int page_table_start[4];

int free_list[4];

enum instructions {
    load,
    store,
    map
};

struct user_input {
    int valid;
    int pid;
    enum instructions instruction;
    int virtual_address;
    int value;
};

void mapInst(int pid, int virtual_address, int protection){
    if(page_table_start[pid] == NOTALLOC){
        int i = 0;
        for(i = 0; i < 4 && free_list[i] == FULL; i++){}
        if(i == 4){
            printf("error too full to create page table for pid %d\n", pid);
            return;
        }
        else{
            page_table_start[pid] = i;
            free_list[i] = FULL;
            printf("Put page table for PID %d into physical frame %d\n", pid, i);
        }
    }
    struct table_entry* entry = (struct table_entry*)(&memory[page_table_start[pid]*16 + (virtual_address>>4)]);
    
    if(entry->alloc == 0){
        printf("allocating new page\n");
        int i = 0;
        for(i = 0; i < 4 && free_list[i] == FULL; i++){}
        if(i == 4){
            printf("error too full to create page\n");
            return;
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
    else{
        printf("page already allocated, updating protection\n");
        entry->protection = protection;   
    }

}

void storeInst(int pid, int virtual_address, int value){
    if (page_table_start[pid] != NOTALLOC){
        struct table_entry* entry = (struct table_entry*)(&memory[page_table_start[pid]*16 + (virtual_address>>4)]);
        if(entry->alloc == 0){
            printf("error: space not allocated yet\n");
            return;
        }
        if(entry->protection == 0){
            printf("error: space is read only\n");
            return;
        }
        int newAddress = ((virtual_address)&(0xf)) + (entry->frame << 4);
        printf("Stored value %d at virtual address %d (physical address %d)\n", value, virtual_address, newAddress);
        memory[newAddress] = value;
        return;
    }
    printf("error: space not allocated yet\n");
}

void loadInst(int pid, int virtual_address) {
    if (page_table_start[pid] != NOTALLOC){
        struct table_entry* entry = (struct table_entry*)(&memory[page_table_start[pid]*16 + (virtual_address>>4)]);
        if(entry->alloc == 0){
            printf("error: space not allocated yet\n");
            return;
        }
        int newAddress = ((virtual_address)&(0xf)) + (entry->frame << 4);
        printf("The value %d is virtual address %d (physical address %d)\n", memory[newAddress], virtual_address, newAddress);
        return;
    }
    printf("error: space not allocated yet\n");

}

void process(int pid, enum instructions instruction, int virtual_address, int value) {
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
    struct user_input ret;
    ret.valid = 0;

    int init_size = strlen(input);
    char delim[] = ",";

    char *ptr = strtok(input, delim);
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

    if (ret.valid == 0) ret.valid = 1;

    return ret;
}

int main() {
    // Initializing memory valuess
    int i = 0;
    for (i = 0; i < 64; i++){
        memory[i] = 0;
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
