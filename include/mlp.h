#ifndef MLP_H
#define MLP_H
#include <stdint.h>
#include <stddef.h>

/* The Media Lane Protocol wire format, in C. The wire is frozen by the
 * specification; these constants match mlp-core (Rust) byte for byte. */

#define FRAGMENT_MAGIC_0 'M'
#define FRAGMENT_MAGIC_1 'L'
#define FRAGMENT_MAGIC_2 'P'
#define FRAGMENT_MAGIC_3 '\0'
#define MEDIA_PROTOCOL_VERSION 1
#define FRAGMENT_FIXED_HEADER 22
#define MAX_FRAGMENT_COUNT 4096
#define MAX_HEADER_EXT 256
#define MAX_STREAM_FRAGMENT 65536
#define MAX_GRPC_COALESCE 65536
#define TARGET_FRAGMENT_PAYLOAD 1150

/* Fragment classes: the wire freezes 0..=4; 5..=255 are reserved. */
#define CLASS_CONTROL 0
#define CLASS_CURSOR 1
#define CLASS_MEDIA 2
#define CLASS_FEEDBACK 3
#define CLASS_PATH 4

#define PRIO_CONTROL 0
#define PRIO_INPUT 1
#define PRIO_REPORT 2
#define PRIO_CURSOR 5
#define PRIO_MEDIA 6
#define PRIO_REFRESH 7

/* The flags byte: bit 7 is reserved and must be zero. */
#define FLAG_FIN 0x01
#define FLAG_ANCHOR 0x02
#define FLAG_RETRANSMIT 0x04
#define FLAG_DUP 0x08
#define FLAG_ECHO_REQUESTED 0x10
#define FLAG_PROBE 0x20
#define FLAG_URGENT 0x40

/* Carriers. */
#define CARRIER_WS 0
#define CARRIER_GRPC 1
#define CARRIER_DATA_CHANNEL 2
#define CARRIER_REPAIR_CHANNEL 3
#define CARRIER_CONTROL_CHANNEL 4
size_t carrier_fragment_payload_max(int carrier);
size_t carrier_message_cap(int carrier);
int carrier_is_datagram(int carrier);

/* TLV kinds. 0x10..=0xEF are skipped silently; 0xF0..=0xFF experimental. */
#define TLV_UPDATE_ID 0x01
#define TLV_UPDATE_BASE 0x02
#define TLV_PART 0x03
#define TLV_UPDATE_BYTES 0x04
#define TLV_CHAIN_EPOCH 0x05
#define TLV_REFLECTS_INPUT_SEQ 0x06
#define TLV_SEND_EPOCH_MS 0x07
#define TLV_DEADLINE_EPOCH_MS 0x08
#define TLV_ECHO_OF_OBJECT_ID 0x09
#define TLV_BUDGET_BYTES 0x0A
#define TLV_PACE_BPS 0x0B
#define TLV_PATH_HINT 0x0C
#define TLV_INPUT_SEQ 0x0D
#define TLV_CELL_COUNT 0x0E
#define TLV_COST_MS 0x0F
#define TLV_STREAM_ID 0x10
#define TLV_FEC_GROUP_ID 0x11
#define TLV_FEC_K 0x12
#define TLV_PATH_ID 0x13
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

/* Sender helper: build the fragments of one update into out (returns the
 * fragment count, or -1 on error). The last fragment carries FIN. */
int sender_enqueue_update(const uint8_t *payload, size_t payload_len, uint32_t object_id,
                          uint8_t *out, size_t outcap, size_t *outlen);

/* Receiver helper: parse-and-accept one encoded fragment (0) or reject it (-1). */
int receiver_accept_fragment(const uint8_t *data, size_t len);

/* Cell accounting. */
typedef struct { uint32_t *pending; size_t pending_count, pending_cap; } cell_accounting_t;
cell_accounting_t *cell_accounting_new(void);
void cell_accounting_free(cell_accounting_t *a);
void cell_accounting_mark_pending(cell_accounting_t *a, const uint32_t *cells, size_t n);
void cell_accounting_mark_applied(cell_accounting_t *a, const uint32_t *cells, size_t n);
size_t cell_accounting_pending_count(cell_accounting_t *a);
int cell_accounting_converged(cell_accounting_t *a);

