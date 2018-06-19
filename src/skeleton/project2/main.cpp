#include "fpga.h"

//set this to 1 if you want to test with input.txt only.
static const size_t num_tests = 1 << 16;

int main(int argc, char **argv)
{
	fpga_tb testbench(SIZE_SHIFTER);

	if(!testbench.is_initialized())
	{
		return -1;
	}

	#ifdef DEBUG

	if(!testbench.load_file(std::string("input.txt")))
	{
		return -1;
	}

	testbench.baseline_calculate();
	testbench.arm_calculate();
	testbench.fpga_calculate();

	#else

	for(size_t i = 0 ; i < num_tests ; i++)
	{
		if(!testbench.load_random())
		{
			return -1;
		}

		testbench.baseline_calculate();
		testbench.arm_calculate();
		testbench.fpga_calculate();
	}

	#endif

	return 0;

}
