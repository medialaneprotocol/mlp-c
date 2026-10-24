#include "mlp.h"
#include <stdio.h>
int main(void) {
    input_manager_t *m = input_manager_new();
    input_manager_submit(m, "KeyA", 1, 1000);
    if (!input_manager_is_pressed(m, "KeyA")) { printf("pressed\n"); return 1; }
    if (input_manager_pressed_count(m) != 1) { printf("count\n"); return 1; }
    input_manager_submit(m, "KeyA", 0, 1000);
    if (input_manager_is_pressed(m, "KeyA")) { printf("released\n"); return 1; }
    uint64_t out[4];
    size_t n = input_manager_expired(m, 2000, out);
    if (n < 1) { printf("expired %zu\n", n); return 1; }
    input_manager_free(m);
    printf("OK\n");
    return 0;
}
