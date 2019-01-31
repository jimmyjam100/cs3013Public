#include <stdio.h>
#include <memory.h>

int contains_virus(char *buf){
    int i;
    int contains;
    int end;
    char *check = "VIRUS";
    end = 0;
    while(end < 4){
        if(buf[end] == '\0') return 0;
        ++end;
    }
    while(buf[end] != '\0'){
        contains = 1;
        for(i = 0; i < 5; i++){
            if(buf[end-i] != check[4-i]) contains = 0;
        }
        if(contains == 1) return 1;
        ++end;
    }
    return 0;
}

int main() {
    printf("Hello, World!\n");
    char *a = strstr("wadawdVIR ada", "VIRUS");
    printf("%s", a);
    return 0;
}