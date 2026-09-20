/**
 * @file    test_channel_bsc_sim.c
 * @brief   Integration test: BSC channel combined with Hamming encode/decode,
 *          verifying pre- vs post-correction bit error rate.
 *
 * @author  Kemal UZGOREN
 * @date    2026-08-30
 *
 * SPDX-License-Identifier: MIT
 */

#include "test_channel_bsc_sim.h"
#include "ecc/channel.h"
#include "ecc/hamming.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#define N_BITS 100000

void test_bsc_zero_probability(void)
{
    uint8_t in[16], out[16];
    for (int i = 0; i < 16; i++) {
        in[i] = (uint8_t)(i % 2);
    }

    uint32_t seed = 42;
    channel_bsc(in, out, 16, 0.0, &seed);

    assert(memcmp(in, out, 16) == 0);
}

void test_bsc_deterministic_seed(void)
{
    uint8_t in[32], out_a[32], out_b[32];
    for (int i = 0; i < 32; i++) {
        in[i] = (uint8_t)(i % 2);
    }

    uint32_t seed_a = 12345;
    uint32_t seed_b = 12345;
    channel_bsc(in, out_a, 32, 0.3, &seed_a);
    channel_bsc(in, out_b, 32, 0.3, &seed_b);

    assert(memcmp(out_a, out_b, 32) == 0);
}

void test_bsc_empirical_flip_rate(void)
{
    static uint8_t in[N_BITS], out[N_BITS];
    memset(in, 0, N_BITS); /* fixed input; any 1 in output is a flip */

    uint32_t seed = 777;
    double p = 0.1;
    channel_bsc(in, out, N_BITS, p, &seed);

    int flips = 0;
    for (int i = 0; i < N_BITS; i++) {
        if (out[i] != in[i]) flips++;
    }

    double empirical = (double)flips / N_BITS;
    assert(fabs(empirical - p) < 0.01);
}

void test_bsc_hamming_ber_improvement(void)
{
    hamming_code_t hc = hamming_init(3); /* (7,4) */
    const int trials = 10000;
    const double p = 0.05;

    uint32_t seed = 2026;
    long pre_errors = 0;
    long post_errors = 0;

    for (int t = 0; t < trials; t++) {
        uint8_t data[4], codeword[7], received[7];

        for (int i = 0; i < 4; i++) {
            data[i] = (uint8_t)((t >> i) & 1); /* cycles through all 16 patterns */
        }

        hamming_encode(&hc, data, codeword);
        channel_bsc(codeword, received, 7, p, &seed);

        for (int i = 0; i < 7; i++) {
            if (received[i] != codeword[i]) pre_errors++;
        }

        hamming_decode(&hc, received); /* corrects in place if single-bit error */

        for (int i = 0; i < 7; i++) {
            if (received[i] != codeword[i]) post_errors++;
        }
    }

    double pre_ber = (double)pre_errors / (trials * 7);
    double post_ber = (double)post_errors / (trials * 7);

    printf("pre-correction BER:  %.5f (raw p=%.2f)\n", pre_ber, p);
    printf("post-correction BER: %.5f\n", post_ber);

    assert(fabs(pre_ber - p) < 0.01);
    assert(post_ber < pre_ber);
    assert(post_ber < p * 0.5);
}

int main(void)
{
    test_bsc_zero_probability();
    test_bsc_deterministic_seed();
    test_bsc_empirical_flip_rate();
    test_bsc_hamming_ber_improvement();

    printf("All channel BSC simulation tests passed.\n");
    return 0;
}