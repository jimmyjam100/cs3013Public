#include <stdio.h>
#include <stdint.h>


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

void process(int pid, enum instructions instruction, int virtual_address, int value) {

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
        printf("%s\n", instruction);
    }
    printf("Hello, World!\n");
    return 0;
}
