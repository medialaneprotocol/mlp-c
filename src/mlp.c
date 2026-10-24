#include "mlp.h"
#include <stdlib.h>
#include <string.h>

header_ext_t *hex_new(void) {
    header_ext_t *h = calloc(1, sizeof(*h));
    h->cap = 8;
    h->entries = calloc(h->cap, sizeof(tlv_entry_t));
    return h;
}
void hex_free(header_ext_t *h) {
    if (!h) return;
    for (size_t i = 0; i < h->count; i++) free(h->entries[i].value);
    free(h->entries);
    free(h);
}

static int hex_grow(header_ext_t *h) {
    if (h->count < h->cap) return 0;
    size_t nc = h->cap * 2;
    tlv_entry_t *ne = realloc(h->entries, nc * sizeof(tlv_entry_t));
    if (!ne) return -1;
    h->entries = ne;
    h->cap = nc;
    return 0;
}

int hex_push_raw(header_ext_t *h, uint8_t kind, const uint8_t *v, size_t len) {
    if (hex_grow(h) != 0) return -1;
    uint8_t *buf = malloc(len);
    if (!buf && len) return -1;
    memcpy(buf, v, len);
    h->entries[h->count].kind = kind;
    h->entries[h->count].value = buf;
    h->entries[h->count].len = len;
    h->count++;
    return 0;
}

int hex_push_u64(header_ext_t *h, uint8_t kind, uint64_t v) {
    uint8_t b[8];
    for (int i = 0; i < 8; i++) b[i] = (v >> (8*i)) & 0xff;
    return hex_push_raw(h, kind, b, 8);
}
int hex_push_u8(header_ext_t *h, uint8_t kind, uint8_t v) {
    uint8_t b[1] = { v };
    return hex_push_raw(h, kind, b, 1);
}
int hex_push_u32(header_ext_t *h, uint8_t kind, uint32_t v) {
    uint8_t b[4];
    for (int i = 0; i < 4; i++) b[i] = (v >> (8*i)) & 0xff;
    return hex_push_raw(h, kind, b, 4);
}
int hex_push_u16pair(header_ext_t *h, uint8_t kind, uint16_t a, uint16_t b) {
    uint8_t buf[4];
    buf[0]=a&0xff; buf[1]=(a>>8)&0xff; buf[2]=b&0xff; buf[3]=(b>>8)&0xff;
    return hex_push_raw(h, kind, buf, 4);
}

const uint8_t *hex_get(header_ext_t *h, uint8_t kind, size_t *outlen) {
    for (size_t i = 0; i < h->count; i++) {
        if (h->entries[i].kind == kind) {
            if (outlen) *outlen = h->entries[i].len;
            return h->entries[i].value;
        }
    }
    if (outlen) *outlen = 0;
    return NULL;
}

static uint64_t rd64(const uint8_t *p){ uint64_t v=0; for(int i=0;i<8;i++) v |= (uint64_t)p[i]<<(8*i); return v; }
static uint32_t rd32(const uint8_t *p){ uint32_t v=0; for(int i=0;i<4;i++) v |= (uint32_t)p[i]<<(8*i); return v; }
static uint16_t rd16(const uint8_t *p){ return (uint16_t)p[0] | ((uint16_t)p[1]<<8); }

size_t hex_encoded_len(header_ext_t *h) {
    size_t n=0;
    for (size_t i=0;i<h->count;i++) n += 2 + h->entries[i].len;
    return n;
}

int hex_encode(header_ext_t *h, uint8_t *out, size_t outcap, size_t *outlen) {
    size_t n = hex_encoded_len(h);
    if (outcap < n) return -1;
    size_t off=0;
    for (size_t i=0;i<h->count;i++) {
        out[off++]=h->entries[i].kind;
        out[off++]=(uint8_t)h->entries[i].len;
        memcpy(out+off, h->entries[i].value, h->entries[i].len);
        off += h->entries[i].len;
    }
    if (outlen) *outlen = n;
    return 0;
}

int hex_parse(header_ext_t *h, const uint8_t *b, size_t n) {
    size_t off=0;
    while (off < n) {
        if (off+2 > n) return -1;
        uint8_t kind = b[off];
        uint8_t l = b[off+1];
        off += 2;
        if (off + l > n) return -1;
        if (hex_push_raw(h, kind, b+off, l) != 0) return -1;
        off += l;
    }
    return 0;
}

