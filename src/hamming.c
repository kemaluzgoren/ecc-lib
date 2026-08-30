/**
 * @file    hamming.c
 * @brief   Implementation of generic Hamming code encode/decode.
 *
 * @author  Kemal UZGOREN
 * @date    2026-08-30
 *
 * SPDX-License-Identifier: MIT
 */

#include "hamming.h"
#include <assert.h>


/* Power-of-two positions (1,2,4,8,...) are parity bits; everything else is data. */
static int is_parity_position(int p);


hamming_code_t hamming_init(int r)
{
    assert(r >= 2);

    hamming_code_t hc;
    hc.r = r;
    hc.n = (1 << r) - 1;
    hc.k = hc.n - r;
    return hc;
}

void hamming_encode(const hamming_code_t *hc, const uint8_t *data, uint8_t *codeword)
{
    assert(hc && data && codeword);

    int idx = 0;
    for (int p = 1; p <= hc->n; p++) {
        if (!is_parity_position(p)) {
            codeword[p - 1] = data[idx++];
        }
    }

    for (int j = 0; j < hc->r; j++) {
        int p = 1 << j;
        uint8_t parity = 0;
        for (int q = 1; q <= hc->n; q++) {
            if ((q & p) && q != p) {  /* every position whose j-th bit is set, except p itself */
                parity ^= codeword[q - 1];
            }
        }
        codeword[p - 1] = parity;
    }
}

int hamming_decode(const hamming_code_t *hc, uint8_t *codeword)
{
    assert(hc && codeword);

    int syndrome = 0;
    for (int p = 1; p <= hc->n; p++) {
        if (codeword[p - 1]) {
            syndrome ^= p;  /* XOR of set-bit positions = error position directly */
        }
    }

    if (syndrome != 0) {
        codeword[syndrome - 1] ^= 1;
    }

    return syndrome;
}

void hamming_extract_data(const hamming_code_t *hc, const uint8_t *codeword, uint8_t *data)
{
    assert(hc && codeword && data);

    int idx = 0;
    for (int p = 1; p <= hc->n; p++) {
        if (!is_parity_position(p)) {
            data[idx++] = codeword[p - 1];
        }
    }
}


static int is_parity_position(int p)
{
    return (p & (p - 1)) == 0;
}