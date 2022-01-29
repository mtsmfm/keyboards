// Copyright 2022 mtsmfm (@mtsmfm)
// SPDX-License-Identifier: GPL-2.0-or-later

#include QMK_KEYBOARD_H
#include "analog.h"
#ifdef CONSOLE_ENABLE
  #include "print.h"
#endif
#include "transactions.h"

#ifndef MAX
#define MAX(X, Y) ((X) > (Y) ? (X) : (Y))
#endif
#ifndef MIN
#define MIN(X, Y) ((X) < (Y) ? (X) : (Y))
#endif

// Defines names for use in layer keycodes and the keymap
enum layer_names
{
    _BASE,
    _FN1,
};

// clang-format off
const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [_BASE] = LAYOUT(
        KC_EQL , KC_1   , KC_2   , KC_3   , KC_4   , KC_5   ,                           KC_6   , KC_7   , KC_8   , KC_9   , KC_0   , KC_MINS,
        KC_TAB , KC_Q   , KC_W   , KC_E   , KC_R   , KC_T   ,                           KC_Y   , KC_U   , KC_I   , KC_O   , KC_P   , KC_BSLS,
        KC_ESC , KC_A   , KC_S   , KC_D   , KC_F   , KC_G   ,                           KC_H   , KC_J   , KC_K   , KC_L   , KC_SCLN, KC_QUOT,
        KC_LSFT, KC_Z   , KC_X   , KC_C   , KC_V   , KC_B   ,                           KC_N   , KC_M   , KC_COMM, KC_DOT , KC_SLSH, KC_RSFT,
                 KC_GRV , KC_BSLS,                                                                                 KC_LBRC, KC_RBRC,
                                            KC_LCTL, KC_LWIN, MO(_FN1) ,     MO(_FN1) , KC_RWIN, KC_RCTL,
                                            KC_DEL , XXXXXXX, DF(_FN1) ,     DF(_FN1) , XXXXXXX, KC_ENT ,
                                            KC_BSPC, KC_RALT,                           KC_LALT, KC_SPC
    ),
    [_FN1] = LAYOUT(
        _______, _______, _______, _______, _______, _______,                           _______, _______, _______, _______, _______, _______,
        _______, _______, KC_UP  , _______, KC_WH_U, _______,                           _______, KC_WH_U, _______, KC_UP  , _______, _______,
        _______, KC_LEFT, KC_DOWN, KC_RGHT, KC_BTN1, KC_BTN2,                           KC_BTN2, KC_BTN1, KC_LEFT, KC_DOWN, KC_RGHT, _______,
        _______, _______, _______, _______, KC_WH_D, _______,                           _______, KC_WH_D, _______, _______, _______, _______,
                 _______, _______,                                                                                 _______, _______,
                                            _______, _______, _______  ,     _______  , _______, _______,
                                            _______, _______, DF(_BASE),     DF(_BASE), _______, _______,
                                            _______, _______,                           _______, _______
    ),
};
// clang-format on

typedef struct {
    int16_t horz_val;
    int16_t vert_val;
} analog_stick_data_t;

typedef struct {
    analog_stick_data_t min;
    analog_stick_data_t max;
} analog_stick_threashold_t;

analog_stick_threashold_t primary_analog_stick_threshold = {
    .min = {.horz_val = 1023, .vert_val = 1023},
    .max = {.horz_val = 0, .vert_val = 0}
};
analog_stick_threashold_t secondary_analog_stick_threshold = {
    .min = {.horz_val = 1023, .vert_val = 1023},
    .max = {.horz_val = 0, .vert_val = 0}
};
analog_stick_data_t secondary_analog_stick;

int8_t pin_val_to_int8(int16_t v) {
    return (int8_t)(v / 4) - 128;
}

static void get_analog_stick_data_secondary_handler(uint8_t in_buflen, const void *in_data, uint8_t out_buflen, void *out_data) {
    analog_stick_data_t *data = (analog_stick_data_t *)out_data;
    *data = secondary_analog_stick;
}

