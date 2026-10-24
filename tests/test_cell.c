#include "mlp.h"
#include <stdio.h>
int main(void) {
    cell_accounting_t *a = cell_accounting_new();
    uint32_t c[] = {0,1,2};
    cell_accounting_mark_pending(a, c, 3);
    if (cell_accounting_pending_count(a) != 3) { printf("pending1\n"); return 1; }
    uint32_t c2[] = {0,1};
    cell_accounting_mark_applied(a, c2, 2);
    if (cell_accounting_pending_count(a) != 1) { printf("pending2\n"); return 1; }
    if (cell_accounting_converged(a)) { printf("not converged\n"); return 1; }
    uint32_t c3[] = {2};
    cell_accounting_mark_applied(a, c3, 1);
    if (!cell_accounting_converged(a)) { printf("converged\n"); return 1; }
    cell_accounting_free(a);
    printf("OK\n");
    return 0;
}
