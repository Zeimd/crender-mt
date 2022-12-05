
#include <algorithm>

#include <stdlib.h>
#include <string.h>

static int float_compare(const void* a, const void* b)
{
	float* x = (float*)a;
	float* y = (float*)b;

	if (*x < *y)
	{
		return -1;
	}
	else if (*x > *y)
	{
		return 1;
	}

	return 0;
}

void float_sort4_insertion_sort(float* input4, const int size)
{
	for (int group = 0; group < size; group += 4)
	{
		for (int i = 1; i < 4; i++)
		{
			float x = input4[group + i];

			int j = i - 1;

			while (j >= 0 && input4[group + j] > x)
			{
				input4[group + j + 1] = input4[group + j];
				--j;
			};

			input4[group + j + 1] = x;
		}
	}
}

void float_sort4_selection_sort(float* input4, const int size)
{
	for (int group = 0; group < size; group += 4)
	{
		for (int i = 0; i < 4; i++)
		{
			int smallIndex = i;
			float smallVal = input4[group + i];

			for (int j = smallIndex+1; j < 4; j++)
			{
				float val = input4[group + j];

				if (val < smallVal)
				{
					smallIndex = j;
					smallVal = val;
				}
			}

			float temp = input4[group + i];
			input4[group + i] = smallVal;
			input4[group + smallIndex] = temp;
		}		
	}
}


void float_sort4_stdlib_qsort(float* input4, const int size)
{
	for (int k = 0; k < size; k += 4)
	{
		qsort(&input4[k], 4, sizeof(float), &float_compare);
	}
}

void float_sort4_minmax_stl(float* input4, const int size)
{
	for (int group = 0; group < size; group += 4)
	{
		float a = input4[group];
		float b = input4[group + 1];
		float c = input4[group + 2];
		float d = input4[group + 3];

		float t0 = std::min(a, b);
		float t1 = std::min(c, d);
		float t2 = std::max(a, b);
		float t3 = std::max(c, d);

		float t4 = std::max(t0, t1);
		float t5 = std::min(t2, t3);

		input4[group] = std::min(t0, t1);
		input4[group + 1] = std::min(t4, t5);
		input4[group + 2] = std::max(t4, t5);
		input4[group + 3] = std::max(t2, t3);
	}
}

void float_sort4_minmax_stl_int_punning(float* input4, const int size)
{
	for (int group = 0; group < size; group += 4)
	{
		int *a = (int*)&input4[group];
		int *b = (int*)&input4[group + 1];
		int *c = (int*)&input4[group + 2];
		int *d = (int*)&input4[group + 3];

		int t0 = std::min(*a, *b);
		int t1 = std::min(*c, *d);

		int t2 = std::max(*a, *b);
		int t3 = std::max(*c, *d);

		int t4 = std::max(t0, t1);
		int t5 = std::min(t2, t3);

		*a = std::min(t0, t1);
		*b = std::min(t4, t5);
		*c = std::max(t4, t5);
		*d = std::max(t2, t3);
	}
}

// Vectorized version of minmax sort
void float_sort4_sse(float* input4, const int size)
{
	for (int k = 0; k < size; k += 4)
	{
		__asm
		{
			mov esi, input4;
			mov eax, k;
			shl eax, 2;

			vmovdqa xmm0, [esi + eax];

			vmovdqa xmm1, xmm0;

			vpsrldq xmm1, xmm1, 8;

			vmovaps xmm2, xmm0;
			vminps xmm0, xmm0, xmm1;
			vmaxps xmm2, xmm2, xmm1;

			vmovaps xmm1, xmm0;
			vunpcklps xmm1, xmm1, xmm2;
			
			vpshufd xmm3, xmm1, 01001110b;

			vmovaps xmm0, xmm1;
			vminps xmm0, xmm0, xmm3;

			vmaxps xmm3, xmm3, xmm1;
			
			vmovaps xmm2, xmm0;

			vpsrldq xmm2, xmm2, 4;

			vmovaps xmm1, xmm2;
			
			vminps xmm1, xmm1, xmm3;
			vmaxps xmm2, xmm2, xmm3;

			vshufps xmm1, xmm1, xmm2, 01000001b;

			vmovaps[esi + eax], xmm1;			
		}
	}
}