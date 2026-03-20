#include <stdio.h>
int main() {
    fprintf(stderr, "START\n");
    fflush(stderr);
    printf("Main start\n");
    fflush(stdout);
    return 0;
}
