#ifndef HAETAE_EXP_H
#define HAETAE_EXP_H

#include "common.h"

static const int64_t haetae_exp_int_lut[11] = {
    65536LL,
    178145LL,
    484082LL,
    1315827LL,
    3575102LL,
    9717756LL,
    26411121LL,
    71744976LL,
    194977586LL,
    529883925LL,
    1440261166LL
};

static inline int32_t haetae_exp_q16(int32_t x_q16) {
    if (x_q16 > 655360) return Q31_MAX;
    if (x_q16 < -655360) return 0;
    
    int is_negative = x_q16 < 0;
    int32_t x_work = is_negative ? -x_q16 : x_q16;
    
    int x_int = x_work >> 16;
    int32_t x_frac = x_work & 0xFFFF;
    
    int64_t x1 = (int64_t)x_frac;
    int64_t x2 = (x1 * x1) >> 16;
    int64_t x3 = (x2 * x1) >> 16;
    int64_t x4 = (x3 * x1) >> 16;
    int64_t x5 = (x4 * x1) >> 16;
    int64_t x6 = (x5 * x1) >> 16;
    int64_t x7 = (x6 * x1) >> 16;
    int64_t x8 = (x7 * x1) >> 16;
    
    int64_t result = 65536LL;
    result += x1;
    result += x2 / 2;
    result += x3 / 6;
    result += x4 / 24;
    result += x5 / 120;
    result += x6 / 720;
    result += x7 / 5040;
    result += x8 / 40320;
    
    if (x_int > 0 && x_int <= 10) {
        result = (result * haetae_exp_int_lut[x_int]) >> 16;
    }
    
    int32_t result_q16;
    if (is_negative) {
        if (result <= 0) return 0;
        
        int64_t scaled = result;
        if (scaled < 1) scaled = 1;
        
        result_q16 = (int32_t)((4294967296LL) / scaled);
        
        if (result_q16 < 1) result_q16 = 1;
    } else {
        result_q16 = (int32_t)result;
    }
    
    if (result_q16 > Q31_MAX) return Q31_MAX;
    if (result_q16 < 0) return 0;
    
    return result_q16;
}

static uint64_t estimate_cycles_haetae(void) {
    uint64_t cycles = 30;
    cycles += 10 * (CM4_INT32.mul * 2 + CM4_INT32.add);
    return cycles;
}

#endif
