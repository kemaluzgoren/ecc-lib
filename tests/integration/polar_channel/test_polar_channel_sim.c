/**
 * @file    test_polar_channel_sim.c
 * @brief   Integration test: AWGN channel combined with Polar SC encode/decode,
 *          verifying exact recovery at low noise and pre- vs post-decode BER.
 *
 * @author  Kemal UZGOREN
 * @date    2026-09-14
 *
 * SPDX-License-Identifier: MIT
 */

#include "test_polar_channel_sim.h"
#include "ecc/channel.h"
#include "ecc/polar.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

void test_polar_awgn_low_noise_exact_recovery(void)
{
    polar_code_t pc = polar_init(4, 8); /* N=16, K=8 */
    uint8_t msg[8] = {1, 0, 1, 1, 0, 0, 1, 0};
    uint8_t codeword[16], recovered[8];
    double llr[16];

    uint32_t seed = 7;
    double sigma = 0.05; /* very low noise: a decode error here would need a ~20-sigma event */

    for (int t = 0; t < 100; t++) {
        polar_encode(&pc, msg, codeword);
        channel_awgn(codeword, llr, 16, sigma, &seed);
        polar_decode(&pc, llr, recovered);
        assert(memcmp(msg, recovered, 8) == 0);
    }
}

void test_polar_awgn_ber_improvement(void)
{
    polar_code_t pc = polar_init(6, 16); /* N=64, K=16, rate 1/4 */
    const int trials = 3000;
    const double sigma = 0.7;

    uint8_t msg[16];
    for (int i = 0; i < 16; i++) {
        msg[i] = (uint8_t)((i % 2) ^ ((i / 3) % 2));
    }

    uint32_t seed = 2026;
    long pre_errors = 0;
    long post_errors = 0;

    for (int t = 0; t < trials; t++) {
        uint8_t codeword[64], recovered[16];
        double llr[64];

        polar_encode(&pc, msg, codeword);
        channel_awgn(codeword, llr, 64, sigma, &seed);

        for (int i = 0; i < 64; i++) {
            int hard = (llr[i] < 0.0) ? 1 : 0;
            if (hard != codeword[i]) pre_errors++;
        }

        polar_decode(&pc, llr, recovered);
        for (int i = 0; i < 16; i++) {
            if (recovered[i] != msg[i]) post_errors++;
        }
    }

    double pre_ber = (double)pre_errors / (trials * 64);
    double post_ber = (double)post_errors / (trials * 16);

    printf("polar pre-decode BER:  %.5f (sigma=%.2f)\n", pre_ber, sigma);
    printf("polar post-decode BER: %.5f\n", post_ber);

    /* sanity band around the theoretical BPSK+AWGN BER for sigma=0.7 (~0.076) */
    assert(pre_ber > 0.03 && pre_ber < 0.13);
    assert(post_ber < pre_ber);
    assert(post_ber < pre_ber * 0.3);
}

int main(void)
{
    test_polar_awgn_low_noise_exact_recovery();
    test_polar_awgn_ber_improvement();

    printf("All polar+AWGN channel simulation tests passed.\n");
    return 0;
}
