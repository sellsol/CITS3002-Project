#include <stdio.h>
#include <string.h>

int main(int argc, char **argv) {
    int count = 0;
    for (int i = 1; i < argc; i++) {
        count += strlen(argv[i]);
    }
    printf("%i", count);
}