header_ext_t *update_meta_to_ext(const update_meta_t *m) {
    header_ext_t *h = hex_new();
    hex_push_u64(h, TLV_UPDATE_ID, m->update_id);
    hex_push_u64(h, TLV_UPDATE_BASE, m->update_base);
    hex_push_u16pair(h, TLV_PART, m->part_index, m->part_count);
    hex_push_u32(h, TLV_UPDATE_BYTES, m->update_bytes);
    hex_push_u32(h, TLV_CHAIN_EPOCH, m->chain_epoch);
    hex_push_u64(h, TLV_REFLECTS_INPUT_SEQ, m->reflects_input_seq);
    hex_push_u32(h, TLV_STREAM_ID, m->stream_id);
    hex_push_u8(h, TLV_PATH_ID, m->path_id);
    hex_push_u64(h, TLV_SEND_EPOCH_MS, m->send_epoch_ms);
    return h;
}

int update_meta_from_ext(const header_ext_t *ext, update_meta_t *out) {
    memset(out, 0, sizeof(*out));
    size_t l;
    const uint8_t *v;
    v = hex_get((header_ext_t*)ext, TLV_UPDATE_ID, &l);
    if (!v || l != 8) return -1;
    out->update_id = rd64(v);
    v = hex_get((header_ext_t*)ext, TLV_UPDATE_BASE, &l);
    if (v && l==8) out->update_base = rd64(v);
    v = hex_get((header_ext_t*)ext, TLV_PART, &l);
    if (v && l==4) { out->part_index = rd16(v); out->part_count = rd16(v+2); }
    else { out->part_count = 1; }
    v = hex_get((header_ext_t*)ext, TLV_UPDATE_BYTES, &l);
    if (v && l==4) out->update_bytes = rd32(v);
    v = hex_get((header_ext_t*)ext, TLV_CHAIN_EPOCH, &l);
    if (v && l==4) out->chain_epoch = rd32(v);
    v = hex_get((header_ext_t*)ext, TLV_REFLECTS_INPUT_SEQ, &l);
    if (v && l==8) out->reflects_input_seq = rd64(v);
    v = hex_get((header_ext_t*)ext, TLV_STREAM_ID, &l);
    if (v && l==4) out->stream_id = rd32(v);
    v = hex_get((header_ext_t*)ext, TLV_PATH_ID, &l);
    if (v && l==1) out->path_id = v[0];
    v = hex_get((header_ext_t*)ext, TLV_SEND_EPOCH_MS, &l);
    if (v && l==8) out->send_epoch_ms = rd64(v);
    return 0;
}

fragment_t *fragment_new(void) {
    fragment_t *f = calloc(1, sizeof(*f));
    f->ext = hex_new();
    return f;
}
void fragment_free(fragment_t *f) {
    if (!f) return;
    hex_free(f->ext);
    free(f->payload);
    free(f);
}

int fragment_encode(const fragment_t *f, uint8_t *out, size_t outcap, size_t *outlen) {
    size_t extlen = hex_encoded_len(f->ext);
    size_t total = FRAGMENT_FIXED_HEADER + extlen + f->payload_len;
    if (outcap < total) return -1;
    out[0]=FRAGMENT_MAGIC_0; out[1]=FRAGMENT_MAGIC_1; out[2]=FRAGMENT_MAGIC_2; out[3]=FRAGMENT_MAGIC_3;
    out[4]=f->header.version;
    out[5]=f->header.cls;
    out[6]=f->header.flags;
    out[7]=f->header.priority;
    /* object_id LE */
    for (int i=0;i<4;i++) out[8+i] = (f->header.object_id >> (8*i)) & 0xff;
    out[12]=f->header.fragment_index & 0xff; out[13]=(f->header.fragment_index>>8)&0xff;
    out[14]=f->header.fragment_count & 0xff; out[15]=(f->header.fragment_count>>8)&0xff;
    for (int i=0;i<4;i++) out[16+i] = (f->header.payload_len >> (8*i)) & 0xff;
    out[20]=extlen & 0xff; out[21]=(extlen>>8)&0xff;
    size_t off = FRAGMENT_FIXED_HEADER;
    size_t el;
    hex_encode(f->ext, out+off, outcap-off, &el);
    off += el;
    memcpy(out+off, f->payload, f->payload_len);
    off += f->payload_len;
    if (outlen) *outlen = off;
    return 0;
}