/* The wire-level half of the scheduler: the part-count rule. */
int scheduler_max_parts(int datagram, double loss_ratio);

/* Input helpers. */
typedef struct { char code[32]; int pressed; uint64_t seq; } input_event_t;
typedef struct { input_event_t *events; size_t count, cap; } input_queue_t;
input_queue_t *input_queue_new(void);
void input_queue_free(input_queue_t *q);
void input_queue_submit(input_queue_t *q, const char *code, int pressed, uint64_t seq);
size_t input_queue_len(input_queue_t *q);

typedef struct { uint64_t next_seq; char **pressed; size_t pressed_count, pressed_cap; uint64_t *deadlines; uint64_t *submitted; size_t sub_count, sub_cap; } input_manager_t;
input_manager_t *input_manager_new(void);
void input_manager_free(input_manager_t *m);
uint64_t input_manager_submit(input_manager_t *m, const char *code, int pressed, uint64_t deadline_ms);
int input_manager_is_pressed(input_manager_t *m, const char *code);
size_t input_manager_pressed_count(input_manager_t *m);
size_t input_manager_expired(input_manager_t *m, uint64_t now_ms, uint64_t *out);

/* 0-RTT ticket helper (a plaintext carrier of the resume point). */
typedef struct { uint8_t *payload; size_t len; uint64_t issued_at_ms; uint64_t nonce; } session_ticket_t;
session_ticket_t *ticket_issue(uint64_t last_applied_update_id, uint32_t chain_epoch, uint64_t now_ms);
void ticket_free(session_ticket_t *t);
int ticket_valid(session_ticket_t *t, uint64_t now_ms);
int ticket_validate(session_ticket_t *t, uint64_t now_ms, uint64_t *out_uid, uint32_t *out_epoch);

/* Connection migration helper. */
typedef struct { int state; uint8_t active_path; uint8_t candidate; uint64_t challenge; uint32_t failures; } path_migration_t;
path_migration_t *migration_new(uint8_t active_path);
void migration_free(path_migration_t *m);
void migration_start_probe(path_migration_t *m, uint8_t candidate, uint64_t challenge);
int migration_complete(path_migration_t *m, uint64_t response);
int migration_is_probing(path_migration_t *m);

/* QoS helpers. */
int qos_mlp_priority(int tier);
int qos_weight(int tier);

/* Loss detection helper: three acks past a packet declare it lost. */
typedef struct { uint64_t *sent; size_t sent_count, sent_cap; uint64_t *acked; size_t acked_count, acked_cap; uint64_t *lost; size_t lost_count, lost_cap; uint64_t largest_acked; int has_largest; uint64_t threshold; } loss_detector_t;
loss_detector_t *loss_detector_new(uint64_t threshold);
void loss_detector_free(loss_detector_t *ld);
void loss_detector_on_send(loss_detector_t *ld, uint64_t pn);
void loss_detector_on_ack(loss_detector_t *ld, const uint64_t *acked, size_t n);
int loss_detector_is_lost(loss_detector_t *ld, uint64_t pn);
size_t loss_detector_lost_count(loss_detector_t *ld);

/* Object assembler: FIN-gated completion under the spec's budgets. */
#define MAX_OPEN_OBJECTS 4
#define MAX_ASSEMBLY_BYTES (1u << 20)
#define DEDUP_WINDOW 4096
typedef struct { uint32_t object_id; uint16_t declared; uint16_t received; uint8_t **parts; size_t *part_lens; size_t bytes_held; int saw_fin; int complete; } open_object_t;
typedef struct { open_object_t *objects; size_t count, cap; uint32_t *seen; size_t seen_count, seen_cap; size_t bytes; } object_assembler_t;
object_assembler_t *assembler_new(void);
void assembler_free(object_assembler_t *a);
/* Returns 1 when this fragment completed the object, 0 while pending,
 * -1 on a parse error, a contradiction, or a budget overrun. */
int assembler_accept(object_assembler_t *a, const uint8_t *data, size_t len, open_object_t **out);
size_t assembler_open_count(object_assembler_t *a);

/* The 64-update ack window. Bit 0 of the bitmap names the highest applied
 * update; update id 0 names no update. */
