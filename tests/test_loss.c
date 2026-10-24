#include "mlp.h"
#include <stdio.h>
int main(void) {
    loss_detector_t *ld = loss_detector_new(3);
    uint64_t acked[] = {0,1,2,4,5,6,7,8,9};
    loss_detector_on_ack(ld, acked, 9);
    if (!loss_detector_is_lost(ld, 3)) { printf("lost\n"); return 1; }
    if (loss_detector_lost_count(ld) != 1) { printf("count %zu\n", loss_detector_lost_count(ld)); return 1; }
    loss_detector_free(ld);
    printf("OK\n");
    return 0;
}
