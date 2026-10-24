#include "mlp.h"
#include <stdio.h>
int main(void) {
    input_queue_t *q = input_queue_new();
    input_queue_submit(q, "KeyA", 1, 1);
    input_queue_submit(q, "KeyA", 0, 2);
    if (input_queue_len(q) != 2) { printf("len\n"); return 1; }
    input_queue_free(q);
    printf("OK\n");
    return 0;
}
