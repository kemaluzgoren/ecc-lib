/**
 * @file    test_channel_awgn_sim.c
 * @brief   Integration test: AWGN channel model correctness (determinism,
 *          low-noise sign sanity, empirical BER vs. the BPSK+AWGN theoretical
 *          bit error rate).
 *
 * @author  Kemal UZGOREN
 * @date    2026-09-14
 *
 * SPDX-License-Identifier: MIT
 */

#include "test_channel_awgn_sim.h"
#include "ecc/channel.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#define N_BITS 100000

void test_awgn_deterministic_seed(void)
{
    uint8_t in[32];
    for (int i = 0; i < 32; i++) {
        in[i] = (uint8_t)(i % 2);
    }

    double llr_a[32], llr_b[32];
    uint32_t seed_a = 12345;
    uint32_t seed_b = 12345;
    channel_awgn(in, llr_a, 32, 0.5, &seed_a);
    channel_awgn(in, llr_b, 32, 0.5, &seed_b);

    for (int i = 0; i < 32; i++) {
        assert(llr_a[i] == llr_b[i]);
    }
}

void test_awgn_llr_sign_low_noise(void)
{
    static uint8_t in[N_BITS];
    static double llr[N_BITS];
    for (int i = 0; i < N_BITS; i++) {
        in[i] = (uint8_t)(i % 2);
    }

    uint32_t seed = 99;
    double sigma = 0.1; /* tiny noise: a sign flip would need a ~10-sigma event */
    channel_awgn(in, llr, N_BITS, sigma, &seed);

    for (int i = 0; i < N_BITS; i++) {
        int hard = (llr[i] < 0.0) ? 1 : 0;
        assert(hard == in[i]);
    }
}

void test_awgn_empirical_ber_matches_theory(void)
{
    static uint8_t in[N_BITS];
    static double llr[N_BITS];
    memset(in, 0, N_BITS); /* fixed bit=0 (bpsk=+1); any negative LLR is a hard error */

    uint32_t seed = 2027;
    double sigma = 0.8;
    channel_awgn(in, llr, N_BITS, sigma, &seed);

    int errors = 0;
    for (int i = 0; i < N_BITS; i++) {
        if (llr[i] < 0.0) errors++;
    }
    double empirical_ber = (double)errors / N_BITS;

    /* theoretical BPSK+AWGN bit error rate: Q(1/sigma) = 0.5*erfc(1/(sigma*sqrt(2))) */
    double theoretical_ber = 0.5 * erfc(1.0 / (sigma * sqrt(2.0)));

    printf("AWGN empirical BER: %.5f (theoretical: %.5f, sigma=%.2f)\n",
           empirical_ber, theoretical_ber, sigma);
    assert(fabs(empirical_ber - theoretical_ber) < 0.01);
}

int main(void)
{
    test_awgn_deterministic_seed();
    test_awgn_llr_sign_low_noise();
    test_awgn_empirical_ber_matches_theory();

    printf("All channel AWGN simulation tests passed.\n");
    return 0;
}
