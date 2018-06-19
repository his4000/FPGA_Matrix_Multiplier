#include "zynq.h"
#include <fstream>
#include <cstring>
#include <sys/time.h>
#include <stdlib.h>
#include <unistd.h>

#include <cstdio>	//for perror
#include <sys/mman.h>	//mmap

#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>

#define BRAM_BASE 0x40000000

#define BRAM_BASE_0	(BRAM_BASE)
#define BRAM_BASE_1	(BRAM_BASE) + 0x1000
#define BRAM_BASE_2	(BRAM_BASE) + 0x2000
#define BRAM_BASE_3	(BRAM_BASE) + 0x3000
#define	VECTOR_BASE	(BRAM_BASE) + 0x4000

#define MATRIX_ADDR BRAM_BASE
#define IPT_VECTOR_ADDR (MATRIX_ADDR + (MATRIX_SIZE * MATRIX_SIZE * sizeof(uint32_t)))
#define OPT_VECTOR_ADDR (IPT_VECTOR_ADDR + MATRIX_SIZE * sizeof(uint32_t))

#define INSTRUCTION_ADDR 0x43C00000
#define MAGIC_CODE 0x5555

#define MATRIX_SCALE	(VECTOR_SIZE) * (VECTOR_SIZE)
#define VECTOR_SIZE		64
#define OUTPUT_SIZE		64
#define	MATRIX_SEG_NUM	4		// Number of pages in matrix
#define MATRIX_SEG_SIZE	4096	// Byte size of each page
#define	VECTOR_SEG_SIZE	512		// Byte size of Vector(input & output)						
#define MATRIX_SEG_CNT	1024	// Number of float variables in one page

#define BRAM_SIZE		(MATRIX_SCALE) + (VECTOR_SIZE) + (OUTPUT_SIZE)

static inline float f16_to_f32(const uint32_t *input);

double fpga_calculate(uint32_t *ipt_matrix_f16, uint32_t *ipt_vector_f16, float *your_vector_f32)
{
	//Map BRAM to virtual memory space and copy data.

	int i, j;
  	int foo;
  	foo=open("/dev/mem", O_RDWR | O_NONBLOCK);

	if(foo == -1){
		std::cout<<"open error"<<std::endl<<"If you cannot use sudo, you have to use it and retry."<<std::endl;
		exit(-1);
	}

	uint32_t* matrix_bram[MATRIX_SEG_NUM];
	uint32_t* vector_bram = (uint32_t*)mmap(NULL, VECTOR_SEG_SIZE, PROT_WRITE, MAP_SHARED, foo, VECTOR_BASE);
	matrix_bram[0] = (uint32_t*)mmap(NULL, MATRIX_SEG_SIZE, PROT_WRITE, MAP_SHARED, foo, BRAM_BASE_0);
	matrix_bram[1] = (uint32_t*)mmap(NULL, MATRIX_SEG_SIZE, PROT_WRITE, MAP_SHARED, foo, BRAM_BASE_1);
	matrix_bram[2] = (uint32_t*)mmap(NULL, MATRIX_SEG_SIZE, PROT_WRITE, MAP_SHARED, foo, BRAM_BASE_2);
	matrix_bram[3] = (uint32_t*)mmap(NULL, MATRIX_SEG_SIZE, PROT_WRITE, MAP_SHARED, foo, BRAM_BASE_3);

	// Input Matrix in BRAM
	for(i=0;i<MATRIX_SEG_NUM;i++){
		for(j=0;j<MATRIX_SEG_CNT;j++)
			memcpy(matrix_bram[i] + j, ipt_matrix_f16 + j + i * MATRIX_SEG_CNT, sizeof(uint32_t));
	}
	// Input vector in BRAM
	for(i=0;i<VECTOR_SIZE;i++)
		memcpy(vector_bram + i, ipt_vector_f16 + i, sizeof(uint32_t));

	volatile unsigned int* fpga_ip = (volatile unsigned int*)mmap(NULL, sizeof(unsigned int), PROT_WRITE, MAP_SHARED, foo, INSTRUCTION_ADDR);

	struct timeval start, end;
	gettimeofday(&start, NULL);

	//Run IP and copy value to DRAM space
	(*fpga_ip)=MAGIC_CODE;
	while((*fpga_ip)==MAGIC_CODE);

 	for(i=0;i<OUTPUT_SIZE;i++)	
		*(your_vector_f32 + i) = *((float*)(vector_bram + i + (VECTOR_SIZE)));

	gettimeofday(&end, NULL);

 	//Cleanup allocated resources (optional).
	close(foo);
	for(i=0;i<MATRIX_SEG_NUM;i++)
		munmap((void*)matrix_bram[i], MATRIX_SEG_SIZE);

	return (double)(end.tv_sec - start.tv_sec) + ((double)(end.tv_usec - start.tv_usec)) / 1000000.0;
}


double arm_calculate(uint32_t *ipt_matrix_f16, uint32_t *ipt_vector_f16, float *arm_vector_f32)
{
	struct timeval start, end;
	gettimeofday(&start, NULL);

	memset(arm_vector_f32, 0, sizeof(float) << SIZE_SHIFTER);

	for(size_t i = -1 ; ++i < matrix_size * matrix_size ; arm_vector_f32[i >> SIZE_SHIFTER]
		+= f16_to_f32(ipt_matrix_f16 + i) * f16_to_f32(ipt_vector_f16 + (i & ((1 << SIZE_SHIFTER) - 1))));

	gettimeofday(&end, NULL);
	return (double)(end.tv_sec - start.tv_sec) + ((double)(end.tv_usec - start.tv_usec)) / 1000000.0;
}

static inline float f16_to_f32(const uint32_t *input)
{
	char *iptc = (char*)input + 2;

	uint32_t half_precision = 0;
	memcpy(&half_precision, iptc + 1, 1);
	memcpy(((void*)&half_precision) + 1, iptc, 1);

	uint32_t opt 	= ((half_precision & 0x8000) << 16)
			| ((((half_precision & 0x7C00) >> 10) + 112) << 23)
			| ((half_precision & 0x3FF) << 13);

	float casted_opt;
	memcpy(&casted_opt, &opt, sizeof(float));
	return casted_opt;
}
