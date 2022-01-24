#include <string.h>
#include "matrix.h"
#include "quantum.h"

#define ROWS_PER_HAND (MATRIX_ROWS / 2)
static pin_t row_pins[MATRIX_ROWS] = MATRIX_ROW_PINS;
static pin_t col_pins[MATRIX_COLS] = MATRIX_COL_PINS;

static inline void setPinOutput_writeLow(pin_t pin) {
    ATOMIC_BLOCK_FORCEON {
        setPinOutput(pin);
        writePinLow(pin);
    }
}

static inline void setPinInputHigh_atomic(pin_t pin) {
    ATOMIC_BLOCK_FORCEON { setPinInputHigh(pin); }
}

static bool select_row(uint8_t row) {
    pin_t pin = row_pins[row];
    if (pin != NO_PIN) {
        setPinOutput_writeLow(pin);
        return true;
    }
    return false;
}

static inline uint8_t readMatrixPin(pin_t pin) {
    if (pin != NO_PIN) {
        return readPin(pin);
    } else {
        return 1;
    }
}

static void unselect_row(uint8_t row) {
    pin_t pin = row_pins[row];
    if (pin != NO_PIN) {
        setPinInputHigh_atomic(pin);
    }
}

void matrix_read_cols_on_row(matrix_row_t current_matrix[], uint8_t current_row) {
    // Start with a clear matrix row
    matrix_row_t current_row_value = 0;

    if (current_row < ROWS_PER_HAND - 1) {
        if (!select_row(current_row)) {  // Select row
            return;                      // skip NO_PIN row
        }
        matrix_output_select_delay();

        // For each col...
        matrix_row_t row_shifter = MATRIX_ROW_SHIFTER;
        for (uint8_t col_index = 0; col_index < MATRIX_COLS; col_index++, row_shifter <<= 1) {
            uint8_t pin_state = readMatrixPin(col_pins[col_index]);

            // Populate the matrix row with the state of the col pin
            current_row_value |= pin_state ? 0 : row_shifter;
        }

        // Unselect row
        unselect_row(current_row);
        matrix_output_unselect_delay(current_row, current_row_value != 0);  // wait for all Col signals to go HIGH
    } else {
        if (readPin(TOUCH_PIN)) {
            current_row_value |= MATRIX_ROW_SHIFTER;
        }
        if (!readPin(STICK_PUSH_PIN)) {
            current_row_value |= (MATRIX_ROW_SHIFTER << 1);
        }
    }

    // Update the matrix
    current_matrix[current_row] = current_row_value;
}
