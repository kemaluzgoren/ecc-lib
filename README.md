# ecc-lib

A C library of error-correcting codes, built incrementally from Hamming through LDPC and Polar codes (targeting the 5G NR standard). Designed to be portable across constrained MCUs and general-purpose Linux targets — no external dependencies.

## Status

- [x] Generic Hamming(2^r-1, 2^r-1-r) encoder/decoder
- [x] Binary symmetric channel (BSC) simulator
- [ ] LDPC (5G NR base graphs, min-sum decoding) — planned
- [ ] Polar (SC/SCL decoding) — planned
- [ ] AWGN channel model — planned
- [ ] Bit-packed storage for memory-constrained targets — planned

## Build

```sh
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

## Project Structure

```
ecc-lib/
├── include/ecc/          public headers (channel.h, hamming.h, ...)
├── src/                  implementation
├── tests/
│   ├── unit/<module>/     single-module correctness tests
│   └── integration/<module>/  cross-module tests (e.g. channel + hamming BER)
├── examples/              standalone CLI demos
└── docs/                  design notes, one file per coder
```

## Documentation

- [docs/hamming.md](docs/hamming.md) — Hamming code: parameters, position scheme, why decoding needs no stored parity-check matrix.

## Requirements

- C11-compatible compiler
- CMake >= 3.15
- No external dependencies

## License

MIT — see [LICENSE](LICENSE).
