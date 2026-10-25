# mlp-c

C implementation of the Media Lane Protocol (MLP) wire format.

The wire is frozen by the [specification](https://github.com/medialaneprotocol/mlp):
this package's PDU codec — fragment header, TLV header extension, update
metadata, carriers — matches `medialaneprotocol/mlp-core` (Rust) byte for
byte, and the golden test asserts the exact bytes of a reference-encoded
fragment.

## What it implements

- **The codec** (`include/mlp.h`, `src/mlp.c`): classes `0..=4` (5..=255
  rejected), the flag byte with bit 7 reserved, the full TLV registry, the
  five carriers with their payload and message caps, the
  `fragment_count ≤ 4096` cap, media's reserved priority band.
- **Object assembly**: FIN-gated completion under the spec's budgets — four
  open objects, one megabyte of partial payloads, a 4096-id dedup window.
- **The 64-update ack window** (`ack_window_t`), the way `mlp-ts` and
  `mlp-go` carry it.
- **The lane vocabulary**: the ten wire states, the park table with its TTL,
  the pressed-set input manager, flow-style credit helpers, gap-based loss
  detection over a real sent set, the token-bucket pacer, QoS tiers, 0-RTT
  tickets, path migration and probing, and the eight-counter Prometheus
  renderer.

## What it does not implement (yet)

The pacing machines, the bandwidth estimator, the repair plans and the
scheduler's priority bands are the Rust machine's. The package exposes no
placeholder for any of them.

## Build & test

```sh
make test
```

The conformance test (`tests/test_interop.c`) parses every vector in
`conformance/vectors.json` — the same corpus `mlp-core`, `mlp-go` and
`mlp-py` test against — and the golden test asserts exact bytes.
