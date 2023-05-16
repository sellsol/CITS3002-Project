#include <stdio.h>

int main(void) {
    int a = 1;
    int b = 1;

    printf("1\n1\n");
    int count = 2;
    while (count <= 10) {
        int c = a + b;
        a = b;
        b = c;
        printf("%i\n");
        count += 1;
    }
}