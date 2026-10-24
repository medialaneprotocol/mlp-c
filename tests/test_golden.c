#include "mlp.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static const unsigned char want[] = {
    0x4d,0x4c,0x50,0x00,0x01,0x02,0x00,0x06,0x2a,0x00,0x00,0x00,
    0x00,0x00,0x02,0x00,0x04,0x00,0x00,0x00,0x43,0x00,0x01,0x08,
    0x07,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x02,0x08,0x06,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x03,0x04,0x01,0x00,0x04,0x00,
    0x04,0x04,0xd2,0x04,0x00,0x00,0x05,0x04,0x03,0x00,0x00,0x00,
    0x06,0x08,0x63,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x10,0x04,
    0x00,0x00,0x00,0x00,0x13,0x01,0x00,0x07,0x08,0xe8,0x03,0x00,
    0x00,0x00,0x00,0x00,0x00,0x01,0x02,0x03,0x04,
};

int main(void) {
    update_meta_t m = {7,6,1,4,1234,3,99,0,0,1000};
    header_ext_t *ext = update_meta_to_ext(&m);
    fragment_t *f = fragment_new();
    f->header.version = MEDIA_PROTOCOL_VERSION;
    f->header.cls = CLASS_MEDIA;
    f->header.flags = 0;
    f->header.priority = PRIO_MEDIA;
    f->header.object_id = 42;
    f->header.fragment_index = 0;
    f->header.fragment_count = 2;
    f->header.payload_len = 4;
    hex_free(f->ext);
    f->ext = ext;
    uint8_t payload[4] = {1,2,3,4};
    f->payload = malloc(4); memcpy(f->payload, payload, 4); f->payload_len = 4;

    uint8_t buf[256];
    size_t outlen = 0;
    if (fragment_encode(f, buf, sizeof(buf), &outlen) != 0) { printf("encode fail\n"); return 1; }
    if (outlen != sizeof(want)) { printf("len %zu vs %zu\n", outlen, sizeof(want)); return 1; }
    if (memcmp(buf, want, sizeof(want)) != 0) { printf("bytes mismatch\n"); return 1; }

    fragment_t *f2 = fragment_new();
    size_t consumed = 0;
    if (fragment_parse(f2, buf, outlen, &consumed) != 0) { printf("parse fail\n"); return 1; }
    if (consumed != outlen) { printf("consumed %zu\n", consumed); return 1; }
    update_meta_t m2;
    if (update_meta_from_ext(f2->ext, &m2) != 0) { printf("meta from ext fail\n"); return 1; }
    if (m2.update_id != 7 || m2.update_base != 6 || m2.part_index != 1 || m2.part_count != 4 ||
        m2.update_bytes != 1234 || m2.chain_epoch != 3 || m2.reflects_input_seq != 99 || m2.send_epoch_ms != 1000) {
        printf("meta mismatch\n"); return 1;
    }
    printf("OK\n");
    fragment_free(f); fragment_free(f2);
    return 0;
}
