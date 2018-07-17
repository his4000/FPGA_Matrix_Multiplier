#include "fpga.h"

#include <cstdio>
#include <cstring>

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <sys/time.h>

#include <sys/mman.h>

#define BRAM_BASE_ADDR		0x40000000
#define CDMA_ADDR			0x7E200000
#define NON_CACHEABLE_ADDR	0x10000000
#define INSTRUCTION_ADDR	0x43C00000

#define MATRIX_ADDR 		BRAM_BASE_ADDR
#define IPT_VECTOR_ADDR (MATRIX_ADDR + (MATRIX_SIZE * MATRIX_SIZE * sizeof(uint16_t)))
#define OPT_VECTOR_ADDR (IPT_VECTOR_ADDR + MATRIX_SIZE * sizeof(uint16_t))
#define BRAM_IPT_WORD_CNT (MATRIX_SIZE) / sizeof(uint32_t)  // the number of word(4Byte) in input vector area

#define DMA_SRC_OFFSET 	6
#define DMA_DEST_OFFSET 8
#define DMA_SIZE_OFFSET 10

#define MAGIC_CODE 0x5555

//you can add variables in here too (It is recommanded to modify fpga_tb class's private field).

void fpga_tb::fpga_allocate_resources()
{
	//allocate resource.
	ipt_matrix_fix8 = new uint8_t[MATRIX_SIZE * MATRIX_SIZE];
	ipt_vector_fix8 = new uint8_t[MATRIX_SIZE];

	foo = open("/dev/mem", O_RDWR);
	fpga_ip = (volatile unsigned int*)mmap(NULL, sizeof(unsigned int), PROT_READ | PROT_WRITE, MAP_SHARED, foo, INSTRUCTION_ADDR);
#ifdef	DMA
	fpga_dma = (volatile unsigned int*)mmap(NULL, 16*sizeof(unsigned int), PROT_READ | PROT_WRITE, MAP_SHARED, foo, CDMA_ADDR);
	dram_matrix = (uint8_t*)mmap(NULL, MATRIX_SIZE*MATRIX_SIZE*sizeof(uint8_t), PROT_READ | PROT_WRITE, MAP_SHARED, foo, NON_CACHEABLE_ADDR);
	dram_vector = (uint8_t*)mmap(NULL, MATRIX_SIZE*sizeof(uint8_t), PROT_READ | PROT_WRITE, MAP_SHARED, foo, NON_CACHEABLE_ADDR + 0x1000);
#else
	fpga_bram = (uint8_t*)mmap(NULL, MATRIX_SIZE*MATRIX_SIZE*sizeof(uint8_t), PROT_READ | PROT_WRITE, MAP_SHARED, foo, BRAM_BASE_ADDR);
#endif
	fpga_bram_vector_io = (uint32_t*)mmap(NULL, MATRIX_SIZE*(sizeof(uint8_t) + sizeof(uint32_t)), PROT_READ | PROT_WRITE, MAP_SHARED, foo, BRAM_BASE_ADDR + 0x1000);
}

