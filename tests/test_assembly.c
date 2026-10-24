#include "mlp.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
int main(void) {
    /* Build two fragments */
    update_meta_t m; memset(&m,0,sizeof(m));
    m.update_id=1; m.part_index=0; m.part_count=2; m.chain_epoch=1; m.send_epoch_ms=1;
    header_ext_t *ext = update_meta_to_ext(&m);
    fragment_t *f1 = fragment_new();
    hex_free(f1->ext); f1->ext = ext;
    f1->header.version=MEDIA_PROTOCOL_VERSION; f1->header.cls=CLASS_MEDIA; f1->header.priority=PRIO_MEDIA;
    f1->header.object_id=1; f1->header.fragment_index=0; f1->header.fragment_count=2; f1->header.payload_len=4;
    uint8_t p1[4]={1,2,3,4}; f1->payload=malloc(4); memcpy(f1->payload,p1,4); f1->payload_len=4;
    uint8_t buf1[256]; size_t n1=0; fragment_encode(f1,buf1,sizeof(buf1),&n1);

    /* second fragment */
    m.part_index=1;
    header_ext_t *ext2 = update_meta_to_ext(&m);
    fragment_t *f2 = fragment_new();
    hex_free(f2->ext); f2->ext = ext2;
    f2->header.version=MEDIA_PROTOCOL_VERSION; f2->header.cls=CLASS_MEDIA; f2->header.priority=PRIO_MEDIA;
    f2->header.object_id=1; f2->header.fragment_index=1; f2->header.fragment_count=2; f2->header.payload_len=4;
    uint8_t p2[4]={5,6,7,8}; f2->payload=malloc(4); memcpy(f2->payload,p2,4); f2->payload_len=4;
    uint8_t buf2[256]; size_t n2=0; fragment_encode(f2,buf2,sizeof(buf2),&n2);

    object_assembler_t *a = assembler_new();
    open_object_t *out = NULL;
    int r1 = assembler_accept(a, buf1, n1, &out);
    if (r1 != 0) { printf("first should be pending, got %d\n", r1); return 1; }
    if (assembler_open_count(a) != 1) { printf("open count\n"); return 1; }
    int r2 = assembler_accept(a, buf2, n2, &out);
    if (r2 != 1) { printf("second should be complete, got %d\n", r2); return 1; }
    if (out->received != 2) { printf("received\n"); return 1; }
    assembler_free(a);
    fragment_free(f1); fragment_free(f2);
    printf("OK\n");
    return 0;
}
