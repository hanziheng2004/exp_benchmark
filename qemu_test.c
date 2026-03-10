#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#include "common.h"
#include "cmsis_nn_exp.h"
#include "cordic_exp.h"
#include "haetae_exp.h"
#include "ref_values.h"

static double calculate_relative_error(double computed, double expected) {
    if (expected == 0) return 0;
    return fabs(computed - expected) / fabs(expected) * 100.0;
}

static inline double q31_to_double(int32_t val) {
    return (double)val / 2147483648.0;
}

static inline double q24_to_double(int32_t val) {
    return (double)val / 65536.0;
}

static inline double q16_to_double(int32_t val) {
    return (double)val / 65536.0;
}

static inline double cmsis_result_to_double(int32_t val) {
    return (double)val / 65536.0;
}

int main(void) {
    printf("========================================\n");
    printf("ARM Cortex-M4 EXP Benchmark\n");
    printf("Architecture: ARM Cortex-M4 (No FPU)\n");
    printf("Test Count: %d\n", TEST_COUNT);
    printf("Pure Fixed-Point Implementation\n");
    printf("Reference: PC high precision (pre-computed)\n");
    printf("========================================\n\n");
    
    printf("Algorithm Implementations:\n");
    printf("  CMSIS-NN: cmsis_nn_exp_q524()\n");
    printf("    - Input: Q5.24 (int32_t)\n");
    printf("    - Output: Q16.16 (int32_t)\n");
    printf("    - Pure fixed-point, no float\n\n");
    printf("  CORDIC: cordic_exp_q24()\n");
    printf("    - Input: Q24 (int32_t)\n");
    printf("    - Output: Q24 (int32_t)\n");
    printf("    - Pure fixed-point, no float\n\n");
    printf("  Haetae: haetae_exp_q16()\n");
    printf("    - Input: Q16.16 (int32_t)\n");
    printf("    - Output: Q16.16 (int32_t)\n");
    printf("    - Pure fixed-point, no float\n\n");
    
    printf("========================================\n");
    printf("Test 1: CMSIS-NN (Pure Fixed-Point)\n");
    printf("========================================\n\n");
    
    double cmsis_max_err = 0, cmsis_total_err = 0;
    uint64_t cmsis_total_cycles = 0;
    clock_t start = clock();
    
    for (int i = 0; i < TEST_COUNT; i++) {
        double expected = ref_expected[i];
        
        int32_t result_q31 = cmsis_nn_exp_q524(ref_input_q524[i]);
        double result = cmsis_result_to_double(result_q31);
        
        double x = ref_expected[i];
        double x_dbl = (x > 0) ? log(x) : -log(1.0/x);
        int x_int = (int)fabs(x_dbl);
        cmsis_total_cycles += estimate_cycles_cmsis(x_int);
        
        double err = calculate_relative_error(result, expected);
        if (err > cmsis_max_err) cmsis_max_err = err;
        cmsis_total_err += err;
    }
    double cmsis_time = (double)(clock() - start) / CLOCKS_PER_SEC;
    
    printf("CMSIS-NN Results:\n");
    printf("  Time: %.4f seconds\n", cmsis_time);
    printf("  Avg Cycles: %llu\n", (unsigned long long)(cmsis_total_cycles / TEST_COUNT));
    printf("  Max Relative Error: %.4f%%\n", cmsis_max_err);
    printf("  Avg Relative Error: %.4f%%\n\n", cmsis_total_err / TEST_COUNT);
    
    printf("========================================\n");
    printf("Test 2: CORDIC (Pure Fixed-Point)\n");
    printf("========================================\n\n");
    
    double cordic_max_err = 0, cordic_total_err = 0;
    uint64_t cordic_total_cycles = 0;
    start = clock();
    
    for (int i = 0; i < TEST_COUNT; i++) {
        double expected = ref_expected[i];
        
        int32_t result_q24 = cordic_exp_q24(ref_input_q24[i]);
        double result = q24_to_double(result_q24);
        
        double x = ref_expected[i];
        double x_dbl = (x > 0) ? log(x) : -log(1.0/x);
        int x_int = (int)fabs(x_dbl);
        cordic_total_cycles += estimate_cycles_cordic(x_int);
        
        double err = calculate_relative_error(result, expected);
        if (err > cordic_max_err) cordic_max_err = err;
        cordic_total_err += err;
    }
    double cordic_time = (double)(clock() - start) / CLOCKS_PER_SEC;
    
    printf("CORDIC Results:\n");
    printf("  Time: %.4f seconds\n", cordic_time);
    printf("  Avg Cycles: %llu\n", (unsigned long long)(cordic_total_cycles / TEST_COUNT));
    printf("  Max Relative Error: %.4f%%\n", cordic_max_err);
    printf("  Avg Relative Error: %.4f%%\n\n", cordic_total_err / TEST_COUNT);
    
    printf("========================================\n");
    printf("Test 3: Haetae (Pure Fixed-Point)\n");
    printf("========================================\n\n");
    
    double haetae_max_err = 0, haetae_total_err = 0;
    uint64_t haetae_total_cycles = 0;
    start = clock();
    
    for (int i = 0; i < TEST_COUNT; i++) {
        double expected = ref_expected[i];
        
        int32_t result_q16 = haetae_exp_q16(ref_input_q16[i]);
        double result = q16_to_double(result_q16);
        
        haetae_total_cycles += estimate_cycles_haetae();
        
        double err = calculate_relative_error(result, expected);
        if (err > haetae_max_err) haetae_max_err = err;
        haetae_total_err += err;
    }
    double haetae_time = (double)(clock() - start) / CLOCKS_PER_SEC;
    
    printf("Haetae Results:\n");
    printf("  Time: %.4f seconds\n", haetae_time);
    printf("  Avg Cycles: %llu\n", (unsigned long long)(haetae_total_cycles / TEST_COUNT));
    printf("  Max Relative Error: %.4f%%\n", haetae_max_err);
    printf("  Avg Relative Error: %.4f%%\n\n", haetae_total_err / TEST_COUNT);
    
    printf("========================================\n");
    printf("Summary Comparison\n");
    printf("========================================\n\n");
    printf("%-12s %10s %12s %12s %12s\n", "Algorithm", "Avg Cycles", "Max Error", "Avg Error", "Time(s)");
    printf("----------------------------------------------------------------\n");
    printf("%-12s %10llu %11.4f%% %11.4f%% %12.4f\n", "CMSIS-NN", 
           (unsigned long long)(cmsis_total_cycles / TEST_COUNT),
           cmsis_max_err, cmsis_total_err / TEST_COUNT, cmsis_time);
    printf("%-12s %10llu %11.4f%% %11.4f%% %12.4f\n", "CORDIC",
           (unsigned long long)(cordic_total_cycles / TEST_COUNT),
           cordic_max_err, cordic_total_err / TEST_COUNT, cordic_time);
    printf("%-12s %10llu %11.4f%% %11.4f%% %12.4f\n", "Haetae",
           (unsigned long long)(haetae_total_cycles / TEST_COUNT),
           haetae_max_err, haetae_total_err / TEST_COUNT, haetae_time);
    
    printf("\n========================================\n");
    printf("Sample Values Comparison\n");
    printf("========================================\n\n");
    
    double test_x[] = {-5.0, -4.0, -3.0, -2.0, -1.0, -0.5, 0.0, 0.5, 1.0, 2.0, 3.0, 4.0, 5.0};
    printf("%8s %12s %12s %12s %12s\n", "x", "Expected", "CMSIS-NN", "CORDIC", "Haetae");
    printf("------------------------------------------------------------\n");
    
    for (int i = 0; i < 13; i++) {
        double x = test_x[i];
        double expected = exp(x);
        
        int32_t x_q524 = (int32_t)(x * 16777216.0);
        int32_t x_q24 = (int32_t)(x * 16777216.0);
        int32_t x_q16 = (int32_t)(x * 65536.0);
        
        int32_t result_cmsis = cmsis_nn_exp_q524(x_q524);
        int32_t result_cordic = cordic_exp_q24(x_q24);
        int32_t result_haetae = haetae_exp_q16(x_q16);
        
        printf("%8.2f %12.6f %12.6f %12.6f %12.6f\n", x, expected,
               cmsis_result_to_double(result_cmsis),
               q24_to_double(result_cordic),
               q16_to_double(result_haetae));
    }
    
    printf("\n========================================\n");
    printf("Test Complete!\n");
    printf("========================================\n");
    
    printf("\nImplementation Notes:\n");
    printf("All algorithms use PURE FIXED-POINT interfaces:\n\n");
    printf("CMSIS-NN: cmsis_nn_exp_q524(int32_t x_q524)\n");
    printf("  - Input: Q5.24 fixed-point (x * 2^24)\n");
    printf("  - Output: Q16.16 fixed-point (result / 2^16)\n");
    printf("  - No floating-point operations in core algorithm\n\n");
    printf("CORDIC: cordic_exp_q24(int32_t x_q24)\n");
    printf("  - Input: Q24 fixed-point (x * 2^24)\n");
    printf("  - Output: Q16.16 fixed-point (result / 2^16)\n");
    printf("  - Uses shift-add operations only\n\n");
    printf("Haetae: haetae_exp_q16(int32_t x_q16)\n");
    printf("  - Input: Q16.16 fixed-point (x * 2^16)\n");
    printf("  - Output: Q16.16 fixed-point (result / 2^16)\n");
    printf("  - Taylor expansion with LUT\n");
    printf("\nCycle counts are estimated for ARM Cortex-M4.\n");
    printf("Reference values are pre-computed on PC with high precision.\n");
    
    return 0;
}