int fragment_parse(fragment_t *f, const uint8_t *buf, size_t buflen, size_t *consumed) {
    if (buflen < FRAGMENT_FIXED_HEADER) return -1;
    if (buf[0]!=FRAGMENT_MAGIC_0 || buf[1]!=FRAGMENT_MAGIC_1 || buf[2]!=FRAGMENT_MAGIC_2 || buf[3]!=FRAGMENT_MAGIC_3) return -1;
    f->header.version = buf[4];
    if (f->header.version != MEDIA_PROTOCOL_VERSION) return -1;
    f->header.cls = buf[5];
    f->header.flags = buf[6];
    f->header.priority = buf[7];
    if (f->header.priority > 7) return -1;
    f->header.object_id = rd32(buf+8);
    f->header.fragment_index = rd16(buf+12);
    f->header.fragment_count = rd16(buf+14);
    f->header.payload_len = rd32(buf+16);
    f->header.header_ext_len = rd16(buf+20);
    if (f->header.fragment_count == 0 || f->header.fragment_index >= f->header.fragment_count) return -1;
    if (f->header.cls == CLASS_MEDIA && f->header.priority <= PRIO_INPUT) return -1;
    uint16_t extlen = f->header.header_ext_len;
    if (extlen != 0 && extlen < 2) return -1;
    if (extlen > MAX_HEADER_EXT) return -1;
    size_t extEnd = FRAGMENT_FIXED_HEADER + extlen;
    size_t payloadEnd = extEnd + f->header.payload_len;
    if (buflen < payloadEnd) return -1;
    hex_free(f->ext);
    f->ext = hex_new();
    if (hex_parse(f->ext, buf+FRAGMENT_FIXED_HEADER, extlen) != 0) return -1;
    free(f->payload);
    f->payload = malloc(f->header.payload_len ? f->header.payload_len : 1);
    memcpy(f->payload, buf+extEnd, f->header.payload_len);
    f->payload_len = f->header.payload_len;
    if (consumed) *consumed = payloadEnd;
    return 0;
}

int sender_enqueue_update(const uint8_t *payload, size_t payload_len, uint32_t object_id,
                          uint8_t *out, size_t outcap, size_t *outlen) {
    /* Single-fragment update (payload must fit). */
    update_meta_t m;
    memset(&m, 0, sizeof(m));
    m.update_id = object_id;
    m.update_base = 0;
    m.part_index = 0;
    m.part_count = 1;
    m.update_bytes = (uint32_t)payload_len;
    m.chain_epoch = 1;
    m.send_epoch_ms = 0;
    header_ext_t *ext = update_meta_to_ext(&m);
    fragment_t *f = fragment_new();
    hex_free(f->ext);
    f->ext = ext;
    f->header.version = MEDIA_PROTOCOL_VERSION;
    f->header.cls = CLASS_MEDIA;
    f->header.flags = 0;
    f->header.priority = PRIO_MEDIA;
    f->header.object_id = object_id;
    f->header.fragment_index = 0;
    f->header.fragment_count = 1;
    f->header.payload_len = (uint32_t)payload_len;
    f->payload = malloc(payload_len ? payload_len : 1);
    memcpy(f->payload, payload, payload_len);
    f->payload_len = payload_len;
    size_t n = 0;
    if (fragment_encode(f, out, outcap, &n) != 0) { fragment_free(f); return -1; }
    if (outlen) *outlen = n;
    fragment_free(f);
    return 1;
}

surface_canvas_t *canvas_new(uint32_t w, uint32_t h) {
    surface_canvas_t *c = calloc(1, sizeof(*c));
    c->w = w; c->h = h;
    c->cells = calloc((size_t)w * h, sizeof(uint16_t));
    return c;
}
void canvas_free(surface_canvas_t *c) {
    if (!c) return;
    free(c->cells);
    free(c);
}
int receiver_accept_fragment(surface_canvas_t *c, const uint8_t *data, size_t len) {
    fragment_t *f = fragment_new();
    size_t consumed = 0;
    if (fragment_parse(f, data, len, &consumed) != 0) { fragment_free(f); return -1; }
    if (consumed != len) { fragment_free(f); return -1; }
    fragment_free(f);
    return 0;
}

static int ca_contains(cell_accounting_t *a, uint32_t c) {
    for (size_t i = 0; i < a->pending_count; i++) if (a->pending[i] == c) return 1;
    return 0;
}
static void ca_remove(cell_accounting_t *a, uint32_t c) {
    for (size_t i = 0; i < a->pending_count; i++) {
        if (a->pending[i] == c) {
            a->pending[i] = a->pending[--a->pending_count];
            return;
        }
    }
}
cell_accounting_t *cell_accounting_new(void) {
    cell_accounting_t *a = calloc(1, sizeof(*a));
    a->pending_cap = 16;
    a->pending = calloc(a->pending_cap, sizeof(uint32_t));
    return a;
}
void cell_accounting_free(cell_accounting_t *a) {
    if (!a) return;
    free(a->pending);
    free(a);
}
void cell_accounting_mark_pending(cell_accounting_t *a, const uint32_t *cells, size_t n) {
    for (size_t i = 0; i < n; i++) {
        if (!ca_contains(a, cells[i])) {
            if (a->pending_count == a->pending_cap) {
                a->pending_cap *= 2;
                a->pending = realloc(a->pending, a->pending_cap * sizeof(uint32_t));
            }
            a->pending[a->pending_count++] = cells[i];
        }
    }
}
void cell_accounting_mark_applied(cell_accounting_t *a, const uint32_t *cells, size_t n) {
    for (size_t i = 0; i < n; i++) ca_remove(a, cells[i]);
}
size_t cell_accounting_pending_count(cell_accounting_t *a) { return a->pending_count; }
int cell_accounting_converged(cell_accounting_t *a) { return a->pending_count == 0; }

