/**
 * @file    test_hamming.c
 * @brief   Channel models shared across all coder families.
 *
 * @author  Kemal UZGOREN
 * @date    2026-08-30
 *
 * SPDX-License-Identifier: MIT
 */

#include "channel.h"
#include <assert.h>

/* xorshift32 - no libc RNG dependency, deterministic given a seed. */
static uint32_t xorshift32(uint32_t *state);
static double next_uniform(uint32_t *seed);

void channel_bsc(const uint8_t *in, uint8_t *out, size_t n, double p, uint32_t *seed)
{
    assert(in && out && seed && *seed != 0);
    assert(p >= 0.0 && p <= 0.5);

    for (size_t i = 0; i < n; i++) {
        uint8_t bit = in[i];
        if (next_uniform(seed) < p) {
            bit ^= 1;
        }
        out[i] = bit;
    }
}


static uint32_t xorshift32(uint32_t *state)
{
    uint32_t x = *state;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    *state = x;
    return x;
}

static double next_uniform(uint32_t *seed)
{
    return (double)xorshift32(seed) / 4294967296.0; /* 2^32 */
}