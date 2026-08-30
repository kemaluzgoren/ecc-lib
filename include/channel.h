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

#endif /* ECC_CHANNEL_H */