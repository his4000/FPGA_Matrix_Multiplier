#ifndef ZYNQ_H
#define ZYNQ_H
#include <iostream>

#define SIZE_SHIFTER 6
#define MATRIX_SIZE (1 << SIZE_SHIFTER)
#define ERROR_THRESHOLD ((double)0.01)	//Max error torelance : 1%

static const int matrix_size = MATRIX_SIZE;

double fpga_calculate(uint32_t *ipt_matrix_f16, uint32_t *ipt_vector_f16, float *your_vector_f32);
double arm_calculate(uint32_t *ipt_matrix_f16, uint32_t *ipt_vector_f16, float *arm_vector_f32);

#endif
