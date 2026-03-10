#ifndef CORDIC_EXP_H
#define CORDIC_EXP_H

#include "common.h"

#define CORDIC_MAX_ITER 25
#define CORDIC_Q_FORMAT 24
#define CORDIC_ONE (1 << CORDIC_Q_FORMAT)

static const int32_t cordic_atanh_q24[CORDIC_MAX_ITER] = {
    9215827, 4285115, 2108178, 1049944,
    524458, 262165, 131074, 65536,
    32768, 16384, 8192, 4096,
    2048, 1024, 512, 256,
    128, 64, 31, 15,
    7, 3, 1, 0, 0
};

static const int32_t cordic_gain_q24 = 20218832;
static const int32_t cordic_ln2_q24 = 11629079;
static const int32_t cordic_max_input_q24 = 18756927;

static inline int32_t cordic_exp_q24(int32_t x_q24) {
    if (x_q24 > (11 << 24)) return Q31_MAX;
    if (x_q24 < (-11 << 24)) return 0;
    
    int sign = x_q24 < 0 ? -1 : 1;
    int32_t x_work = x_q24 < 0 ? -x_q24 : x_q24;
    
    int k = 0;
    while (x_work > cordic_max_input_q24 && k < 15) {
        x_work -= cordic_ln2_q24;
        k++;
    }
    
    int64_t x_val = cordic_gain_q24;
    int64_t y_val = 0;
    int64_t z_val = x_work;
    
    for (int i = 1; i <= CORDIC_MAX_ITER; i++) {
        int d = (z_val >= 0) ? 1 : -1;
        
        int64_t x_new = x_val + ((d * y_val) >> i);
        int64_t y_new = y_val + ((d * x_val) >> i);
        z_val = z_val - d * cordic_atanh_q24[i-1];
        x_val = x_new;
        y_val = y_new;
        
        if (i == 4 || i == 13) {
            d = (z_val >= 0) ? 1 : -1;
            x_new = x_val + ((d * y_val) >> i);
            y_new = y_val + ((d * x_val) >> i);
            z_val = z_val - d * cordic_atanh_q24[i-1];
            x_val = x_new;
            y_val = y_new;
        }
    }
    
    int64_t result = x_val + y_val;
    
    if (k > 0) {
        for (int j = 0; j < k; j++) {
            result = result << 1;
        }
    }
    
    int32_t result_q16;
    if (sign < 0) {
        if (result <= 1) return 0;
        int64_t temp = ((int64_t)CORDIC_ONE << 16) / result;
        result_q16 = (int32_t)temp;
    } else {
        result_q16 = (int32_t)(result >> 8);
    }
    
    if (result_q16 > Q31_MAX) return Q31_MAX;
    if (result_q16 < 0) return 0;
    
    return result_q16;
}

static uint64_t estimate_cycles_cordic(int32_t x_int) {
    uint64_t cycles = 10;
    cycles += (uint64_t)(x_int * (CM4_INT32.add + CM4_INT32.cmp + CM4_INT32.branch));
    cycles += (CORDIC_MAX_ITER + 2) * (CM4_INT32.mul * 2 + CM4_INT32.add * 3 + CM4_INT32.cmp + CM4_INT32.branch);
    
    return cycles;
}

#endif
