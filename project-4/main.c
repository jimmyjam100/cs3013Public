#include <stdio.h>
#include <stdint.h>

#define NOTALLOC -1
#define SWAPED -2

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

void mapInst(int pid, int virtual_address, int protection){
    if(page_table_start[pid] == NOTALLOC){
        int i = 0;
        for(i = 0; i < 4 && free_list[i] == EMPTY; i++){}
        if(i == 4){
            printf("error too full to create page table for pid %d\n", pid);
            return;
        }
        else{
            page_table_start[pid] = i;
            free_list[i] = FULL;
        }
    }
    struct table_entry* entry = (struct table_entry*)(&memory[page_table_start[pid]*16 + (virtual_address>>4)]);

}

void storeInst(int pid, int virtual_address, int value){

}

void loadInst(int pid, int virtual_address){

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


void preProcess(char *input) {

}


int main() {
    int i = 0;
    for (i = 0; i < 64; i++){
        memory[i] = 0;
    }
    for (i = 0; i < 4; i++){
        free_list[i] = EMPTY;
        page_table_start[i] = NOTALLOC;
    }
    while (1) {
        char instruction[100];
        printf("Instruction? ");
        if (scanf("%s", instruction) == EOF) {
            printf("\nEnd of program\n");
            return 0;
        }
        // do stuff with instructions
        printf("%s\n", instruction);
    }
    printf("Hello, World!\n");
    return 0;
}
