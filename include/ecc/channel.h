/**
 * @file    channel.h
 * @brief   Channel models shared across all coder families.
 *
 * @author  Kemal UZGOREN
 * @date    2026-08-30
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef ECC_CHANNEL_H
#define ECC_CHANNEL_H

#include <stdint.h>
#include <stddef.h>

/* Flips each bit in `in` independently with probability p (0.0-0.5).
 * `out` may alias `in`. `seed` must be non-zero and is updated in place,
 * so repeated calls continue the same pseudo-random sequence. */
void channel_bsc(const uint8_t *in, uint8_t *out, size_t n, double p, uint32_t *seed);

/* BPSK-modulates each bit (0 -> +1.0, 1 -> -1.0), adds zero-mean Gaussian
 * noise with standard deviation `sigma`, and writes the LLR
 * ln(P(bit=0|y)/P(bit=1|y)) = 2*y/sigma^2 directly to `llr_out` - the
 * sufficient statistic for BPSK+AWGN, and the natural "received"
 * representation for a channel with continuous (not discrete) output.
 * `seed` must be non-zero and is updated in place, like channel_bsc. */
void channel_awgn(const uint8_t *in, double *llr_out, size_t n, double sigma, uint32_t *seed);

#endif /* ECC_CHANNEL_H */