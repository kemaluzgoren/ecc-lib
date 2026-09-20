/**
 * @file    polar.c
 * @brief   Implementation of the Polar encoder and SC decoder.
 *
 * @author  Kemal UZGOREN
 * @date    2026-09-14
 *
 * SPDX-License-Identifier: MIT
 */

#include "ecc/polar.h"
#include "polar_reliability_table.h"
#include <assert.h>
#include <math.h>
#include <string.h>


/* G_N = F^(x)n applied to `u` (length `len`, a power of 2), written to `x`.
 * Combines the two halves (v1 = left XOR right, v2 = right) and recurses -
 * no bit-reversal permutation. Fixed POLAR_MAX_N/2-sized scratch buffers
 * (rather than len-sized VLAs) keep this portable to MSVC, which has no
 * C11 VLA support; see docs/Polar.md Limitations for the stack-size cost. */
static void polar_transform(const uint8_t *u, int len, uint8_t *x);

/* min-sum LLR combining functions for the SC decoder tree. */
static double polar_llr_f(double a, double b);
static double polar_llr_g(double a, double b, uint8_t u);

/* Recursive successive-cancellation decode over `llr` (length `len`), with
 * per-position frozen/info flags in `is_info` (same length). Writes the
 * decoded u-vector to `u_out` and its partial re-encoding (needed by the
 * parent call's g-function) to `beta_out`. Kept module-internal, but the
 * signature is generic enough that a future SCL decoder could reuse it. */
static void sc_recurse(const double *llr, int len, const uint8_t *is_info,
                        uint8_t *u_out, uint8_t *beta_out);


polar_code_t polar_init(int n_log2, int K)
{
    assert(n_log2 >= 1 && n_log2 <= POLAR_MAX_LOG2N);

    polar_code_t pc;
    pc.n_log2 = n_log2;
    pc.N = 1 << n_log2;

    assert(K >= 1 && K <= pc.N);
    pc.K = K;

    memset(pc.is_info, 0, sizeof(pc.is_info));

    /* Walk the 3GPP sequence in ascending-reliability order, keeping only
     * positions < N. The first N-K of those (least reliable) stay frozen;
     * the remaining K (most reliable) are marked as information positions. */
    int seen = 0;
    for (int r = 0; r < POLAR_MAX_N; r++) {
        int idx = polar_3gpp_reliability_sequence[r];
        if (idx < pc.N) {
            seen++;
            if (seen > pc.N - pc.K) {
                pc.is_info[idx] = 1;
            }
        }
    }

    return pc;
}

void polar_encode(const polar_code_t *pc, const uint8_t *info_bits, uint8_t *codeword)
{
    assert(pc && info_bits && codeword);

    uint8_t u[POLAR_MAX_N];
    int idx = 0;
    for (int i = 0; i < pc->N; i++) {
        u[i] = pc->is_info[i] ? info_bits[idx++] : 0;
    }

    polar_transform(u, pc->N, codeword);
}

void polar_decode(const polar_code_t *pc, const double *channel_llr, uint8_t *info_bits)
{
    assert(pc && channel_llr && info_bits);

    uint8_t u[POLAR_MAX_N], beta[POLAR_MAX_N];
    sc_recurse(channel_llr, pc->N, pc->is_info, u, beta);

    int idx = 0;
    for (int i = 0; i < pc->N; i++) {
        if (pc->is_info[i]) {
            info_bits[idx++] = u[i];
        }
    }
}


static void polar_transform(const uint8_t *u, int len, uint8_t *x)
{
    if (len == 1) {
        x[0] = u[0];
        return;
    }

    int half = len / 2;
    uint8_t v1[POLAR_MAX_N / 2], v2[POLAR_MAX_N / 2];
    for (int i = 0; i < half; i++) {
        v1[i] = (uint8_t)(u[i] ^ u[i + half]);
        v2[i] = u[i + half];
    }

    polar_transform(v1, half, x);
    polar_transform(v2, half, x + half);
}

static double polar_llr_f(double a, double b)
{
    double sign = ((a < 0.0) != (b < 0.0)) ? -1.0 : 1.0;
    double mag = (fabs(a) < fabs(b)) ? fabs(a) : fabs(b);
    return sign * mag;
}

static double polar_llr_g(double a, double b, uint8_t u)
{
    return u ? (b - a) : (b + a);
}

static void sc_recurse(const double *llr, int len, const uint8_t *is_info,
                        uint8_t *u_out, uint8_t *beta_out)
{
    if (len == 1) {
        uint8_t bit = is_info[0] ? (uint8_t)(llr[0] < 0.0 ? 1 : 0) : 0;
        u_out[0] = bit;
        beta_out[0] = bit;
        return;
    }

    int half = len / 2;
    double  llr_left[POLAR_MAX_N / 2], llr_right[POLAR_MAX_N / 2];
    uint8_t u_left[POLAR_MAX_N / 2],   u_right[POLAR_MAX_N / 2];
    uint8_t beta_left[POLAR_MAX_N / 2], beta_right[POLAR_MAX_N / 2];

    for (int i = 0; i < half; i++) {
        llr_left[i] = polar_llr_f(llr[i], llr[i + half]);
    }
    sc_recurse(llr_left, half, is_info, u_left, beta_left);

    for (int i = 0; i < half; i++) {
        llr_right[i] = polar_llr_g(llr[i], llr[i + half], beta_left[i]);
    }
    sc_recurse(llr_right, half, is_info + half, u_right, beta_right);

    for (int i = 0; i < half; i++) {
        u_out[i] = u_left[i];
        u_out[i + half] = u_right[i];
        beta_out[i] = (uint8_t)(beta_left[i] ^ beta_right[i]);  /* GF(2) linearity of transform() */
        beta_out[i + half] = beta_right[i];
    }
}
