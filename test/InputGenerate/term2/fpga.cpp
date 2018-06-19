#include "fpga.h"

#include <cstdio>
#include <cstring>

#include <fstream>

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <sys/mman.h>

#define	ERR_COR	0x800

//you can add variables in here too (It is recommanded to modify fpga_tb class's private field).

void fpga_tb::fpga_allocate_resources()
{
	//allocate resource.
	ipt_matrix_fix8 = new uint8_t[MATRIX_SIZE * MATRIX_SIZE];
	ipt_vector_fix8 = new uint8_t[MATRIX_SIZE];
}

void fpga_tb::fpga_load_execute_and_copy(const uint16_t *ipt_matrix_fix16, const uint16_t *ipt_vector_fix16, uint32_t *your_vector_fix32)
{
	fpga_trans_fix16_to_fix8(ipt_matrix_fix16, ipt_vector_fix16);
	//Copy data to BRAM(or you may use DMA).
	uint32_t* finput_matrix = new uint32_t[MATRIX_SIZE*MATRIX_SIZE/sizeof(uint32_t)];
	uint32_t* finput_vector = new uint32_t[MATRIX_SIZE/sizeof(uint32_t)];

	memcpy(finput_matrix, ipt_matrix_fix8, MATRIX_SIZE*MATRIX_SIZE*sizeof(uint8_t));
	memcpy(finput_vector, ipt_vector_fix8, MATRIX_SIZE*sizeof(uint8_t));

	using namespace std;
	ofstream simulInputFile;
	simulInputFile.open("simulInput.txt");
	FILE* log;
	log = fopen("Input.log", "w");
	FILE* input16;
	input16 = fopen("Input16.log", "w");
	int i, j;

	for(i=0;i<MATRIX_SIZE*MATRIX_SIZE;i++)
		fprintf(input16, "%d: %X\n", i, *(ipt_matrix_fix16+i));
	for(i=0;i<MATRIX_SIZE;i++)
		fprintf(input16, "%d: %X\n", i, *(ipt_vector_fix16+i));

	for(i=0;i<MATRIX_SIZE*MATRIX_SIZE/4;i++){
		fprintf(log, "%d: %X\n", 4*i, *(ipt_matrix_fix8+4*i));
		fprintf(log, "%d: %X\n", 4*i+1, *(ipt_matrix_fix8+4*i+1));
		fprintf(log, "%d: %X\n", 4*i+2, *(ipt_matrix_fix8+4*i+2));
		fprintf(log, "%d: %X\n", 4*i+3, *(ipt_matrix_fix8+4*i+3));
		simulInputFile << std::hex << *(finput_matrix+i) << endl;
	}
	for(i=0;i<MATRIX_SIZE/4;i++){
		fprintf(log, "%d: %X\n", 4*i, *(ipt_vector_fix8+4*i));
		fprintf(log, "%d: %X\n", 4*i+1, *(ipt_vector_fix8+4*i+1));
		fprintf(log, "%d: %X\n", 4*i+2, *(ipt_vector_fix8+4*i+2));
		fprintf(log, "%d: %X\n", 4*i+3, *(ipt_vector_fix8+4*i+3));
		simulInputFile << std::hex << *(finput_vector+i) << endl;
	}

	//Execute your magic code.

	uint8_t ipt_matrix_fix8[MATRIX_SIZE*MATRIX_SIZE] = {0};
	uint8_t ipt_vector_fix8[MATRIX_SIZE] = {0};

	for(i=0;i<MATRIX_SIZE;i++)
		ipt_vector_fix8[i] = ipt_vector_fix16[i] >> 3;
	for(i=0;i<MATRIX_SIZE*MATRIX_SIZE;i++)
		ipt_matrix_fix8[i] = ipt_matrix_fix16[i] >> 3;

	for(i=0;i<MATRIX_SIZE;i++){
		for(j=0;j<MATRIX_SIZE;j++)
			your_vector_fix32[i] += (ipt_vector_fix8[j] * ipt_matrix_fix8[i*MATRIX_SIZE+j]) >> 2;
		your_vector_fix32[i] += ERR_COR;
	}

	//and copy BRAM's data to DRAM space.
	
	simulInputFile.close();
	fclose(log);
	fclose(input16);

	delete [] finput_matrix;
	delete [] finput_vector;
}

void fpga_tb::fpga_cleanup()
{
	//Cleanup allocated resources.
	delete [] ipt_matrix_fix8;
	delete [] ipt_vector_fix8;
}

// I use 8 bits precision to improve performance by decreasing off-chip load data width
void fpga_tb::fpga_trans_fix16_to_fix8(const uint16_t* ipt_matrix_fix16, const uint16_t* ipt_vector_fix16){
	for(int i=0;i<MATRIX_SIZE;i++){
		*(ipt_vector_fix8 + i) = (uint8_t)((*(ipt_vector_fix16 + i)) >> 3);
		for(int j=0;j<MATRIX_SIZE;j++)
			*(ipt_matrix_fix8 + i*MATRIX_SIZE + j) = (uint8_t)((*(ipt_matrix_fix16 + i*MATRIX_SIZE + j)) >> 3);
	}
}
