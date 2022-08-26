#include "stdio.h"
#include "stdlib.h"
extern int main() {
    FILE *f = fopen("tty","rw");
    if (!f) {
        printf("Failed to open TTY!\n");
        return 1;
    }
    printf("Enter something: ");
    char *buff = malloc(100);
    fread(buff,1,100,f);
    printf("You entered: ");
    printf(buff);
    printf("\n");
    fclose(f);
    return 0;
}