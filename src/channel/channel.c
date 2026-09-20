/**
 * @file    channel.c
 * @brief   Implementation of channel models (BSC, AWGN) shared across all
 *          coder families.
 *
 * @author  Kemal UZGOREN
 * @date    2026-08-30
 *
 * SPDX-License-Identifier: MIT
 */

#include "ecc/channel.h"
#include <assert.h>
#include <math.h>

#define CHANNEL_PI 3.14159265358979323846

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

void channel_awgn(const uint8_t *in, double *llr_out, size_t n, double sigma, uint32_t *seed)
{
    assert(in && llr_out && seed && *seed != 0);
    assert(sigma > 0.0);

    for (size_t i = 0; i < n; i++) {
        double bpsk = in[i] ? -1.0 : 1.0;

        /* Box-Muller: only the cosine sample is used (the paired sine sample
         * is discarded) to keep this stateless across calls - simpler than
         * caching a spare value, at the cost of half the entropy per draw. */
        double u1 = next_uniform(seed);
        if (u1 <= 0.0) {
            u1 = 1e-12; /* avoid log(0); xorshift32 can (rarely) yield exactly 0 */
        }
        double u2 = next_uniform(seed);
        double noise = sigma * sqrt(-2.0 * log(u1)) * cos(2.0 * CHANNEL_PI * u2);

        double y = bpsk + noise;
        llr_out[i] = 2.0 * y / (sigma * sigma);
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