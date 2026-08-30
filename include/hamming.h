/**
 * @file    hamming.h
 * @brief   Generic Hamming(2^r-1, 2^r-1-r) encoder/decoder.
 *
 * @author  Kemal UZGOREN
 * @date    2026-08-30
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef ECC_HAMMING_H
#define ECC_HAMMING_H

#include <stdint.h>

typedef struct {
    int r;  /* number of parity bits */
    int n;  /* codeword length = 2^r - 1 */
    int k;  /* data length = n - r */
} hamming_code_t;

/* r=3 -> (7,4), r=4 -> (15,11), r=5 -> (31,26), ... */
hamming_code_t hamming_init(int r);

/* data: hc->k elements (0/1), codeword: hc->n elements (0/1, output) */
void hamming_encode(const hamming_code_t *hc, const uint8_t *data, uint8_t *codeword);

/* Corrects codeword in place if a single-bit error is found.
 * Returns 0 if no error, otherwise the corrected position (1-indexed). */
int hamming_decode(const hamming_code_t *hc, uint8_t *codeword);

/* Pulls the hc->k data bits back out of a (corrected) codeword. */
void hamming_extract_data(const hamming_code_t *hc, const uint8_t *codeword, uint8_t *data);

#endif /* ECC_HAMMING_H */