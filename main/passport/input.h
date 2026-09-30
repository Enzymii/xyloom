#pragma once
#include <stdbool.h>
#include <stdint.h>

typedef enum { KEY_NONE, KEY_LEFT, KEY_RIGHT, KEY_OK, KEY_INVALID } passport_key_t;
typedef enum { INPUT_NONE, LEFT_SHORT, LEFT_LONG, RIGHT_SHORT, RIGHT_LONG, OK_SHORT, INPUT_WAKE } passport_input_event_t;
enum { INPUT_DEBOUNCE_MS = 30, INPUT_LONG_MS = 700 };
typedef struct {
    passport_key_t candidate, stable;
    uint32_t candidate_since, pressed_at;
    bool consumed, suppress_until_release;
} passport_input_t;

void passport_input_init(passport_input_t *input);
/* One physical ladder sample per tick. Invalid ADC samples cancel the gesture.
 * Sleeping wakes on stable press, then consumes the entire wake gesture.
 * Blocked transitions also consume presses that outlive the animation. */
passport_input_event_t passport_input_sample(passport_input_t *input,
    passport_key_t raw, uint32_t now, bool sleeping, bool blocked);
