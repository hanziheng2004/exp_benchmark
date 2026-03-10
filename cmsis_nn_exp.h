#ifndef CMSIS_NN_EXP_H
#define CMSIS_NN_EXP_H

#include "common.h"

#define NN_Q31_MAX 0x7FFFFFFF
#define NN_Q31_MIN (-0x7FFFFFFF - 1)

static inline int32_t arm_nn_doubling_high_mult(int32_t val, int32_t mult) {
    int64_t result = (int64_t)val * mult;
    return (int32_t)(result >> 31);
}

static inline int32_t arm_nn_divide_by_power_of_two(int32_t val, int32_t exp) {
    int32_t result = val >> exp;
    int32_t remainder = val - (result << exp);
    if (remainder < 0) remainder = -remainder;
    if ((remainder << 1) >= (1 << exp)) {
        if (val >= 0) result++;
        else result--;
    }
    return result;
}

static inline int32_t arm_nn_mult_by_power_of_two(int32_t val, int32_t exp) {
    const int32_t thresh = ((1 << (31 - exp)) - 1);
    int32_t result = val << exp;
    if (val > thresh) result = NN_Q31_MAX;
    if (val < -thresh) result = NN_Q31_MIN;
    return result;
}

#define MUL_SAT(a, b) arm_nn_doubling_high_mult((a), (b))
#define DIV_POW2(a, b) arm_nn_divide_by_power_of_two((a), (b))
#define MUL_POW2(a, b) arm_nn_mult_by_power_of_two((a), (b))
#define MASK_IF_ZERO(x) ((x) == 0 ? ~0 : 0)
#define MASK_IF_NON_ZERO(x) ((x) != 0 ? ~0 : 0)
#define SELECT_USING_MASK(mask, a, b) (((mask) & (a)) ^ (~(mask) & (b)))

static inline int32_t arm_nn_exp_on_negative_values(int32_t val) {
    int32_t mask = 0;
    int32_t shift = 24;

    const int32_t val_mod_minus_quarter = (val & ((1 << shift) - 1)) - (1 << shift);
    const int32_t remainder = val_mod_minus_quarter - val;
    const int32_t x = (val_mod_minus_quarter << 5) + (1 << 28);
    const int32_t x2 = MUL_SAT(x, x);

    int32_t result = 1895147668 +
        MUL_SAT(1895147668, x + DIV_POW2(MUL_SAT(DIV_POW2(MUL_SAT(x2, x2), 2) + MUL_SAT(x2, x), 715827883) + x2, 1));

#define SELECT_IF_NON_ZERO(x_val)                                                                                       \
    {                                                                                                                   \
        mask = MASK_IF_NON_ZERO(remainder & (1 << shift++));                                                            \
        result = SELECT_USING_MASK(mask, MUL_SAT(result, x_val), result);                                               \
    }

    SELECT_IF_NON_ZERO(1672461947)
    SELECT_IF_NON_ZERO(1302514674)
    SELECT_IF_NON_ZERO(790015084)
    SELECT_IF_NON_ZERO(290630308)
    SELECT_IF_NON_ZERO(39332535)
    SELECT_IF_NON_ZERO(720401)
    SELECT_IF_NON_ZERO(242)

#undef SELECT_IF_NON_ZERO

    mask = MASK_IF_ZERO(val);
    return SELECT_USING_MASK(mask, NN_Q31_MAX, result);
}

static const int64_t cmsis_exp_int_lut[12] = {
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
    1440261166LL,
    3913319052LL
};

static inline int32_t cmsis_nn_exp_q524(int32_t x_q524) {
    if (x_q524 > (11 << 24)) return Q31_MAX;
    if (x_q524 < (-11 << 24)) return 0;
    
    int is_negative = x_q524 < 0;
    int32_t x_work = is_negative ? -x_q524 : x_q524;
    
    int x_int = x_work >> 24;
    int32_t x_frac = x_work & 0xFFFFFF;
    
    int32_t val = -(x_frac << 2);
    int32_t exp_frac_q31 = arm_nn_exp_on_negative_values(val);
    
    int64_t exp_frac_q16 = ((int64_t)exp_frac_q31 >> 15);
    
    int64_t result_q16;
    
    if (is_negative) {
        result_q16 = exp_frac_q16;
        if (x_int > 0 && x_int <= 11) {
            int64_t exp_int_recip_q16 = (65536LL << 16) / cmsis_exp_int_lut[x_int];
            result_q16 = (result_q16 * exp_int_recip_q16) >> 16;
        }
    } else {
        if (exp_frac_q16 <= 0) return 0;
        int64_t temp = (65536LL << 16) / exp_frac_q16;
        result_q16 = temp;
        if (x_int > 0 && x_int <= 11) {
            result_q16 = (result_q16 * cmsis_exp_int_lut[x_int]) >> 16;
        }
    }
    
    if (result_q16 > Q31_MAX) return Q31_MAX;
    if (result_q16 < 0) return 0;
    
    return (int32_t)result_q16;
}

static uint64_t estimate_cycles_cmsis(int x_int) {
    uint64_t cycles = 20;
    cycles += 10 * CM4_INT32.mul;
    cycles += 8 * CM4_INT32.add;
    cycles += 7 * CM4_INT32.cmp;
    cycles += 7 * CM4_INT32.branch;
    if (x_int > 0) cycles += CM4_INT32.mul + CM4_INT32.div;
    return cycles;
}

#endif