void keyboard_post_init_kb(void) {
    setPinInputHigh(STICK_PUSH_PIN);

    if (is_keyboard_master()) {
        for (uint8_t i = 0; i < 10; i++) {
            int16_t horz_val = analogReadPin(HORZ_PIN);
            int16_t vert_val = analogReadPin(VERT_PIN);

            primary_analog_stick_threshold.min.horz_val = MIN(primary_analog_stick_threshold.min.horz_val, horz_val - STICK_THRESHOLD);
            primary_analog_stick_threshold.min.vert_val = MIN(primary_analog_stick_threshold.min.vert_val, vert_val - STICK_THRESHOLD);

            primary_analog_stick_threshold.max.horz_val = MAX(primary_analog_stick_threshold.max.horz_val, horz_val + STICK_THRESHOLD);
            primary_analog_stick_threshold.max.vert_val = MAX(primary_analog_stick_threshold.max.vert_val, vert_val + STICK_THRESHOLD);

            analog_stick_data_t data;
            if (transaction_rpc_recv(GET_ANALOG_STICK_DATA, sizeof(data), &data)) {
                secondary_analog_stick_threshold.min.horz_val = MIN(secondary_analog_stick_threshold.min.horz_val, data.horz_val - STICK_THRESHOLD);
                secondary_analog_stick_threshold.min.vert_val = MIN(secondary_analog_stick_threshold.min.vert_val, data.vert_val - STICK_THRESHOLD);

                secondary_analog_stick_threshold.max.horz_val = MAX(secondary_analog_stick_threshold.max.horz_val, data.horz_val + STICK_THRESHOLD);
                secondary_analog_stick_threshold.max.vert_val = MAX(secondary_analog_stick_threshold.max.vert_val, data.vert_val + STICK_THRESHOLD);

                secondary_analog_stick.horz_val = data.horz_val;
                secondary_analog_stick.vert_val = data.vert_val;
            }

            wait_ms(10);
        }
    } else {
        transaction_register_rpc(GET_ANALOG_STICK_DATA, get_analog_stick_data_secondary_handler);
    }
}

void pointing_device_task(void) {
    // from 0 to 1023
    int16_t horz_val = analogReadPin(HORZ_PIN);
    int16_t vert_val = analogReadPin(VERT_PIN);

    report_mouse_t report = pointing_device_get_report();

    // from -127 to 127
    if (horz_val < primary_analog_stick_threshold.min.horz_val || primary_analog_stick_threshold.max.horz_val < horz_val) {
        report.x += pin_val_to_int8(horz_val) / 6;
    }
    if (vert_val < primary_analog_stick_threshold.min.vert_val || primary_analog_stick_threshold.max.vert_val < vert_val) {
        report.y += pin_val_to_int8(vert_val) / 6;
    }
    if (secondary_analog_stick.horz_val < secondary_analog_stick_threshold.min.horz_val || secondary_analog_stick_threshold.max.horz_val < secondary_analog_stick.horz_val) {
        report.x += pin_val_to_int8(secondary_analog_stick.horz_val) / 6;
    }
    if (secondary_analog_stick.vert_val < secondary_analog_stick_threshold.min.vert_val || secondary_analog_stick_threshold.max.vert_val < secondary_analog_stick.vert_val) {
        report.y += pin_val_to_int8(secondary_analog_stick.vert_val) / 6;
    }

    pointing_device_set_report(report);
    pointing_device_send();
}

void matrix_slave_scan_kb(void) {
    secondary_analog_stick.horz_val = analogReadPin(HORZ_PIN);
    secondary_analog_stick.vert_val = analogReadPin(VERT_PIN);

    matrix_slave_scan_user();
}

void housekeeping_task_kb(void) {
    if (is_keyboard_master()) {
        analog_stick_data_t data;
        if (transaction_rpc_recv(GET_ANALOG_STICK_DATA, sizeof(data), &data)) {
            secondary_analog_stick = data;
        }
    }
}
