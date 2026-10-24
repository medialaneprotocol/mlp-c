#include "mlp.h"
#include <stdio.h>
int main(void) {
    session_ticket_t *t = ticket_issue(42, 7, 1000);
    uint64_t uid; uint32_t ep;
    if (!ticket_validate(t, 1000, &uid, &ep)) { printf("validate\n"); return 1; }
    if (uid != 42 || ep != 7) { printf("values\n"); return 1; }
    if (!ticket_valid(t, 1000)) { printf("valid\n"); return 1; }
    ticket_free(t);
    printf("OK\n");
    return 0;
}