void fpga_tb::fpga_load_execute_and_copy(const uint16_t *ipt_matrix_fix16, const uint16_t *ipt_vector_fix16, uint32_t *your_vector_fix32)
{
	/* For time checking */
#ifdef ESTIMATE
	struct timeval start, end;
	double fpga_time;

	gettimeofday(&start, NULL);
#endif
	fpga_trans_fix16_to_fix8(ipt_matrix_fix16, ipt_vector_fix16);
#ifdef ESTIMATE
	gettimeofday(&end, NULL);
	fpga_time = (double)(end.tv_sec - start.tv_sec) + ((double)(end.tv_usec - start.tv_usec)) / 1000000.0;
	printf("Time for FPGA trans fix16 to fix8 : %lf\n", fpga_time);

	gettimeofday(&start, NULL);
#endif
#ifdef	DMA
	//Copy data to non-cacheable area
	memcpy(dram_matrix, ipt_matrix_fix8, MATRIX_SIZE*MATRIX_SIZE*sizeof(uint8_t));
	memcpy(dram_vector, ipt_vector_fix8, MATRIX_SIZE*sizeof(uint8_t));
#else
	//Copy data to BRAM
	memcpy(fpga_bram, ipt_matrix_fix8, MATRIX_SIZE*MATRIX_SIZE*sizeof(uint8_t));
	memcpy(fpga_bram_vector_io, ipt_vector_fix8, MATRIX_SIZE*sizeof(uint8_t));
#endif
#ifdef ESTIMATE
	gettimeofday(&end, NULL);
	fpga_time = (double)(end.tv_sec - start.tv_sec) + ((double)(end.tv_usec - start.tv_usec)) / 1000000.0;
#endif
#if	defined(ESTIMATE) && defined(DMA)
	printf("Time memcpy to NON-CACHEABLE AREA : %lf\n", fpga_time);
#elif defined(ESTIMATE)
	printf("Time memcpy to BRAM : %lf\n", fpga_time);
#endif
#if defined(ESTIMATE) && defined(DMA)
	gettimeofday(&start, NULL);
#endif
#ifdef	DMA
	*(fpga_dma+DMA_SRC_OFFSET) = NON_CACHEABLE_ADDR;
	*(fpga_dma+DMA_DEST_OFFSET) = BRAM_BASE_ADDR;
	*(fpga_dma+DMA_SIZE_OFFSET) = sizeof(uint8_t) * (MATRIX_SIZE * MATRIX_SIZE + MATRIX_SIZE);

	while((*(fpga_dma+1) & 0x00000002) == 0);
#endif
#if defined(ESTIMATE) && defined(DMA)
	gettimeofday(&end, NULL);
	fpga_time = (double)(end.tv_sec - start.tv_sec) + ((double)(end.tv_usec - start.tv_usec)) / 1000000.0;
	printf("Time for DMA : %lf\n", fpga_time);
#endif

	//Execute your magic code.
	*fpga_ip = MAGIC_CODE;

#ifdef ESTIMATE
	gettimeofday(&start, NULL);
#endif
	while(*(fpga_ip) == MAGIC_CODE);
#ifdef ESTIMATE
	gettimeofday(&end, NULL);
	fpga_time = (double)(end.tv_sec - start.tv_sec) + ((double)(end.tv_usec - start.tv_usec)) / 1000000.0;
	printf("Time for FPGA running time : %lf\n", fpga_time);
	gettimeofday(&start, NULL);
#endif
	//and copy BRAM's data to DRAM space.
	for(int i=0;i<MATRIX_SIZE;i++)
		*(your_vector_fix32+i) = *((uint32_t*)(fpga_bram_vector_io + BRAM_IPT_WORD_CNT + i));
#ifdef ESTIMATE
	gettimeofday(&end, NULL);
	fpga_time = (double)(end.tv_sec - start.tv_sec) + ((double)(end.tv_usec - start.tv_usec)) / 1000000.0;
	printf("Time for transfer output : %lf\n", fpga_time);
#endif
}

void fpga_tb::fpga_cleanup()
{
	//Cleanup allocated resources.
#ifdef	DMA
	munmap(dram_vector, MATRIX_SIZE*sizeof(uint8_t));
	munmap(dram_matrix, MATRIX_SIZE*MATRIX_SIZE*sizeof(uint8_t));
	munmap((void*)fpga_dma, 16*sizeof(unsigned int));
#else
	munmap(fpga_bram_vector_io, MATRIX_SIZE*(sizeof(uint8_t)+sizeof(uint32_t)));
	munmap(fpga_bram, MATRIX_SIZE*MATRIX_SIZE*sizeof(uint8_t));
#endif
	munmap((void*)fpga_ip, sizeof(unsigned int));
	close(foo);

	delete [] ipt_matrix_fix8;
	delete [] ipt_vector_fix8;
}

// I use 8 bits precision to improve performance by decreasing off-chip load data width
inline void fpga_tb::fpga_trans_fix16_to_fix8(const uint16_t* ipt_matrix_fix16, const uint16_t* ipt_vector_fix16){
	for(int i=0;i<MATRIX_SIZE;i++)
		*(ipt_vector_fix8 + i) = (uint8_t)((*(ipt_vector_fix16 + i)) >> 3);
	for(int j=0;j<MATRIX_SIZE*MATRIX_SIZE;j++)
		*(ipt_matrix_fix8 + j) = (uint8_t)((*(ipt_matrix_fix16 + j)) >> 3);
}
