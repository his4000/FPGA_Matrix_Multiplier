#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/time.h>

#define INPUT_SIZE (64 * 64 + 64)

int main(int argc, char **argv)
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	srand(tv.tv_usec);

	FILE *ofp = fopen("input.txt", "w");

	for(int i = 0 ; i < INPUT_SIZE ; i++)
	{
		unsigned int a;
		do
		{
			a = rand() % 0x7FF;
		}
		while(a < 0x40);

		fprintf(ofp, "%u\n", a);
	}

	fclose(ofp);

	return 0;

}
