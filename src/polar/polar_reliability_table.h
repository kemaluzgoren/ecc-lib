/**
 * @file    polar_reliability_table.h
 * @brief   3GPP TS 38.212 Table 5.3.1.2-1 polar reliability sequence (internal).
 *
 * @author  Kemal UZGOREN
 * @date    2026-09-14
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef ECC_POLAR_RELIABILITY_TABLE_H
#define ECC_POLAR_RELIABILITY_TABLE_H

#include <stdint.h>
#include "ecc/polar.h"

/* polar_3gpp_reliability_sequence[i] = Q_i, the bit-channel index of the i-th
 * least reliable synthetic channel (ascending reliability: index 0 is least
 * reliable, index POLAR_MAX_N-1 is most reliable).
 *
 * Source: 3GPP TS 38.212 Table 5.3.1.2-1, transcribed programmatically from
 * the officially published ETSI TS 138 212 V17.3.0 (2022-09) specification
 * PDF (freely available from etsi.org). Cross-checked element-for-element
 * (1024/1024 exact match) against the independent open-source reference
 * implementation at github.com/vodafone-chair/5g-nr-polar
 * (components/get_3GPP_sequence_pattern.m). */
extern const uint16_t polar_3gpp_reliability_sequence[POLAR_MAX_N];

#endif /* ECC_POLAR_RELIABILITY_TABLE_H */
