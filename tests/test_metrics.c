#include "mlp.h"
#include <stdio.h>
#include <string.h>
int main(void) {
    mlp_metrics_t *m = metrics_new();
    metrics_fragment_sent(m, 100);
    metrics_update_applied(m);
    char buf[512];
    size_t n = metrics_render(m, buf, sizeof(buf));
    if (n == 0) { printf("render\n"); return 1; }
    if (strstr(buf, "mlp_fragments_sent_total 1") == NULL) { printf("fragments\n"); return 1; }
    if (strstr(buf, "mlp_bytes_sent_total 100") == NULL) { printf("bytes\n"); return 1; }
    metrics_free(m);
    printf("OK\n");
    return 0;
}
