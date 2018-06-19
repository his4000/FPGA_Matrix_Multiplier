#include "zynq.h"
#include <iostream>
#include <fstream>
#include <cstring>

#include <sys/time.h>

#include <hls_half.h>

using std::cin;
using std::cout;
using std::endl;
using std::cerr;

using std::ifstream;

int main(int argc, char **argv)
{
	ifstream ipt_file;
	float *ipt_matrix_f32, *ipt_vector_f32, *opt_vector_f32;
	uint32_t *ipt_matrix_f16, *ipt_vector_f16;

	//open input file.
	ipt_file.exceptions(ifstream::failbit | ifstream::badbit | ifstream::eofbit);
	try
	{
		ipt_file.open("input.txt");
	}
	catch(ifstream::failure e)
	{
		cerr << "Error opening file." << endl;
		cerr << e.what() << endl;
		return -1;
	}

	//32-bit floating point operands
	ipt_matrix_f32 = new float[matrix_size * matrix_size];
	ipt_vector_f32 = new float[matrix_size];
	opt_vector_f32 = new float[matrix_size];
	
	//16-bit floating point operands
	ipt_matrix_f16 = new uint32_t[matrix_size * matrix_size];
	ipt_vector_f16 = new uint32_t[matrix_size];

	//Get input from file
	try
	{
		for(size_t i = 0 ; i < matrix_size * matrix_size ; i++)
		{
			ipt_file >> ipt_matrix_f32[i];
			half val(ipt_matrix_f32[i]);

			//Converting to Big endian
			ipt_matrix_f16[i] = 0;
			memcpy(((void*)(ipt_matrix_f16 + i)) + 2, ((void*)&val) + 1, 1);
			memcpy(((void*)(ipt_matrix_f16 + i)) + 3, ((void*)&val) + 0, 1);
		}
		
		for(size_t i = 0 ; i < matrix_size ; i++)
		{
			ipt_file >> ipt_vector_f32[i];
			half val(ipt_vector_f32[i]);

			//Converting to Big endian
			ipt_vector_f16[i] = 0;
			memcpy(((void*)(ipt_vector_f16 + i)) + 2, ((void*)&val) + 1, 1);
			memcpy(((void*)(ipt_vector_f16 + i)) + 3, ((void*)&val) + 0, 1);

			opt_vector_f32[i] = 0;
		}
	}
	catch(ifstream::failure e)
	{
		if(ipt_file.rdstate() & ifstream::eofbit)
		{
			cerr << "Stream handler met EOF earlier than expected\n";
			cerr << "Please check length of your file and try again\n";
		}
		cerr << e.what() << endl;
		return -1;
	}

	
	cout<<"Baseline start"<<endl;
	//f32 * f32 calculation(Baseline)
	for(size_t i = -1 ; ++i < matrix_size * matrix_size ; opt_vector_f32[i >> SIZE_SHIFTER]
		+= ipt_matrix_f32[i] * ipt_vector_f32[i & ((1 << SIZE_SHIFTER) - 1)]);
	cout<<"Baseline end"<<endl;
	cout<<"FPGA start"<<endl;
	//Your FPGA code
	float *your_vector_f32 = new float[matrix_size];
	double t_you = fpga_calculate(ipt_matrix_f16, ipt_vector_f16, your_vector_f32);
	cout<<"FPGA end"<<endl;
	cout<<"ARM start"<<endl;
	//ZEDboard CPU code
	float *arm_vector_f32 = new float[matrix_size];
	double t_arm = arm_calculate(ipt_matrix_f16, ipt_vector_f16, arm_vector_f32);
	cout<<"ARM end"<<endl;
	cout<<"Print start"<<endl;
	//print
	cout 	<< "(baseline) (CPU code) (your code)\n";
	for(size_t i = 0 ; i < matrix_size ; i++)
	{
		double arm_error = (arm_vector_f32[i] - opt_vector_f32[i]) / opt_vector_f32[i];
		double your_error = (your_vector_f32[i] - opt_vector_f32[i]) / opt_vector_f32[i];

		cout 	<< opt_vector_f32[i] << ", "
			<< arm_vector_f32[i] << "(" << arm_error * 100 << "%), "
			<< your_vector_f32[i] << "(" << your_error * 100 << "%)\n";

		if(your_error > ERROR_THRESHOLD || your_error < -ERROR_THRESHOLD)
		{
			cerr	<< "Your result exceeded error threshold("
				<< ERROR_THRESHOLD * 100 << "%)\n";
		}
	}

	cout << "Elapsed time(Your IP) : " << t_you << endl;
	cout << "Elapsed time(ARM CPU) : " << t_arm << endl;
	cout<<"Print end"<<endl;
	//cleanup
	delete [] your_vector_f32;
	delete [] arm_vector_f32;
	
	delete [] ipt_matrix_f16;
	delete [] ipt_vector_f16;
	
	delete [] ipt_matrix_f32;
	delete [] ipt_vector_f32;
	delete [] opt_vector_f32;

}

