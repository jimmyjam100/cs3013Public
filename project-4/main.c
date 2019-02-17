#include <stdio.h>
#include <stdint.h>

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
        printf("%s\n", instruction);1
    }
    printf("Hello, World!\n");
    return 0;
}