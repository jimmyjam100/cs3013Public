#include <stdio.h>

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