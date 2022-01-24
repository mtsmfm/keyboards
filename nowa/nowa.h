// Copyright 2022 mtsmfm (@mtsmfm)
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "quantum.h"

/* This is a shortcut to help you visually see your layout.
 *
 * The first section contains all of the arguments representing the physical
 * layout of the board and position of the keys.
 *
 * The second converts the arguments into a two-dimensional array which
 * represents the switch matrix.
 */
// clang-format off
#define LAYOUT(                                 \
    L05, L04, L03, L02, L01, L00,              R00, R01, R02, R03, R04, R05, \
    L15, L14, L13, L12, L11, L10,              R10, R11, R12, R13, R14, R15, \
    L25, L24, L23, L22, L21, L20,              R20, R21, R22, R23, R24, R25, \
    L35, L34, L33, L32, L31, L30,              R30, R31, R32, R33, R34, R35, \
         L41, L40,                                            R40, R41,      \
                        L51, L50, L60,    R60, R50, R51,                     \
                        L53, L52, L61,    R61, R52, R53,                     \
                        L55, L54,              R54, R55                      \
    )                                           \
    {                                           \
        {L00,   L01,   L02,   L03,   L04,   L05},   \
        {L10,   L11,   L12,   L13,   L14,   L15},   \
        {L20,   L21,   L22,   L23,   L24,   L25},   \
        {L30,   L31,   L32,   L33,   L34,   L35},   \
        {KC_NO, KC_NO, KC_NO, L40,   L41,   KC_NO}, \
        {L50,   L51,   L52,   L53,   L54,   L55},   \
        {L60,   L61,   KC_NO, KC_NO, KC_NO, KC_NO}, \
        {R00,   R01,   R02,   R03,   R04,   R05},   \
        {R10,   R11,   R12,   R13,   R14,   R15},   \
        {R20,   R21,   R22,   R23,   R24,   R25},   \
        {R30,   R31,   R32,   R33,   R34,   R35},   \
        {KC_NO, KC_NO, KC_NO, R40,   R41,   KC_NO}, \
        {R50,   R51,   R52,   R53,   R54,   R55},   \
        {R60,   R61,   KC_NO, KC_NO, KC_NO, KC_NO}  \
    }
// clang-format on
