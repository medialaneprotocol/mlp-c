#include "mlp.h"
#include <stdio.h>
int main(void) {
    if (scheduler_max_parts(0, 0.0) != 8) { printf("clean stream\n"); return 1; }
    if (scheduler_max_parts(0, 0.1) != 2) { printf("lossy stream\n"); return 1; }
    if (scheduler_max_parts(1, 0.1) != 1) { printf("datagram\n"); return 1; }
    printf("OK\n");
    return 0;
}
