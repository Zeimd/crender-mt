
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


void float_sort4_stdlib_qsort(float* input4, const int size)
{
	for (int k = 0; k < size; k += 4)
	{
		qsort(&input4[k], 4, sizeof(float), &float_compare);
	}
}


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