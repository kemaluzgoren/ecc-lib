/**
 * @file    test_polar.c
 * @brief   Unit tests: 3GPP reliability table integrity, frozen-set selection,
 *          encode against externally-verified golden vectors, noiseless
 *          encode/decode roundtrip, and SC decoder robustness to a corrupted
 *          channel LLR.
 *
 * @author  Kemal UZGOREN
 * @date    2026-09-14
 *
 * SPDX-License-Identifier: MIT
 */

#include "ecc/polar.h"
#include "polar_reliability_table.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>

#define LARGE_LLR 40.0


static void test_reliability_table_is_permutation(void);
static void test_frozen_set_known_example(void);
static void test_encode_matches_golden_vector_n8k4(void);
static void test_encode_matches_golden_vector_n16k8(void);
static void fill_msg_pattern(uint8_t *msg, int K);
static void test_encode_decode_roundtrip_noiseless(int n_log2, int K);
static void test_sc_tolerates_weakened_llr(void);


int main(void)
{
    test_reliability_table_is_permutation();
    test_frozen_set_known_example();
    test_encode_matches_golden_vector_n8k4();
    test_encode_matches_golden_vector_n16k8();

    test_encode_decode_roundtrip_noiseless(3, 4);   /* N=8,  K=4  */
    test_encode_decode_roundtrip_noiseless(4, 8);   /* N=16, K=8  */
    test_encode_decode_roundtrip_noiseless(5, 16);  /* N=32, K=16 */
    test_encode_decode_roundtrip_noiseless(6, 32);  /* N=64, K=32 */

    test_sc_tolerates_weakened_llr();

    printf("All polar tests passed.\n");
    return 0;
}


static void test_reliability_table_is_permutation(void)
{
    uint8_t seen[POLAR_MAX_N];
    memset(seen, 0, sizeof(seen));

    for (int i = 0; i < POLAR_MAX_N; i++) {
        uint16_t v = polar_3gpp_reliability_sequence[i];
        assert(v < POLAR_MAX_N);
        assert(seen[v] == 0);  /* each value appears exactly once */
        seen[v] = 1;
    }
}

/* 3GPP TS 38.212, N=8 K=4: frozen={0,1,2,4}, info={3,5,6,7}. */
static void test_frozen_set_known_example(void)
{
    polar_code_t pc = polar_init(3, 4); /* N=8 */

    uint8_t expected_is_info[8] = {0, 0, 0, 1, 0, 1, 1, 1};
    assert(memcmp(pc.is_info, expected_is_info, 8) == 0);
}

/* Cross-checked against py-polar-codes (github.com/mcba1n/polar-codes),
 * using this same 3GPP frozen set, encoding msg=[0,1,0,0]. */
static void test_encode_matches_golden_vector_n8k4(void)
{
    polar_code_t pc = polar_init(3, 4); /* N=8, K=4 */
    uint8_t msg[4] = {0, 1, 0, 0};
    uint8_t codeword[8];
    uint8_t expected[8] = {1, 1, 0, 0, 1, 1, 0, 0};

    polar_encode(&pc, msg, codeword);
    assert(memcmp(codeword, expected, 8) == 0);
}

/* Cross-checked against py-polar-codes, encoding msg=[0,1,0,0,1,0,0,1]. */
static void test_encode_matches_golden_vector_n16k8(void)
{
    polar_code_t pc = polar_init(4, 8); /* N=16, K=8 */
    uint8_t msg[8] = {0, 1, 0, 0, 1, 0, 0, 1};
    uint8_t codeword[16];
    uint8_t expected[16] = {1, 0, 0, 0, 1, 0, 0, 0, 0, 1, 1, 1, 0, 1, 1, 1};

    polar_encode(&pc, msg, codeword);
    assert(memcmp(codeword, expected, 16) == 0);
}

static void fill_msg_pattern(uint8_t *msg, int K)
{
    for (int i = 0; i < K; i++) {
        msg[i] = (uint8_t)((i % 2) ^ ((i / 3) % 2));
    }
}

static void test_encode_decode_roundtrip_noiseless(int n_log2, int K)
{
    polar_code_t pc = polar_init(n_log2, K);
    uint8_t msg[POLAR_MAX_N / 2], recovered[POLAR_MAX_N / 2], codeword[POLAR_MAX_N];
    double llr[POLAR_MAX_N];

    fill_msg_pattern(msg, K);
    polar_encode(&pc, msg, codeword);

    for (int i = 0; i < pc.N; i++) {
        llr[i] = codeword[i] ? -LARGE_LLR : LARGE_LLR;
    }

    polar_decode(&pc, llr, recovered);
    assert(memcmp(msg, recovered, (size_t)K) == 0);
}

/* N=16, K=4 (rate 1/4): fully flipping a single channel LLR's sign (the
 * worst case short of erasure) is still corrected at every position for
 * this low-rate code - verified empirically against py-polar-codes across
 * all 16 positions before picking position 0 here. Unlike Hamming's
 * structural single-error guarantee, this is a property of this specific
 * (N,K), not a general SC decoder guarantee - don't assume it holds at
 * higher rates. */
static void test_sc_tolerates_weakened_llr(void)
{
    polar_code_t pc = polar_init(4, 4); /* N=16, K=4 */
    uint8_t msg[4] = {1, 0, 1, 1};
    uint8_t codeword[16], recovered[4];
    double llr[16];

    polar_encode(&pc, msg, codeword);
    for (int i = 0; i < 16; i++) {
        llr[i] = codeword[i] ? -LARGE_LLR : LARGE_LLR;
    }
    llr[0] = -llr[0]; /* corrupt one channel's LLR sign */

    polar_decode(&pc, llr, recovered);
    assert(memcmp(msg, recovered, 4) == 0);
}
