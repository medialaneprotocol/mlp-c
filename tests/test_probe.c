#include "mlp.h"
#include <stdio.h>
int main(void) {
    path_prober_t *p = prober_new(1000);
    probe_result_t r0 = {0, 100.0, 0.05, 1000000};
    probe_result_t r1 = {1, 50.0, 0.01, 2000000};
    prober_record(p, r0);
    prober_record(p, r1);
    if (prober_best_path(p) != 1) { printf("best\n"); return 1; }
    prober_free(p);
    printf("OK\n");
    return 0;
}
