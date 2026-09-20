/**
 * @file    test_hamming.c
 * @brief   Unit tests: dimensions, encode/decode roundtrip, single-bit correction.
 *
 * @author  Kemal UZGOREN
 * @date    2026-08-30
 *
 * SPDX-License-Identifier: MIT
 */

#include "ecc/hamming.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>

#define MAX_N 64  /* covers r up to 6 (n=63) */


static void test_dimensions(void);
static void fill_pattern(uint8_t *data, int k);
static void test_encode_decode_roundtrip(int r);
static void test_single_bit_correction(int r);


int main(void)
{
    test_dimensions();

    for (int r = 3; r <= 6; r++) {
        test_encode_decode_roundtrip(r);
        test_single_bit_correction(r);
    }

    printf("All hamming tests passed.\n");
    return 0;
}


static void test_dimensions(void)
{
    hamming_code_t hc;

    hc = hamming_init(3);
    assert(hc.n == 7 && hc.k == 4);

    hc = hamming_init(4);
    assert(hc.n == 15 && hc.k == 11);

    hc = hamming_init(5);
    assert(hc.n == 31 && hc.k == 26);
}

static void fill_pattern(uint8_t *data, int k)
{
    for (int i = 0; i < k; i++) {
        data[i] = (uint8_t)(i % 2);
    }
}

static void test_encode_decode_roundtrip(int r)
{
    hamming_code_t hc = hamming_init(r);
    uint8_t data[MAX_N], recovered[MAX_N], codeword[MAX_N];

    fill_pattern(data, hc.k);
    hamming_encode(&hc, data, codeword);

    int syndrome = hamming_decode(&hc, codeword); /* no error injected */
    assert(syndrome == 0);

    hamming_extract_data(&hc, codeword, recovered);
    assert(memcmp(data, recovered, hc.k) == 0);
}

static void test_single_bit_correction(int r)
{
    hamming_code_t hc = hamming_init(r);
    uint8_t data[MAX_N], original[MAX_N], corrupted[MAX_N];

    fill_pattern(data, hc.k);
    hamming_encode(&hc, data, original);

    /* flip every position, one at a time, and verify exact recovery */
    for (int pos = 1; pos <= hc.n; pos++) {
        memcpy(corrupted, original, hc.n);
        corrupted[pos - 1] ^= 1;

        int syndrome = hamming_decode(&hc, corrupted);
        assert(syndrome == pos);
        assert(memcmp(corrupted, original, hc.n) == 0);
    }
}