typedef struct { uint64_t highest; uint64_t bits; } ack_window_t;
void ack_window_reset(ack_window_t *w);
void ack_window_mark(ack_window_t *w, uint64_t update_id);
int ack_window_has(const ack_window_t *w, uint64_t update_id);
void ack_window_seed_from_resume(ack_window_t *w, uint64_t highest_applied);

/* Metrics helper: the telemetry sink's eight counters. */
typedef struct { uint64_t fragments_sent, fragments_received, bytes_sent, bytes_received, updates_applied, convergences, losses, echoes; } mlp_metrics_t;
mlp_metrics_t *metrics_new(void);
void metrics_free(mlp_metrics_t *m);
void metrics_fragment_sent(mlp_metrics_t *m, uint64_t bytes);
void metrics_fragment_received(mlp_metrics_t *m, uint64_t bytes);
void metrics_update_applied(mlp_metrics_t *m);
void metrics_convergence(mlp_metrics_t *m);
void metrics_loss(mlp_metrics_t *m);
void metrics_echo(mlp_metrics_t *m);
size_t metrics_render(mlp_metrics_t *m, char *buf, size_t cap);

/* Path probing helper. */
typedef struct { uint8_t path_id; double rtt_ms; double loss_ratio; uint64_t goodput_bps; } probe_result_t;
typedef struct { probe_result_t *results; size_t count, cap; uint64_t interval_ms; } path_prober_t;
path_prober_t *prober_new(uint64_t interval_ms);
void prober_free(path_prober_t *p);
void prober_record(path_prober_t *p, probe_result_t r);
int prober_best_path(path_prober_t *p); /* returns path_id or -1 */

/* Pacer helper: a token bucket with a 1.25x burst allowance. */
typedef struct { uint64_t pace_bps; uint64_t max_grant; double tokens; double last_refill; } simple_pacer_t;
simple_pacer_t *pacer_new(uint64_t pace_bps, uint64_t max_grant);
void pacer_free(simple_pacer_t *p);
int pacer_try_acquire(simple_pacer_t *p, uint64_t bytes, double now_sec);
uint64_t pacer_budget(simple_pacer_t *p);

/* Lane policy helper. */
typedef struct { int lane_mode; int media_lane_enabled; } media_lane_policy_t;
media_lane_policy_t *policy_new(int enabled);
void policy_free(media_lane_policy_t *p);

/* The lane's wire states (MLP spec §7): ten values, 0..=9. */
#define LANE_STATE_STARTING 0
#define LANE_STATE_LIVE 1
#define LANE_STATE_DEGRADED 2
#define LANE_STATE_CONVERGING 3
#define LANE_STATE_STALE 4
#define LANE_STATE_PARKED 5
#define LANE_STATE_DISABLED 6
#define LANE_STATE_DECODE_BOUND 7
#define LANE_STATE_NO_FEEDBACK 8
#define LANE_STATE_MEDIA_UNSUPPORTED 9

typedef struct { int state; uint64_t since_ms; int converged; uint64_t pending_cells; uint64_t last_applied_ms; int has_applied; uint64_t stale_budget_ms; int no_feedback; double loss_ratio; } lane_lifecycle_t;
lane_lifecycle_t *lifecycle_new(void);
void lifecycle_free(lane_lifecycle_t *l);
void lifecycle_set_applied(lane_lifecycle_t *l, uint64_t now_ms, uint64_t pending_cells);

/* The park table and its TTLs (MLP spec §9.2). */
#define RESUME_TTL_SECONDS 900u
#define PARK_TTL_FLOOR_SECONDS 120u
#define PARK_TTL_CEIL_SECONDS 900u
#define PARK_TTL_DEFAULT 300u
typedef struct { int parked; uint64_t last_applied_id; uint32_t chain_epoch; uint64_t pending_cells; uint64_t parked_at_ms; uint64_t ttl_seconds; } resume_manager_t;
resume_manager_t *resume_manager_new(void);
void resume_manager_free(resume_manager_t *r);
void resume_manager_park(resume_manager_t *r, uint64_t last_applied_id, uint32_t chain_epoch, uint64_t pending_cells, uint64_t now_ms);
int resume_manager_resume(resume_manager_t *r, uint64_t now_ms, uint64_t *out_id, uint32_t *out_epoch, uint64_t *out_pending);
int resume_manager_is_parked(resume_manager_t *r);

#endif
