#ifndef ZYNQ_H
#define ZYNQ_H
#include <iostream>
#include <string>

#define SCALE_SHIFTER 8	//Integer part : 4, point : 12
#define SCALE_FACTOR (1 << SCALE_SHIFTER)

#define SIZE_SHIFTER 6
#define MATRIX_SIZE (1 << SIZE_SHIFTER)
#define ERROR_THRESHOLD ((double)0.01)	//Max error torelance : 1%

class zynq_tb
{
public:
	zynq_tb() = delete;
	zynq_tb(const zynq_tb &) = delete;
	zynq_tb(const zynq_tb &&) = delete;

	zynq_tb &operator=(const zynq_tb &) = delete;
	zynq_tb &operator=(const zynq_tb &&) = delete;

	zynq_tb(size_t size_shifter);
	~zynq_tb();

	const bool &is_initialized();

	bool load_file(std::string filename);
	bool load_random();

	void baseline_calculate();
	void arm_calculate();
	void fpga_calculate();

	virtual void fpga_allocate_resources() = 0;
	virtual void fpga_load_execute_and_copy(const uint16_t *ipt_matrix_fix16, const uint16_t *ipt_vector_fix16, uint32_t *your_vector_fix32) = 0;
	virtual void fpga_cleanup() = 0;
/*
	virtual void fpga_load_to_bram(const uint16_t *ipt_matrix_fix16, const uint16_t *ipt_vector_fix16) = 0;
	virtual void fpga_execute_and_copy(uint32_t *your_vector_f32) = 0;
	virtual void fpga_cleanup() = 0;
*/


private:

	bool initialized;

	size_t size_shifter;
	size_t matrix_size;

	uint16_t *ipt_matrix_fix16, *ipt_vector_fix16;
	uint32_t *arm_vector_fix32, *your_vector_fix32;
	float *baseline_vector_f32;

	size_t total_errors;
	double arm_time_total, fpga_time_total;
	double arm_time, fpga_time;

};

#endif
