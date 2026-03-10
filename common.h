#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>

#define Q31_MAX  0x7FFFFFFF
#define Q31_MIN  (-0x7FFFFFFF - 1)

typedef struct {
    uint32_t add;
    uint32_t sub;
    uint32_t mul;
    uint32_t div;
    uint32_t cmp;
    uint32_t branch;
    uint32_t clz;
} CycleModel;

static const CycleModel CM4_INT32 = {
    .add = 1, .sub = 1, .mul = 3, .div = 4,
    .cmp = 1, .branch = 1, .clz = 1
};

static inline double q_output_to_double(int32_t val) {
    return (double)val / 65536.0;
}

static inline double calculate_ulp_error(double computed, double expected) {
    if (expected == 0) return 0;
    double rel_err = fabs(computed - expected) / fabs(expected);
    return rel_err * 4294967296.0;
}

#endif
