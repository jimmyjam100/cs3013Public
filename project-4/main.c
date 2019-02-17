#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>


char memory[64];

struct table_entry{
    uint8_t alloc:1;
    uint8_t valid:1;
    uint8_t protection:1;
    uint8_t frame:2;
    uint8_t buff:3;
};

int page_table_start[4];

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

void process(int pid, enum instructions instruction, int virtual_address, int value) {
    return;
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
    while (1) {
        char instruction[100];
        printf("Instruction? ");
        if (scanf("%s", instruction) == EOF) {
            printf("\nEnd of program\n");
            return 0;
        }
        // do stuff with instructions
        printf("Input received: %s\n", instruction);
        struct user_input ui = preProcess(instruction);
        if (ui.valid != 1) {
            printf("Invalid user input\n");
        }
        process(ui.pid, ui.instruction, ui.virtual_address, ui.value);
    }
    return 0;
}
