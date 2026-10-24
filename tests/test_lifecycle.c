#include "mlp.h"
#include <stdio.h>
int main(void) {
    lane_lifecycle_t *l = lifecycle_new();
    lifecycle_set_active(l);
    if (l->parked) { printf("parked\n"); return 1; }
    lifecycle_set_parked(l);
    if (!lifecycle_is_parked(l)) { printf("not parked\n"); return 1; }
    lifecycle_set_loss_ratio(l, 0.1);
    if (!l->loss_degraded) { printf("degraded\n"); return 1; }
    lifecycle_free(l);

    resume_manager_t *r = resume_manager_new();
    resume_manager_park(r, 42, 7, 100);
    if (!resume_manager_is_parked(r)) { printf("park\n"); return 1; }
    uint64_t id; uint32_t ep; uint64_t pc;
    if (!resume_manager_resume(r, &id, &ep, &pc)) { printf("resume\n"); return 1; }
    if (id != 42 || ep != 7 || pc != 100) { printf("values\n"); return 1; }
    resume_manager_free(r);
    printf("OK\n");
    return 0;
}
