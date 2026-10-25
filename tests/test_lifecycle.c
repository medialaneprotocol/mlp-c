#include "mlp.h"
#include <stdio.h>
int main(void) {
    lane_lifecycle_t *l = lifecycle_new();
    if (l->state != LANE_STATE_STARTING) { printf("start\n"); return 1; }
    lifecycle_set_applied(l, 1000, 8);
    if (l->state != LANE_STATE_CONVERGING) { printf("converging %d\n", l->state); return 1; }
    lifecycle_set_applied(l, 1100, 0);
    if (l->state != LANE_STATE_LIVE) { printf("live %d\n", l->state); return 1; }
    lifecycle_free(l);

    resume_manager_t *r = resume_manager_new();
    resume_manager_park(r, 42, 7, 100, 0);
    if (!resume_manager_is_parked(r)) { printf("park\n"); return 1; }
    uint64_t id; uint32_t ep; uint64_t pc;
    if (resume_manager_resume(r, 60000, &id, &ep, &pc) != 1) { printf("resume\n"); return 1; }
    if (id != 42 || ep != 7 || pc != 100) { printf("values\n"); return 1; }
    resume_manager_park(r, 43, 8, 1, 0);
    if (resume_manager_resume(r, 301000, &id, &ep, &pc) != -1) { printf("ttl\n"); return 1; }
    resume_manager_free(r);

    /* The ack window: bit 0 names the highest applied update. */
    ack_window_t w;
    ack_window_reset(&w);
    ack_window_mark(&w, 1);
    ack_window_mark(&w, 2);
    ack_window_mark(&w, 3);
    if (!ack_window_has(&w, 1) || !ack_window_has(&w, 3)) { printf("window\n"); return 1; }
    if (ack_window_has(&w, 0)) { printf("id 0 names no update\n"); return 1; }
    ack_window_mark(&w, 3 + 64);
    if (ack_window_has(&w, 3)) { printf("a 64 jump leaves the new update alone\n"); return 1; }
    printf("OK\n");
    return 0;
}
