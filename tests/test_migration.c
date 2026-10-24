#include "mlp.h"
#include <stdio.h>
int main(void) {
    path_migration_t *m = migration_new(0);
    migration_start_probe(m, 1, 12345);
    if (!migration_is_probing(m)) { printf("probing\n"); return 1; }
    if (!migration_complete(m, 12345)) { printf("complete\n"); return 1; }
    if (m->active_path != 1) { printf("path\n"); return 1; }
    migration_free(m);
    printf("OK\n");
    return 0;
}
