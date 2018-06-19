#include <stdio.h>
#include <stdlib.h>

#define SIZE 64
//#define DEBUG

typedef union {
  float f;
  unsigned int i;
}foo;

int main(int argc, char** argv)
{
  int i, j;
  foo container;

  float input_a[SIZE];
  float input_b[SIZE][SIZE];
  float output;

  // initialization

  for (i = 0; i < SIZE; i++){
	  do{
		  //input_a[i] = ((float)(rand() % 1000))/(float)(RAND_MAX);
		  input_a[i] = ((float)(rand() % 1000)) / 100.0f;
	  }while(input_a[i] < 0.3f);
  }
  for (i = 0; i < SIZE; i++){
	  for (j = 0; j < SIZE; j++){
		  //input_b[i][j] = (float)rand()/(float)(RAND_MAX);
		  do{
			  input_b[i][j] = ((float)(rand() % 1000)) / 100.0f;
		  }while(input_b[i][j] < 0.3f);
	  }
  }

#ifdef DEBUG
  FILE* log;
  log = fopen("debug.log", "w");
  for (i = 0; i < SIZE; i++)
  {
	  output = 0.0f;
	  for (j = 0; j < SIZE; j++){
		  printf("%5d  ", SIZE*i+j);
		  fprintf(log, "%5d  ", SIZE*i+j);
		  output += input_a[SIZE-1-j] * input_b[SIZE-1-i][SIZE-1-j];
		  container.f = input_a[SIZE-1-j];
		  printf(" (%f %X) ", container.f, container.i);
		  fprintf(log, " (%f %X) ", container.f, container.i);
		  container.f = input_b[SIZE-1-i][SIZE-1-j];
		  printf(" (%f %X) ", container.f, container.i);
		  fprintf(log, " (%f %X) ", container.f, container.i);
		  container.f = output;
		  printf("partial_output: (%f %X)\n", container.f, container.i);
		  fprintf(log, "partial_output: (%f %X)\n", container.f, container.i);
	  }
  }
  fclose(log);
#endif

  output = 0.0f;
  // V*V = scalar
  for (i = 0; i < SIZE; i++){
	  for (j = 0; j < SIZE; j++)
    	output += input_a[SIZE-1-i] * input_b[SIZE-1-i][SIZE-1-j];
  }

  // shape input txt
  FILE *fd;
  fd = fopen("input.txt", "w");

  for (i = 0; i < SIZE; i++)
  {
	  for (j = 0; j < SIZE; j++){
    	container.f = input_b[i][j];
    	fprintf(fd, "%X\n", container.i);
	  }
  }
  for (i = 0; i < SIZE; i++)
  {
    container.f = input_a[i];
    fprintf(fd, "%X\n", container.i);
  }
  fclose(fd);

  // shape output txt
  fd = fopen("output.txt", "w");
  container.f = output;
  fprintf(fd, "%X\n", container.i);
  fclose(fd);

  return 0;
}
