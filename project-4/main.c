#include <stdio.h>

int main() {
    while (1) {
        char *instruction;
        printf("Instruction? ");
        if (scanf("%s", instruction) == EOF) {
            printf("\nEnd of program\n");
        }
        // do stuff with instructions
    }
    printf("Hello, World!\n");
    return 0;
}