# LDPC Code — Design Notes (Not Implemented)

**Status: research notes only.** No `ldpc.c`/`ldpc.h` exists in this repo. This document preserves the conceptual groundwork and source-material scoping done while evaluating LDPC for this project, in case work resumes later.

## Quick Recap

- Low-density parity-check code: a genuinely stored, sparse **parity-check matrix H** (unlike Hamming, where `H` is never materialized — position doubles as its own address).
- "Low density" = almost all of `H` is 0; each row (check) touches only a few columns (variable/code bits). This sparsity is what makes both encoding and iterative decoding practical.
- `H` can be visualized as a **Tanner graph**: a bipartite graph with variable nodes (code bits) on one side, check nodes (parity equations) on the other, edges = the 1s in `H`.
- Decoding is **iterative message passing** (min-sum / belief propagation) over this graph — structurally very different from Polar's single-pass recursive SC decoder.

## 5G NR Structure: Base Graph + Lifting (Quasi-Cyclic LDPC)

3GPP TS 38.212 §5.3.2 doesn't store a full `H` directly. Instead:

1. **Base graph (protograph)** — a small prototype matrix: **BG1** (46 rows × 68 cols) or **BG2** (42 rows × 52 cols). Each cell is either empty or a **shift value** `V(i,j)`.
2. **Lifting** — the real `H` is built by expanding every base-graph cell into a `Zc × Zc` block: empty → all-zero block; shift value `V(i,j)` → the identity matrix circularly shifted right by `V(i,j) mod Zc`.

`Zc` (lifting size) scales the small prototype up to the actual block length. `Zc` values are grouped into **8 lifting-size sets** (`iLS` 0–7, TS 38.212 Table 5.3.2-1), each with its own shift-value table — a given `V(i,j)` depends on which `iLS` set the chosen `Zc` belongs to.

Two base graphs exist because BG1 targets higher code rates / larger blocks, BG2 targets lower rates / smaller blocks.

## Encoding

Not detailed here — the reference procedure (TS 38.212 §5.3.2) solves `H·[c;w]=0` for the parity bits `w`, exploiting `H`'s structured (approximately lower-triangular) form rather than building a generator matrix `G`.

## Decoding: Min-Sum (Belief Propagation)

Iterative, not a single recursive pass like Polar SC:
- Variable nodes send LLR messages to their connected check nodes.
- Check nodes combine incoming messages (min-sum approximation, conceptually similar to Polar's `f`-function) and send updates back.
- Repeats for a fixed number of iterations or until all parity checks are satisfied.

## Comparison with What's Actually in This Repo

| | Hamming | Polar | LDPC (not built) |
|---|---|---|---|
| Structure | Position = address, no matrix | Recursive tree (`F^⊗n`) | Sparse matrix / Tanner graph |
| Decode | One syndrome computation | Single-pass recursive SC | Iterative message passing (min-sum) |
| Complexity driver | None (formulaic) | Tree depth (`log N`) | Graph size × iteration count |
