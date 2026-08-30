/**
 * @file    test_channel_bsc_sim.h
 * @brief   Integration test: BSC channel combined with Hamming encode/decode,
 *          verifying pre- vs post-correction bit error rate.
 *
 * @author  Kemal UZGOREN
 * @date    2026-08-30
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef TEST_CHANNEL_BSC_SIM_H
#define TEST_CHANNEL_BSC_SIM_H

void test_bsc_zero_probability(void);
void test_bsc_deterministic_seed(void);
void test_bsc_empirical_flip_rate(void);
void test_bsc_hamming_ber_improvement(void);

#endif /* TEST_CHANNEL_BSC_SIM_H */