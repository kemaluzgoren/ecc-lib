/**
 * @file    test_polar_channel_sim.h
 * @brief   Integration test: AWGN channel combined with Polar SC encode/decode,
 *          verifying exact recovery at low noise and pre- vs post-decode BER.
 *
 * @author  Kemal UZGOREN
 * @date    2026-09-14
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef TEST_POLAR_CHANNEL_SIM_H
#define TEST_POLAR_CHANNEL_SIM_H

void test_polar_awgn_low_noise_exact_recovery(void);
void test_polar_awgn_ber_improvement(void);

#endif /* TEST_POLAR_CHANNEL_SIM_H */