int scheduler_max_chunks(int datagram, double loss_ratio) {
    (void)loss_ratio;
    if (datagram) return 1;
    return 8;
}

bwe_estimate_t bwe_estimate(uint64_t pace_floor_bps, double rtt_ms) {
    bwe_estimate_t e;
    e.rtt_ms = rtt_ms;
    e.pace_bps = pace_floor_bps;
    return e;
}

input_queue_t *input_queue_new(void) {
    input_queue_t *q = calloc(1, sizeof(*q));
    q->cap = 16;
    q->events = calloc(q->cap, sizeof(input_event_t));
    return q;
}
void input_queue_free(input_queue_t *q) {
    if (!q) return;
    free(q->events);
    free(q);
}
void input_queue_submit(input_queue_t *q, const char *code, int pressed, uint64_t seq) {
    if (q->count == q->cap) {
        q->cap *= 2;
        q->events = realloc(q->events, q->cap * sizeof(input_event_t));
    }
    strncpy(q->events[q->count].code, code, 31);
    q->events[q->count].pressed = pressed;
    q->events[q->count].seq = seq;
    q->count++;
}
size_t input_queue_len(input_queue_t *q) { return q->count; }

void xor_parity(const uint8_t **data, const size_t *lens, size_t n, uint8_t *out, size_t outlen) {
    memset(out, 0, outlen);
    for (size_t i = 0; i < n; i++) {
        for (size_t j = 0; j < lens[i] && j < outlen; j++) {
            out[j] ^= data[i][j];
        }
    }
}

session_ticket_t *ticket_issue(uint64_t last_applied_update_id, uint32_t chain_epoch, uint64_t now_ms) {
    session_ticket_t *t = calloc(1, sizeof(*t));
    t->payload = calloc(12, 1);
    for (int i = 0; i < 8; i++) t->payload[i] = (last_applied_update_id >> (8*i)) & 0xff;
    for (int i = 0; i < 4; i++) t->payload[8+i] = (chain_epoch >> (8*i)) & 0xff;
    t->len = 12;
    t->issued_at_ms = now_ms;
    t->nonce = now_ms ^ 0x9e3779b97f4a7c15ULL;
    return t;
}
void ticket_free(session_ticket_t *t) {
    if (!t) return;
    free(t->payload);
    free(t);
}
int ticket_valid(session_ticket_t *t, uint64_t now_ms) {
    if (!t) return 0;
    if (now_ms < t->issued_at_ms) return 0;
    if (now_ms - t->issued_at_ms > 3600*1000) return 0;
    return 1;
}
int ticket_validate(session_ticket_t *t, uint64_t now_ms, uint64_t *out_uid, uint32_t *out_epoch) {
    if (!ticket_valid(t, now_ms)) return 0;
    if (t->len < 12) return 0;
    uint64_t uid = 0; uint32_t ep = 0;
    for (int i = 0; i < 8; i++) uid |= (uint64_t)t->payload[i] << (8*i);
    for (int i = 0; i < 4; i++) ep |= (uint32_t)t->payload[8+i] << (8*i);
    if (out_uid) *out_uid = uid;
    if (out_epoch) *out_epoch = ep;
    return 1;
}

path_migration_t *migration_new(uint8_t active_path) {
    path_migration_t *m = calloc(1, sizeof(*m));
    m->active_path = active_path;
    m->state = 0; /* IDLE */
    return m;
}
void migration_free(path_migration_t *m) { free(m); }
void migration_start_probe(path_migration_t *m, uint8_t candidate, uint64_t challenge) {
    m->state = 1; /* PROBING */
    m->candidate = candidate;
    m->challenge = challenge;
}
int migration_complete(path_migration_t *m, uint64_t response) {
    if (m->state != 1) return 0;
    if (response == m->challenge) {
        m->active_path = m->candidate;
        m->state = 2; /* MIGRATED */
        return 1;
    }
    m->state = 3; /* FAILED */
    m->failures++;
    return 0;
}
int migration_is_probing(path_migration_t *m) { return m->state == 1; }
