# ecc-lib

A C library of error-correcting codes: generic Hamming codes, BSC/AWGN channel models, and a Polar encoder + successive-cancellation decoder using the 3GPP TS 38.212 frozen-bit sequence (core N=2^n transform). Designed to be portable across constrained MCUs and general-purpose Linux targets — no external dependencies.

## Status

Implemented:
- Generic Hamming(2^r-1, 2^r-1-r) encoder/decoder
- Binary symmetric channel (BSC) simulator
- AWGN channel model (BPSK, direct LLR output)
- Polar encoder + SC decoder (3GPP TS 38.212 reliability sequence, core N=2^n transform)

Not implemented (out of scope for this project):
- LDPC (5G NR base graphs, min-sum decoding)
- 5G NR polar rate matching (puncturing/shortening/interleaving)
- Polar CRC-aided SCL decoding
- Bit-packed storage for memory-constrained targets

## Build

```sh
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

## Project Structure

```
ecc-lib/
├── include/ecc/           public headers, flat (channel.h, hamming.h, polar.h, ...)
├── src/<module>/          implementation, grouped by coder/module (private headers/tables live here too)
├── tests/
│   ├── unit/<module>/     single-module correctness tests
│   └── integration/<module>/  cross-module tests (e.g. channel + hamming BER)
├── examples/              standalone CLI demos
└── docs/                  design notes, one file per coder
```

## Documentation

- [docs/Hamming.md](docs/Hamming.md) — Hamming code: parameters, position scheme, why decoding needs no stored parity-check matrix.
- [docs/Polar.md](docs/Polar.md) — Polar code: channel polarization, 3GPP frozen-bit selection, encoding, SC decoding, limitations.
- [docs/LDPC.md](docs/LDPC.md) — LDPC: design notes only, not implemented. Base graph/lifting theory and scoping findings for anyone resuming this later.

## Requirements

- C11-compatible compiler
- CMake >= 3.15
- No external dependencies

## License

MIT — see [LICENSE](LICENSE).

## Note on Patents

This library implements algorithms described in publicly published 3GPP specifications for educational and research purposes. Standards like 5G NR may be covered by third-party standard-essential patents (SEPs) licensed under FRAND terms; this project makes no claim to, and grants no rights under, any such patents. Commercial deployment of 3GPP-standardized technology may require separate patent licensing — consult qualified counsel if that applies to you.
