# Hamming Code

## Quick Recap

- Generic $(2^r-1,\ 2^r-1-r)$ single-error-correcting linear block code.
- Position `p` is a **parity** bit if `p` is a power of two (`1,2,4,8,...`); every other position is **data**.
- Decoding needs no stored `H` matrix and no lookup table: the syndrome, computed as the XOR of all set-bit *positions*, **is** the error position directly.
- `hamming_init(r)` selects the size: `r=3` → (7,4), `r=4` → (15,11), `r=5` → (31,26), etc.
- Corrects exactly 1 bit error per codeword ($t=1$). 2+ errors are usually mis-corrected, not just missed — see [Limitations](#limitations).

## Parameters

| r | n | k | rate k/n |
|---|---|---|---|
| 3 | 7 | 4 | 0.571 |
| 4 | 15 | 11 | 0.733 |
| 5 | 31 | 26 | 0.839 |
| 6 | 63 | 57 | 0.905 |

`r` is the only free parameter. `n = 2^r - 1`, `k = n - r` follow automatically — this is what makes the code "generic" rather than needing a hardcoded table per size.

## Position Scheme

Every position `1..n` doubles as its own binary address. No explicit generator/parity-check matrix is ever constructed in code; the matrix is implicit in how positions are numbered.

Example for r=3 (n=7):

| Position | Binary | Role |
|---|---|---|
| 1 | 001 | parity |
| 2 | 010 | parity |
| 3 | 011 | data |
| 4 | 100 | parity |
| 5 | 101 | data |
| 6 | 110 | data |
| 7 | 111 | data |

`is_parity_position(p)` checks `(p & (p-1)) == 0`, i.e. whether `p` has exactly one set bit.

## Encoding

1. Walk positions `1..n`; drop the `k` data bits into every non-power-of-two slot, in order.
2. For each parity position `p = 1<<j`, XOR together every position `q` whose `j`-th bit is set (excluding `p` itself). That XOR is the parity value.

Each parity bit only depends on data bits and other parity bits with a *different* power-of-two index, so they can be computed independently, in any order.

## Decoding

```
syndrome = XOR of all positions p where codeword[p] == 1
```

If `syndrome == 0`, no error. Otherwise, `syndrome` **is** the 1-indexed position to flip — no search, no stored matrix.

**Why this works:** the syndrome is mathematically `H · e^T` where `e` is the error vector. With this position-as-address scheme, `H`'s column for position `p` is just `p` written in binary. For a single-bit error at position `p`, `H · e^T` collapses to exactly that one column — i.e. the number `p` itself. XOR-ing every set-bit position of the received word computes precisely this, without ever materializing `H`.

## API

```c
hamming_code_t hamming_init(int r);
void hamming_encode(const hamming_code_t *hc, const uint8_t *data, uint8_t *codeword);
int  hamming_decode(const hamming_code_t *hc, uint8_t *codeword);  /* corrects in place, returns error position or 0 */
void hamming_extract_data(const hamming_code_t *hc, const uint8_t *codeword, uint8_t *data);
```

Usage:

```c
hamming_code_t hc = hamming_init(3);          /* (7,4) */
uint8_t data[4]     = {1, 0, 1, 1};
uint8_t codeword[7];
hamming_encode(&hc, data, codeword);

/* ... codeword goes through a channel, possibly flipping one bit ... */

int pos = hamming_decode(&hc, codeword);       /* corrects codeword in place */
uint8_t recovered[4];
hamming_extract_data(&hc, codeword, recovered);
```

## Limitations

- **Single-error correction only.** $t=1$ regardless of `r` — this is a structural property of the standard Hamming construction, not a tuning parameter (see `docs/` note on `d_min` if/when added).
- **2-bit errors are detected (syndrome ≠ 0) but typically mis-corrected** — the decoder will flip a different bit, making things worse, not just fail to fix them. Don't rely on this code for channels where multi-bit errors are common.
- For a configurable $t$ (correct more than 1 error), a different code family (BCH) is needed — out of scope for this module.

## Bit Representation

Each bit is currently one `uint8_t` (1 byte per bit) — simple to read/debug, wasteful in memory. Bit-packing (8 bits per byte) is a planned follow-up once correctness is fully validated; not done yet to keep the first implementation easy to verify against the unit tests.