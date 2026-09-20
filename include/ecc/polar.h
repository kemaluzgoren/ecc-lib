/**
 * @file    polar.h
 * @brief   Polar (Arikan) encoder and successive-cancellation (SC) decoder.
 *          Frozen-bit selection uses the fixed 3GPP TS 38.212 Table 5.3.1.2-1
 *          reliability sequence. Core transform only: N = 2^n exactly, no
 *          5G NR rate matching (puncturing/shortening/interleaving/circular
 *          buffer). SC decoding only - CRC-aided SCL is future work.
 *
 * @author  Kemal UZGOREN
 * @date    2026-09-14
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef ECC_POLAR_H
#define ECC_POLAR_H

#include <stdint.h>

/* Nmax fixed by the 3GPP reliability sequence (TS 38.212 Table 5.3.1.2-1);
 * the mother code length N = 2^n_log2 must not exceed this. */
#define POLAR_MAX_LOG2N 10
#define POLAR_MAX_N     (1 << POLAR_MAX_LOG2N)   /* 1024 */

typedef struct {
    int n_log2;                     /* N = 2^n_log2, 1 <= n_log2 <= POLAR_MAX_LOG2N */
    int N;                          /* mother code / block length */
    int K;                          /* number of information bits, 1 <= K <= N */
    uint8_t is_info[POLAR_MAX_N];   /* is_info[i]==1: position i carries information;
                                        ==0: frozen to 0. Only indices [0,N) are meaningful. */
} polar_code_t;

/* n_log2 selects N=2^n_log2; K is the number of information bits. The frozen/
 * information split follows the 3GPP TS 38.212 fixed reliability sequence,
 * restricted to positions < N, with the K most reliable of those chosen as
 * information positions (the rest are frozen to 0). */
polar_code_t polar_init(int n_log2, int K);

/* info_bits: pc->K elements (0/1). codeword: pc->N elements (0/1, output).
 * Hard-bit GF(2) linear transform (G_N = F^Kronecker-n), no bit-reversal
 * permutation - encoding never needs soft values. */
void polar_encode(const polar_code_t *pc, const uint8_t *info_bits, uint8_t *codeword);

/* channel_llr: pc->N log-likelihood ratios, one per received codeword bit, using
 * the convention LLR = ln( P(bit=0) / P(bit=1) ) (positive => bit more likely 0).
 * info_bits: pc->K elements (0/1, output) - the decoded information bits.
 * Plain (non-list) successive-cancellation decoding, min-sum LLR-domain recursion. */
void polar_decode(const polar_code_t *pc, const double *channel_llr, uint8_t *info_bits);

#endif /* ECC_POLAR_H */
