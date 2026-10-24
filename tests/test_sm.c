#include "mlp.h"
#include <stdio.h>
int main(void) {
    state_machine_t *sm = sm_new();
    sm_transition(sm, 2); /* ACTIVE */
    if (!sm_is_active(sm)) { printf("active\n"); return 1; }
    sm_transition(sm, 6); /* CLOSED */
    if (!sm_is_closed(sm)) { printf("closed\n"); return 1; }
    sm_free(sm);

    media_lane_policy_t *p = policy_new(1);
    if (!p->media_lane_enabled) { printf("policy\n"); return 1; }
    policy_free(p);
    printf("OK\n");
    return 0;
}
