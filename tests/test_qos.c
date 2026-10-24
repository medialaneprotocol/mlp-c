#include "mlp.h"
#include <stdio.h>
int main(void) {
    if (qos_mlp_priority(3) >= qos_mlp_priority(2)) { printf("prio\n"); return 1; }
    if (qos_weight(3) <= qos_weight(2)) { printf("weight\n"); return 1; }
    printf("OK\n");
    return 0;
}
