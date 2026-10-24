#ifndef MLP_H
#define MLP_H
#include <stdint.h>
#include <stddef.h>

#define FRAGMENT_MAGIC_0 'M'
#define FRAGMENT_MAGIC_1 'L'
#define FRAGMENT_MAGIC_2 'P'
#define FRAGMENT_MAGIC_3 '\0'
#define MEDIA_PROTOCOL_VERSION 1
#define FRAGMENT_FIXED_HEADER 22
#define MAX_HEADER_EXT 256

#define CLASS_MEDIA 2

#define PRIO_INPUT 1
#define PRIO_MEDIA 6

/* TLV kinds */
#define TLV_UPDATE_ID 0x01
#define TLV_UPDATE_BASE 0x02
#define TLV_PART 0x03
#define TLV_UPDATE_BYTES 0x04
#define TLV_CHAIN_EPOCH 0x05
#define TLV_REFLECTS_INPUT_SEQ 0x06
#define TLV_SEND_EPOCH_MS 0x07
#define TLV_GREASE 0x7F

typedef struct { uint8_t kind; uint8_t *value; size_t len; } tlv_entry_t;
typedef struct { tlv_entry_t *entries; size_t count; size_t cap; } header_ext_t;

header_ext_t *hex_new(void);
void hex_free(header_ext_t *h);
int hex_push_u64(header_ext_t *h, uint8_t kind, uint64_t v);
int hex_push_u32(header_ext_t *h, uint8_t kind, uint32_t v);
int hex_push_u16pair(header_ext_t *h, uint8_t kind, uint16_t a, uint16_t b);
const uint8_t *hex_get(header_ext_t *h, uint8_t kind, size_t *outlen);
size_t hex_encoded_len(header_ext_t *h);
int hex_encode(header_ext_t *h, uint8_t *out, size_t outcap, size_t *outlen);
int hex_parse(header_ext_t *h, const uint8_t *b, size_t n);

typedef struct {
    uint64_t update_id;
    uint64_t update_base;
    uint16_t part_index;
    uint16_t part_count;
    uint32_t update_bytes;
    uint32_t chain_epoch;
    uint64_t reflects_input_seq;
    uint64_t send_epoch_ms;
} update_meta_t;

header_ext_t *update_meta_to_ext(const update_meta_t *m);
int update_meta_from_ext(const header_ext_t *ext, update_meta_t *out);

typedef struct {
    uint8_t version;
    uint8_t cls;
    uint8_t flags;
    uint8_t priority;
    uint32_t object_id;
    uint16_t fragment_index;
    uint16_t fragment_count;
    uint32_t payload_len;
    uint16_t header_ext_len;
} fragment_header_t;

typedef struct {
    fragment_header_t header;
    header_ext_t *ext;
    uint8_t *payload;
    size_t payload_cap;
    size_t payload_len;
} fragment_t;

fragment_t *fragment_new(void);
void fragment_free(fragment_t *f);
int fragment_encode(const fragment_t *f, uint8_t *out, size_t outcap, size_t *outlen);
int fragment_parse(fragment_t *f, const uint8_t *buf, size_t buflen, size_t *consumed);

#endif

#define CARRIER_WS 0
#define CARRIER_GRPC 1
#define FLAG_ANCHOR 1
#define FLAG_ECHO_REQUESTED 2
#define FLAG_DUPLICATE 4

/* Minimal sender helper: build fragments for one update into out (returns count, -1 on error) */
int sender_enqueue_update(const uint8_t *payload, size_t payload_len, uint32_t object_id,
                          uint8_t *out, size_t outcap, size_t *outlen);

/* Minimal receiver helper */
typedef struct { uint16_t *cells; uint32_t w, h; } surface_canvas_t;
surface_canvas_t *canvas_new(uint32_t w, uint32_t h);
void canvas_free(surface_canvas_t *c);
int receiver_accept_fragment(surface_canvas_t *c, const uint8_t *data, size_t len);

/* Cell accounting */
typedef struct { uint32_t *pending; size_t pending_count, pending_cap; } cell_accounting_t;
cell_accounting_t *cell_accounting_new(void);
void cell_accounting_free(cell_accounting_t *a);
void cell_accounting_mark_pending(cell_accounting_t *a, const uint32_t *cells, size_t n);
void cell_accounting_mark_applied(cell_accounting_t *a, const uint32_t *cells, size_t n);
size_t cell_accounting_pending_count(cell_accounting_t *a);
int cell_accounting_converged(cell_accounting_t *a);

/* Scheduler helper */
int scheduler_max_chunks(int datagram, double loss_ratio);
