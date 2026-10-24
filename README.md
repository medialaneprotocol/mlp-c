# mlp-c

C reference implementation of the Media Lane Protocol (MLP) wire format.

Mirrors `medialaneprotocol/mlp-core` (Rust) byte-for-byte. Currently implements
the PDU codec: fragment header (22 bytes), TLV header extension, and update
metadata. Full sender/receiver and scheduler are planned.

## Build & test

```sh
gcc -Iinclude src/mlp.c tests/test_golden.c -o test_golden && ./test_golden
```
