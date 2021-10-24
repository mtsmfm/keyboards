/* Copyright 2021 mtsmfm
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include QMK_KEYBOARD_H
#include "joystick.h"

// Defines names for use in layer keycodes and the keymap
enum layer_names {
    _BASE,
    _FN
};

// Defines the keycodes used by our macros in process_record_user
enum custom_keycodes {
    JS_LEFT = SAFE_RANGE,
    JS_RIGHT,
    JS_UP,
    JS_DOWN,
};

enum {
    TD_LTFN_JS
};

typedef enum {
    TD_UNKNOWN,
    TD_TAP,
    TD_HOLD
} td_state_t;

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    /* Base */
    [_BASE] = LAYOUT(
                                  TD(TD_LTFN_JS),
        JS_LEFT, JS_DOWN, JS_RIGHT,      JS_BUTTON3, JS_BUTTON0, JS_BUTTON6, JS_BUTTON7,
                          JS_UP,         JS_BUTTON2, JS_BUTTON1, JS_BUTTON4, JS_BUTTON5
    ),
    [_FN] = LAYOUT(
                                  XXXXXXX,
        JS_LEFT, JS_DOWN, JS_RIGHT,      JS_BUTTON0, JS_BUTTON0, JS_BUTTON0, JS_BUTTON0,
                          JS_UP,         JS_BUTTON0, JS_BUTTON0, JS_BUTTON0, JS_BUTTON0
    )
};

joystick_config_t joystick_axes[JOYSTICK_AXES_COUNT] = {
    [0] = JOYSTICK_AXIS_VIRTUAL,
    [1] = JOYSTICK_AXIS_VIRTUAL
};

static uint8_t axesFlags = 0;
enum axes {
    XLow = 1,
    XHigh = 2,
    YLow = 4,
    YHigh = 8,
    Updated = 16
};

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    switch (keycode) {
        case JS_LEFT:
            if (record->event.pressed) {
                axesFlags |= XLow;
            } else {
                axesFlags &= ~XLow;
            }
            axesFlags |= Updated;
            break;
        case JS_RIGHT:
            if (record->event.pressed) {
                axesFlags |= XHigh;
            } else {
                axesFlags &= ~XHigh;
            }
            axesFlags |= Updated;
            break;
        case JS_UP:
            if (record->event.pressed) {
                axesFlags |= YLow;
            } else {
                axesFlags &= ~YLow;
            }
            axesFlags |= Updated;
            break;
        case JS_DOWN:
            if (record->event.pressed) {
                axesFlags |= YHigh;
            } else {
                axesFlags &= ~YHigh;
            }
            axesFlags |= Updated;
            break;
    }
    return true;
}

void joystick_task(void) {
    if (axesFlags & Updated) {
        if (axesFlags & XLow && axesFlags & XHigh) {
            joystick_status.axes[0] = 0;
        } else if (axesFlags & XLow) {
            joystick_status.axes[0] = -127;
        } else if (axesFlags & XHigh) {
            joystick_status.axes[0] = 127;
        } else {
          joystick_status.axes[0] = 0;
        }

        if (axesFlags & YLow && axesFlags & YHigh) {
            joystick_status.axes[1] = -127;
        } else if (axesFlags & YLow) {
            joystick_status.axes[1] = -127;
        } else if (axesFlags & YHigh) {
            joystick_status.axes[1] = 127;
        } else {
          joystick_status.axes[1] = 0;
        }

        send_joystick_packet(&joystick_status);
        axesFlags &= ~Updated;
    }
}

td_state_t cur_dance(qk_tap_dance_state_t *state) {
    if (state->count == 1) {
        if (state->interrupted || !state->pressed) return TD_TAP;
        else return TD_HOLD;
    }
    else return TD_UNKNOWN;
}

static td_state_t td_state;

void ltfn_js_finished(qk_tap_dance_state_t *state, void *user_data) {
    td_state = cur_dance(state);
    switch (td_state) {
        case TD_TAP:
            process_joystick(JS_BUTTON0, &(keyrecord_t){.event.pressed = true});
            break;
        case TD_HOLD:
            process_joystick(JS_BUTTON1, &(keyrecord_t){.event.pressed = true});
    //        layer_on(_FN);
            break;
        case TD_UNKNOWN:
            break;
    }
}

void ltfn_js_reset(qk_tap_dance_state_t *state, void *user_data) {
    switch (td_state) {
        case TD_TAP:
            process_joystick(JS_BUTTON0, &(keyrecord_t){.event.pressed = false});
            break;
        case TD_HOLD:
            process_joystick(JS_BUTTON1, &(keyrecord_t){.event.pressed = false});
            // layer_off(_FN);
            break;
        case TD_UNKNOWN:
            break;
    }
}

qk_tap_dance_action_t tap_dance_actions[] = {
    [TD_LTFN_JS] = ACTION_TAP_DANCE_FN_ADVANCED(NULL, ltfn_js_finished, ltfn_js_reset)
};
