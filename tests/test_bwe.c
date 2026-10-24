#include "mlp.h"
#include <stdio.h>
int main(void) {
    bwe_estimate_t e = bwe_estimate(100000, 100.0);
    if (e.pace_bps != 100000) { printf("pace\n"); return 1; }
    if (e.rtt_ms != 100.0) { printf("rtt\n"); return 1; }
    printf("OK\n");
    return 0;
}
