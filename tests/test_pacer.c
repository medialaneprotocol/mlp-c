#include "mlp.h"
#include <stdio.h>
int main(void) {
    simple_pacer_t *p = pacer_new(8000, 16384);
    if (!pacer_try_acquire(p, 100, 0.0)) { printf("acquire\n"); return 1; }
    if (pacer_budget(p) != 16384) { printf("budget\n"); return 1; }
    pacer_free(p);
    printf("OK\n");
    return 0;
}
