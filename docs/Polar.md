# Polar Code

## Quick Recap

- Arikan polar code, core transform only: block length $N = 2^n$ exactly. No 5G NR rate matching (puncturing/shortening/sub-block interleaving/circular buffer) — out of scope for this module.
- **Channel polarization**: combining two copies of a noisy channel synthesizes one channel that's *better* than the original and one that's *worse*. Recursing this $n$ times over $N=2^n$ channels drives most of the synthetic channels toward either "almost perfect" or "almost useless" — the more extreme the polarization, the more efficiently a code can allocate bits.
- **Frozen bits** are pinned to 0 and placed on the least-reliable synthetic channels; **information bits** go on the most-reliable ones. Reliability ordering uses the fixed 3GPP TS 38.212 Table 5.3.1.2-1 sequence — no per-code construction algorithm needed.
- **Decoding**: plain Successive Cancellation (SC) only. CRC-aided SCL (better performance, more complexity) is future work.

## Parameters

| n_log2 | N | K | rate | frozen positions | info positions |
|---|---|---|---|---|---|
| 3 | 8 | 4 | 0.500 | {0,1,2,4} | {3,5,6,7} |
| 4 | 16 | 8 | 0.500 | {0,1,2,3,4,5,8,9} | {6,7,10,11,12,13,14,15} |
| 5 | 32 | 16 | 0.500 | {0,1,2,3,4,5,6,8,9,10,12,16,17,18,20,24} | {7,11,13,14,15,19,21,22,23,25,26,27,28,29,30,31} |

`n_log2` is the only free construction parameter (alongside `K`): `N = 2^n_log2` follows automatically, and the frozen/information split is derived from the fixed 3GPP sequence rather than computed per-code.

## Frozen-Bit Selection

The 3GPP sequence is a single, fixed permutation of `{0, ..., 1023}`: `polar_3gpp_reliability_sequence[i] = Q_i`, where `Q_i` is the bit-channel index of the `i`-th *least* reliable synthetic channel (ascending reliability — index 0 is least reliable, index 1023 is most reliable). To build an `(N, K)` code:

1. Walk the sequence in order, keeping only entries `Q_i < N` (there are always exactly `N` of them, since the sequence is a permutation of `0..1023` and `N` is a power of two `≤ 1024`).
2. This restricted list is itself already in ascending-reliability order. The first `N - K` (least reliable) are **frozen**; the last `K` (most reliable) are **information** positions.

No per-code construction algorithm (Bhattacharyya bounds, Gaussian approximation, density evolution, ...) is needed — the ordering is a fixed table lookup, exactly like `hamming_init`'s position-as-address scheme needs no stored parity-check matrix.

Source: 3GPP TS 38.212 Table 5.3.1.2-1. The `N=8, K=4` row above is pinned by a unit test.

## Encoding

$G_N = F^{\otimes n}$, where $F = \begin{bmatrix}1&0\\1&1\end{bmatrix}$ — no bit-reversal permutation. Conceptually: split the length-`N` input `u` into two halves; XOR the second half into the first (`v1 = u_left XOR u_right`, `v2 = u_right`); recurse on each half; concatenate the results. This is the same "combine, then recurse" shape as `hamming_encode`'s parity computation, just structured as a binary tree instead of a flat loop:

```
transform(u, len):
    if len == 1: x[0] = u[0]; return
    half = len / 2
    v1[i] = u[i] XOR u[i+half]    for i in 0..half-1
    v2[i] = u[i+half]
    transform(v1, half) -> x[0 .. half-1]
    transform(v2, half) -> x[half .. len-1]
```

`polar_encode` scatters `info_bits` into the `is_info` positions of a length-`N` vector (frozen positions stay 0), then runs this transform. It's a hard-bit GF(2) operation throughout — no soft values needed for encoding.

## SC Decoding

Successive Cancellation decodes leaf bits one at a time, in order, using log-likelihood ratios (LLRs) with the convention `LLR = ln(P(bit=0) / P(bit=1))` — positive means "more likely 0". The decoder mirrors the encoder's binary tree: at each internal node, an `f`-function combines the two child LLR halves for the *left* subtree, and (once the left subtree has produced hard bits) a `g`-function combines them for the *right* subtree:

```c
f(a, b) = sign(a) * sign(b) * min(|a|, |b|)     /* min-sum approximation */
g(a, b, u) = u ? (b - a) : (b + a)               /* u = already-decoded left-subtree bit */
```

At a leaf: frozen positions are forced to 0 regardless of the LLR (the decoder already "knows" the answer); information positions take a hard decision (`llr < 0 → 1, else → 0`). Each node also propagates a *partial sum* (its own re-encoding of the bits it just decoded) back up to its parent, computed cheaply via GF(2) linearity (`transform(a XOR b) = transform(a) XOR transform(b)`) rather than a full re-encode — this is what keeps the whole decode O(N log N).

`f` above is the **min-sum approximation** of the exact combining formula (`2·atanh(tanh(a/2)·tanh(b/2))`), chosen for simplicity: no `tanh`/`atanh`, just comparisons and `fabs`. It's what most practical SC/SCL decoders use, at a small performance cost relative to the exact formula — see Limitations.

## API

```c
polar_code_t polar_init(int n_log2, int K);
void polar_encode(const polar_code_t *pc, const uint8_t *info_bits, uint8_t *codeword);
void polar_decode(const polar_code_t *pc, const double *channel_llr, uint8_t *info_bits);
```

Usage, encoding a message, sending it through an AWGN channel, and decoding:

```c
polar_code_t pc = polar_init(4, 8);           /* N=16, K=8 */
uint8_t info[8] = {1, 0, 1, 1, 0, 0, 1, 0};
uint8_t codeword[16];
polar_encode(&pc, info, codeword);

double llr[16];
uint32_t seed = 42;
channel_awgn(codeword, llr, 16, 0.5, &seed);   /* sigma = noise standard deviation */

uint8_t recovered[8];
polar_decode(&pc, llr, recovered);
```

`channel_awgn` (see [channel.h](../include/ecc/channel.h)) BPSK-modulates each bit and outputs the LLR `2*y/sigma^2` directly — the sufficient statistic for BPSK+AWGN — so no separate LLR-conversion step is needed for this channel. Bridging from `channel_bsc`'s hard-bit output instead would require a one-line conversion (`llr = (1 - 2*bit) * ln((1-p)/p)`), left to the caller rather than added as library API, since it's channel-specific glue rather than a decoder concern.

## Limitations

- **SC decoding only.** No CRC-aided SCL (list decoding) — a real, non-negligible error floor remains at moderate-to-high code rates. A different decoder family (SCL) is needed for stronger guarantees — out of scope for this module.
- **No 5G NR rate matching.** `N` must be an exact power of two — no puncturing, shortening, sub-block interleaving, or circular-buffer rate matching.
- **Min-sum approximation**, not the exact SC combining formula — a small, well-known performance cost in exchange for simplicity (no `tanh`/`atanh`).

## Bit Representation

Hard bits (`info_bits`, `codeword`, the `is_info` mask) stay one `uint8_t` each, consistent with `hamming.c`'s "simplicity over memory" tradeoff (see `docs/Hamming.md`). LLRs are `double` — unlike hard bits, they're inherently continuous/soft values, so bit-packing doesn't apply to them at all.
