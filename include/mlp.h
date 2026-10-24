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
#define CLASS_DATAGRAM 5
#define CLASS_FEC 6

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
#define TLV_PATH_ID 0x13
#define TLV_STREAM_ID 0x10
#define TLV_GREASE 0x7F

typedef struct { uint8_t kind; uint8_t *value; size_t len; } tlv_entry_t;
typedef struct { tlv_entry_t *entries; size_t count; size_t cap; } header_ext_t;

header_ext_t *hex_new(void);
void hex_free(header_ext_t *h);
int hex_push_u64(header_ext_t *h, uint8_t kind, uint64_t v);
int hex_push_u32(header_ext_t *h, uint8_t kind, uint32_t v);
int hex_push_u8(header_ext_t *h, uint8_t kind, uint8_t v);
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
    uint32_t stream_id;
    uint8_t path_id;
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

/* BWE helper */
typedef struct { double rtt_ms; uint64_t pace_bps; } bwe_estimate_t;
bwe_estimate_t bwe_estimate(uint64_t pace_floor_bps, double rtt_ms);

/* Input helper */
typedef struct { char code[32]; int pressed; uint64_t seq; } input_event_t;
typedef struct { input_event_t *events; size_t count, cap; } input_queue_t;
input_queue_t *input_queue_new(void);
void input_queue_free(input_queue_t *q);
void input_queue_submit(input_queue_t *q, const char *code, int pressed, uint64_t seq);
size_t input_queue_len(input_queue_t *q);

/* FEC helper */
void xor_parity(const uint8_t **data, const size_t *lens, size_t n, uint8_t *out, size_t outlen);

/* 0-RTT ticket helper */
typedef struct { uint8_t *payload; size_t len; uint64_t issued_at_ms; uint64_t nonce; } session_ticket_t;
session_ticket_t *ticket_issue(uint64_t last_applied_update_id, uint32_t chain_epoch, uint64_t now_ms);
void ticket_free(session_ticket_t *t);
int ticket_valid(session_ticket_t *t, uint64_t now_ms);
int ticket_validate(session_ticket_t *t, uint64_t now_ms, uint64_t *out_uid, uint32_t *out_epoch);

/* Connection migration helper */
typedef struct { int state; uint8_t active_path; uint8_t candidate; uint64_t challenge; uint32_t failures; } path_migration_t;
path_migration_t *migration_new(uint8_t active_path);
void migration_free(path_migration_t *m);
void migration_start_probe(path_migration_t *m, uint8_t candidate, uint64_t challenge);
int migration_complete(path_migration_t *m, uint64_t response);
int migration_is_probing(path_migration_t *m);

/* QoS helpers */
int qos_mlp_priority(int tier);
int qos_weight(int tier);

/* Loss detection helper */
typedef struct { uint64_t *lost; size_t lost_count, lost_cap; uint64_t largest_acked; int has_largest; uint64_t threshold; } loss_detector_t;
loss_detector_t *loss_detector_new(uint64_t threshold);
void loss_detector_free(loss_detector_t *ld);
void loss_detector_on_send(loss_detector_t *ld, uint64_t pn);
void loss_detector_on_ack(loss_detector_t *ld, const uint64_t *acked, size_t n);
int loss_detector_is_lost(loss_detector_t *ld, uint64_t pn);
size_t loss_detector_lost_count(loss_detector_t *ld);

/* Object assembler helper */
typedef struct { uint32_t object_id; uint16_t declared; uint16_t received; uint8_t **parts; size_t *part_lens; int complete; } open_object_t;
typedef struct { open_object_t *objects; size_t count, cap; } object_assembler_t;
object_assembler_t *assembler_new(void);
void assembler_free(object_assembler_t *a);
int assembler_accept(object_assembler_t *a, const uint8_t *data, size_t len, open_object_t **out);
size_t assembler_open_count(object_assembler_t *a);

/* Metrics helper */
typedef struct { uint64_t fragments_sent; uint64_t bytes_sent; uint64_t updates_applied; uint64_t convergences; } mlp_metrics_t;
mlp_metrics_t *metrics_new(void);
void metrics_free(mlp_metrics_t *m);
void metrics_fragment_sent(mlp_metrics_t *m, uint64_t bytes);
void metrics_update_applied(mlp_metrics_t *m);
size_t metrics_render(mlp_metrics_t *m, char *buf, size_t cap);

/* Path probing helper */
typedef struct { uint8_t path_id; double rtt_ms; double loss_ratio; uint64_t goodput_bps; } probe_result_t;
typedef struct { probe_result_t *results; size_t count, cap; uint64_t interval_ms; } path_prober_t;
path_prober_t *prober_new(uint64_t interval_ms);
void prober_free(path_prober_t *p);
void prober_record(path_prober_t *p, probe_result_t r);
int prober_best_path(path_prober_t *p); /* returns path_id or -1 */

/* Pacer helper */
typedef struct { uint64_t pace_bps; uint64_t max_grant; double tokens; double last_refill; } simple_pacer_t;
simple_pacer_t *pacer_new(uint64_t pace_bps, uint64_t max_grant);
void pacer_free(simple_pacer_t *p);
int pacer_try_acquire(simple_pacer_t *p, uint64_t bytes, double now_sec);
uint64_t pacer_budget(simple_pacer_t *p);
