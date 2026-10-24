#include "mlp.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Embedded golden vector: media-update (must match conformance/vectors.json) */
static const char *hexstr = "4d4c5000010200062a0000000000020004000000430001080700000000000000020806000000000000000304010004000404d2040000050403000000060863000000000000001004000000001301000708e80300000000000001020304";

static int hexval(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

int main(void) {
    size_t len = strlen(hexstr) / 2;
    uint8_t *buf = malloc(len);
    for (size_t i = 0; i < len; i++) {
        buf[i] = (hexval(hexstr[2*i]) << 4) | hexval(hexstr[2*i+1]);
    }
    fragment_t *f = fragment_new();
    size_t consumed = 0;
    if (fragment_parse(f, buf, len, &consumed) != 0) { printf("parse fail\n"); return 1; }
    if (consumed != len) { printf("consumed %zu != %zu\n", consumed, len); return 1; }
    /* re-encode */
    uint8_t out[256]; size_t outlen = 0;
    if (fragment_encode(f, out, sizeof(out), &outlen) != 0) { printf("encode fail\n"); return 1; }
    if (outlen != len) { printf("outlen %zu != %zu\n", outlen, len); return 1; }
    if (memcmp(out, buf, len) != 0) { printf("mismatch\n"); return 1; }
    fragment_free(f);
    free(buf);
    printf("OK\n");
    return 0;
}
