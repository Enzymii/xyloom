#include "input.h"
#include <string.h>

void passport_input_init(passport_input_t *input) {
    memset(input, 0, sizeof(*input));
}

passport_input_event_t passport_input_sample(passport_input_t *s,
    passport_key_t raw, uint32_t now, bool sleeping, bool blocked) {
    if (raw == KEY_INVALID) {
        s->suppress_until_release = true;
        s->candidate = KEY_INVALID;
        s->candidate_since = now;
        return INPUT_NONE;
    }
    /* The previous stable release is sufficient, even when the first sample
     * after an animation is already a new press. Clear before replacing the
     * candidate; an unreleased or invalid gesture remains suppressed. */
    if (!blocked && s->candidate == KEY_NONE && s->stable == KEY_NONE &&
        (uint32_t)(now - s->candidate_since) >= INPUT_DEBOUNCE_MS)
        s->suppress_until_release = false;
    if (blocked) s->suppress_until_release = true;
    if (raw != s->candidate) {
        s->candidate = raw;
        s->candidate_since = now;
    }
    if (raw != s->stable && (uint32_t)(now - s->candidate_since) >= INPUT_DEBOUNCE_MS) {
        passport_key_t previous = s->stable;
        s->stable = raw;
        if (raw == KEY_NONE) {
            bool skip = s->consumed || s->suppress_until_release;
            uint32_t duration = s->candidate_since - s->pressed_at;
            s->consumed = false;
            s->suppress_until_release = blocked;
            if (skip || previous == KEY_NONE || previous == KEY_INVALID) return INPUT_NONE;
            if (duration >= INPUT_LONG_MS) {
                return previous == KEY_LEFT ? LEFT_LONG : previous == KEY_RIGHT ? RIGHT_LONG : INPUT_NONE;
            }
            return previous == KEY_LEFT ? LEFT_SHORT : previous == KEY_RIGHT ? RIGHT_SHORT : OK_SHORT;
        }
        s->pressed_at = now;
        s->consumed = false;
        /* A ladder jump without a stable release is not a second gesture. */
        if (previous != KEY_NONE) s->suppress_until_release = true;
        if (sleeping) {
            s->suppress_until_release = true;
            return INPUT_WAKE;
        }
    }
    if (raw == s->stable && s->stable != KEY_NONE && !s->consumed && !s->suppress_until_release &&
        (uint32_t)(now - s->pressed_at) >= INPUT_LONG_MS) {
        s->consumed = true;
        return s->stable == KEY_LEFT ? LEFT_LONG : s->stable == KEY_RIGHT ? RIGHT_LONG : INPUT_NONE;
    }
    return INPUT_NONE;
}
