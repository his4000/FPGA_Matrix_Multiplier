#include "zynq.h" 

#include <fstream>
#include <sys/time.h>

#include <cstring>
#include <new>

using std::cin;
using std::cout;
using std::endl;
using std::cerr;

using std::string;
using std::ifstream;

zynq_tb::zynq_tb(size_t size_shifter_)
{
	initialized = false;

	size_shifter = size_shifter_;
	matrix_size = 1 << size_shifter;

	try
	{
		ipt_matrix_fix16 = new uint16_t[matrix_size * matrix_size];
		ipt_vector_fix16 = new uint16_t[matrix_size];

		arm_vector_fix32 = new uint32_t[matrix_size];
		your_vector_fix32 = new uint32_t[matrix_size];

		baseline_vector_f32 = new float[matrix_size];
	}
	catch(const std::bad_alloc &e)
	{
		cout << "Memory allocation failed\n";
		return;
	}

	initialized = true;

	total_errors = 0;

	arm_time_total = 0;
	fpga_time_total = 0;
}

zynq_tb::~zynq_tb()
{
	#ifndef DEBUG
	cout << "Elapsed time(ARM CPU) : " << arm_time_total << endl;
	cout << "Elapsed time(Your IP) : " << fpga_time_total << endl;
	cout << "Total number of erros : " << total_errors << endl;
	#endif

	delete []ipt_matrix_fix16;
	delete []ipt_vector_fix16;

	delete []arm_vector_fix32;
	delete []your_vector_fix32;

	delete []baseline_vector_f32;
}

const bool& zynq_tb::is_initialized()
{
	return initialized;
}

bool zynq_tb::load_file(string filename)
{
	
	if(!initialized)
	{
		cerr << "Class is not initialized yet\n";
		return false;
	}

	ifstream ipt_file;

	//open input file.
	ipt_file.exceptions(ifstream::failbit | ifstream::badbit | ifstream::eofbit);
	try
	{
		ipt_file.open(filename);
	}
	catch(ifstream::failure &e)
	{
		cerr << "Error opening file." << endl;
		cerr << e.what() << endl;
		return false;
	}

	//Get input from file
	try
	{
		for(size_t i = 0 ; i < matrix_size * matrix_size ; i++)
		{
			ipt_file >> ipt_matrix_fix16[i];
		}
		
		for(size_t i = 0 ; i < matrix_size ; i++)
		{
			ipt_file >> ipt_vector_fix16[i];

			baseline_vector_f32[i] = 0;
			arm_vector_fix32[i] = 0;
		}
	}
	catch(ifstream::failure e)
	{
		if(ipt_file.rdstate() & ifstream::eofbit)
		{
			cerr << "Stream handler met EOF earlier than expected.\n";
			cerr << "Please check length of your file and try again.\n";
		}
		cerr << e.what() << endl;
		ipt_file.close();
		return false;
	}

	ipt_file.close();
	return true;
}

bool zynq_tb::load_random()
{
	if(!initialized)
	{
		cerr << "Class is not initialized yet\n";
		return false;
	}

	struct timeval tv;
	gettimeofday(&tv, NULL);
	srand(tv.tv_usec);

	for(size_t i = 0 ; i < matrix_size * matrix_size ; i++)
	{
		do
		{
			ipt_matrix_fix16[i] = rand() & 0x7FF;	//modulo 2048
		}
		while(ipt_matrix_fix16[i] < 0x40);

	}

	for(size_t i = 0 ; i < matrix_size ; i++)
	{
		do
		{
			ipt_vector_fix16[i] = rand() & 0x7FF;
		}
		while(ipt_vector_fix16[i] < 0x40);
		
		baseline_vector_f32[i] = 0;
		arm_vector_fix32[i] = 0;
	}

	return true;
}


void zynq_tb::baseline_calculate()
{
	for(size_t i = -1 ; ++i < matrix_size * matrix_size ; baseline_vector_f32[i >> size_shifter]
		+= ((float)ipt_matrix_fix16[i] / SCALE_FACTOR) * (((float)(ipt_vector_fix16[i & (matrix_size - 1)])) / SCALE_FACTOR));
}

void zynq_tb::arm_calculate()
{
	struct timeval start, end;
	gettimeofday(&start, NULL);

	for(size_t i = -1 ; ++i < matrix_size * matrix_size ; arm_vector_fix32[i >> size_shifter]
		+= ((ipt_matrix_fix16[i] * ipt_vector_fix16[i & (matrix_size - 1)]) >> SCALE_SHIFTER));
	
	gettimeofday(&end, NULL);
	arm_time = (double)(end.tv_sec - start.tv_sec) + ((double)(end.tv_usec - start.tv_usec)) / 1000000.0;
	arm_time_total += arm_time;

}

void zynq_tb::fpga_calculate()
{
	fpga_allocate_resources();

	struct timeval start, end;
	gettimeofday(&start, NULL);

	//calculate
	fpga_load_execute_and_copy(ipt_matrix_fix16, ipt_vector_fix16, your_vector_fix32);

	gettimeofday(&end, NULL);
	fpga_time = (double)(end.tv_sec - start.tv_sec) + ((double)(end.tv_usec - start.tv_usec)) / 1000000.0;
	fpga_time_total += fpga_time;

	//cleanup
	fpga_cleanup();

	//report
	size_t error_cnt = 0;

	#ifdef DEBUG
	cout 	<< "(baseline) (CPU code) (your code)\n";
	#endif
	for(size_t i = 0 ; i < matrix_size ; i++)
	{
		double your_value = (float)your_vector_fix32[i] / (1 << SCALE_SHIFTER);
		double your_error = (your_value - baseline_vector_f32[i]) / baseline_vector_f32[i];

		#ifdef DEBUG
		double arm_value = (float)arm_vector_fix32[i] / (1 << SCALE_SHIFTER);
		double arm_error = (arm_value - baseline_vector_f32[i]) / baseline_vector_f32[i];

		cout 	<< baseline_vector_f32[i] << ", "
			<< arm_value << "(" << arm_error * 100 << "%), "
			<< your_value << "(" << your_error * 100 << "%)\n";
		#endif

		if(your_error > ERROR_THRESHOLD || your_error < -ERROR_THRESHOLD)
		{
			#ifdef DEBUG
			cerr	<< "Your result exceeded error threshold("
				<< ERROR_THRESHOLD * 100 << "%)\n";
			#endif
			error_cnt++;
		}
	}

	#ifdef DEBUG
	cout << "Elapsed time(ARM CPU) : " << arm_time << endl;
	cout << "Elapsed time(Your IP) : " << fpga_time << endl;
	cout << "Number of errors : " << error_cnt << '/' << matrix_size
		<< '(' << ((double)error_cnt * 100) / matrix_size << "%)" << endl;
	#endif

	total_errors += error_cnt;
}

