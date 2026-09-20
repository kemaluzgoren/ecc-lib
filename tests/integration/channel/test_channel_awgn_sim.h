/**
 * @file    test_channel_awgn_sim.h
 * @brief   Integration test: AWGN channel model correctness (determinism,
 *          low-noise sign sanity, empirical BER vs. the BPSK+AWGN theoretical
 *          bit error rate).
 *
 * @author  Kemal UZGOREN
 * @date    2026-09-14
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef TEST_CHANNEL_AWGN_SIM_H
#define TEST_CHANNEL_AWGN_SIM_H

void test_awgn_deterministic_seed(void);
void test_awgn_llr_sign_low_noise(void);
void test_awgn_empirical_ber_matches_theory(void);

#endif /* TEST_CHANNEL_AWGN_SIM_H */
