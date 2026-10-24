#include "mlp.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main(void) {
    uint8_t payload[] = {1,2,3,4,5};
    uint8_t buf[256];
    size_t outlen = 0;
    int n = sender_enqueue_update(payload, 5, 7, buf, sizeof(buf), &outlen);
    if (n != 1) { printf("count %d\n", n); return 1; }
    fragment_t *f = fragment_new();
    size_t consumed = 0;
    if (fragment_parse(f, buf, outlen, &consumed) != 0) { printf("parse fail\n"); return 1; }
    if (consumed != outlen) { printf("consumed %zu\n", consumed); return 1; }
    update_meta_t m;
    if (update_meta_from_ext(f->ext, &m) != 0) { printf("meta fail\n"); return 1; }
    if (m.update_id != 7 || m.update_bytes != 5) { printf("meta mismatch\n"); return 1; }
    printf("OK\n");
    fragment_free(f);
    return 0;
}
