#include "utilities.h"

int64_t clampValue (int64_t min, int64_t max, int64_t value) {
    if (value > max) { return max; }
    if (value < min) { return min; }
    return value;
}