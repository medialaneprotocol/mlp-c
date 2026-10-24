#include "mlp.h"
#include <stdio.h>
int main(void) {
    if (scheduler_max_chunks(0, 0.1) != 8) { printf("chunks\n"); return 1; }
    if (scheduler_max_chunks(1, 0.1) != 1) { printf("datagram\n"); return 1; }
    printf("OK\n");
    return 0;
}
