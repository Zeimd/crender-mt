/*****************************************************************************
*
* alg-test.cpp
*
* By Jari Korkala 5/3/2016
*
* For purposes of testing SIMD algorithms.
*
*****************************************************************************/

#include <iostream>
#include <cmath>
#include <cstdlib>

#include <timerlib.h>
#include <ceng/datatypes/aligned-buffer.h>

void normalized_float_to_uint8(const float *input, unsigned char *output,const int size);

void normalized_float_to_uint8_sse(const float *input, unsigned char *output, const int size);

void normalized_float_to_uint8_optimized_sse(const float *input, unsigned char *output, const int size);

void normalized_float_to_uint8_avx(const float *input, unsigned char *output, const int size);

void uint8_to_normalized_float(const unsigned char *input, float *output,const int size);

void uint8_to_normalized_float_strip4(const unsigned char *input, float *output, const int size);

void uint8_to_normalized_float_strip8(const unsigned char *input, float *output, const int size);

void uint8_to_normalized_float_strip16(const unsigned char *input, float *output, const int size);

void uint8_to_normalized_float_sse(const unsigned char *input16,float *output16,const int size);

void uint8_to_normalized_float_optimized_sse(const unsigned char *input16, float *output16,const int size);

void uint8_to_normalized_float_sse_v3(const unsigned char *input16, float *output16, const int size);

void uint8_to_normalized_float_avx(const unsigned char *input16, float *output16, const int size);

void round_trip_test(const unsigned char *input);

void accuracy_test(float *exact, float *test, const int size);

int main()
{
	_declspec(align(32)) unsigned char input_sse[256];

	for (int k = 0; k < 256; ++k)
	{
		input_sse[k] = unsigned char(k);
	}

	float results[256];

	std::cout << "C version:" << std::endl;

	uint8_to_normalized_float(input_sse,results,256);

	_declspec(align(32)) unsigned char round_trip[256];

	normalized_float_to_uint8(results, round_trip,256);

	//round_trip_test(round_trip);

	_declspec(align(32)) unsigned char sse_round_trip[256];

	// SSE default version

	/*
	

	std::cout << "default sse version:" << std::endl;

	_declspec(align(16)) float results_default_sse[256];

	uint8_to_normalized_float_sse(input_sse, results_default_sse,256);

	//accuracy_test(results, results_default_sse,256);

	normalized_float_to_uint8_sse(results_default_sse, sse_round_trip, 256);

	//round_trip_test(sse_round_trip);

	std::cout << "*******************************************************" << std::endl;
	std::cout << "optimized sse version:" << std::endl;

	_declspec(align(16)) float results_optimized_sse[256];

	uint8_to_normalized_float_optimized_sse(input_sse, results_optimized_sse, 256);

	//accuracy_test(results, results_optimized_sse, 256);

	normalized_float_to_uint8_optimized_sse(results_optimized_sse, sse_round_trip, 256);
	*/
	//round_trip_test(sse_round_trip);

	_declspec(align(32)) float results_avx[256];

	/*
	uint8_to_normalized_float_avx(input_sse, results_avx, 256);

	accuracy_test(results, results_avx, 256);

	normalized_float_to_uint8_avx(results_avx, sse_round_trip, 256);
	
	round_trip_test(sse_round_trip);
	*/
	//return 0;

	double start_time, end_time;
	double duration;
	double base_duration;

	const int test_size = 16 * 10000000;

	float *speed_input = (float*)Ceng::AlignedMalloc(test_size*sizeof(float),64);
	
	unsigned char *speed_result = (unsigned char*)Ceng::AlignedMalloc(test_size*sizeof(unsigned char),64);

	std::cout << "randomizing input" << std::endl;
	for (int k = 0; k < test_size; ++k)
	{
		speed_input[k] = (rand() % 257) / 256.0f;
		speed_result[k] = 0;
	}

	std::cout << "*******************************************************" << std::endl;
	std::cout << "speed test: normalized float -> uint8" << std::endl;

	start_time = Timer();

	normalized_float_to_uint8(speed_input, speed_result, test_size);

	end_time = Timer();

	base_duration = end_time - start_time;

	std::cout << "scalar C : " << end_time - start_time << std::endl;

	start_time = Timer();

	normalized_float_to_uint8_sse(speed_input, speed_result, test_size);

	end_time = Timer();

	duration = end_time - start_time;

	std::cout << "SSE v1: " << duration << " , ratio = " << duration / base_duration << std::endl;

	start_time = Timer();

	normalized_float_to_uint8_optimized_sse(speed_input, speed_result, test_size);

	end_time = Timer();

	duration = end_time - start_time;

	std::cout << "SSE v2: " << duration << " , ratio = " << duration / base_duration << std::endl;

	start_time = Timer();

	normalized_float_to_uint8_avx(speed_input, speed_result, test_size);

	end_time = Timer();

	duration = end_time - start_time;

	std::cout << "AVX: " << duration << " , ratio = " << duration / base_duration << std::endl;

	std::cout << "*******************************************************" << std::endl;
	std::cout << "speed test: uint8 -> normalized float" << std::endl;

	start_time = Timer();

	uint8_to_normalized_float(speed_result,speed_input,test_size);

	end_time = Timer();

	base_duration = end_time - start_time;

	std::cout << "scalar C : " << end_time - start_time << std::endl;

	start_time = Timer();

	uint8_to_normalized_float_strip4(speed_result, speed_input, test_size);

	end_time = Timer();

	duration = end_time - start_time;

	std::cout << "scalar , 4x strip : " << duration << " , ratio = " << duration / base_duration << std::endl;

	start_time = Timer();

	uint8_to_normalized_float_strip8(speed_result, speed_input, test_size);

	end_time = Timer();

	duration = end_time - start_time;

	std::cout << "scalar , 8x strip : " << duration << " , ratio = " << duration / base_duration << std::endl;

	start_time = Timer();

	uint8_to_normalized_float_strip16(speed_result, speed_input, test_size);

	end_time = Timer();

	duration = end_time - start_time;

	std::cout << "scalar , 16x strip : " << duration << " , ratio = " << duration / base_duration << std::endl;

	start_time = Timer();

	uint8_to_normalized_float_sse(speed_result, speed_input, test_size);

	end_time = Timer();

	duration = end_time - start_time;

	std::cout << "SSE v1 : " << duration << " , ratio = " << duration / base_duration << std::endl;

	start_time = Timer();

	uint8_to_normalized_float_optimized_sse(speed_result, speed_input, test_size);

	end_time = Timer();

	duration = end_time - start_time;

	std::cout << "SSE v2 : " << duration << " , ratio = " << duration / base_duration << std::endl;

	start_time = Timer();

	uint8_to_normalized_float_sse_v3(speed_result, speed_input, test_size);

	end_time = Timer();

	duration = end_time - start_time;

	std::cout << "SSE v3 : " << duration << " , ratio = " << duration / base_duration << std::endl;

	start_time = Timer();

	uint8_to_normalized_float_avx(speed_result, speed_input, test_size);

	end_time = Timer();

	duration = end_time - start_time;

	std::cout << "AVX : " << duration << " , ratio = " << duration / base_duration << std::endl;

	Ceng::AlignedFree(speed_input);
	Ceng::AlignedFree(speed_result);

	//delete[] speed_input;
	//delete[] speed_result;

	return 0;
}

void accuracy_test(float *exact, float *test, const int size)
{
	for (int k = 0; k < size; ++k)
	{
		std::cout << "comparison at " << k << " : " << exact[k] << " , " << test[k]
			<< " | delta = " << std::fabs(exact[k] - test[k]) << std::endl;
	}
}

void round_trip_test(const unsigned char *input)
{
	std::cout << "round trip results:" << std::endl;

	for (int k = 0; k < 256; ++k)
	{
		if (input[k] != k)
		{
			std::cout << "k = " << k << " , round = " << int(input[k]) << std::endl;
		}
	}
}

const float scale = 1.0f / 255.0f;

void uint8_to_normalized_float(const unsigned char *input,float *output,const int size)
{
	for (int k = 0; k < size; ++k)
	{
		output[k] = float(input[k]) * scale;
	}
}

void uint8_to_normalized_float_strip4(const unsigned char *input, float *output, const int size)
{
	for (int k = 0; k < size; k+=4)
	{
		output[k] = float(input[k]) * scale;
		output[k+1] = float(input[k+1]) * scale;
		output[k+2] = float(input[k+2]) * scale;
		output[k+3] = float(input[k+3]) * scale;
	}
}

void uint8_to_normalized_float_strip8(const unsigned char *input, float *output, const int size)
{
	for (int k = 0; k < size; k += 8)
	{
		output[k] = float(input[k]) * scale;
		output[k + 1] = float(input[k + 1]) * scale;
		output[k + 2] = float(input[k + 2]) * scale;
		output[k + 3] = float(input[k + 3]) * scale;
		output[k + 4] = float(input[k + 4]) * scale;
		output[k + 5] = float(input[k + 5]) * scale;
		output[k + 6] = float(input[k + 6]) * scale;
		output[k + 7] = float(input[k + 7]) * scale;
	}
}

void uint8_to_normalized_float_strip16(const unsigned char *input, float *output, const int size)
{
	for (int k = 0; k < size; k += 16)
	{
		output[k] = float(input[k]) * scale;
		output[k + 1] = float(input[k + 1]) * scale;
		output[k + 2] = float(input[k + 2]) * scale;
		output[k + 3] = float(input[k + 3]) * scale;
		output[k + 4] = float(input[k + 4]) * scale;
		output[k + 5] = float(input[k + 5]) * scale;
		output[k + 6] = float(input[k + 6]) * scale;
		output[k + 7] = float(input[k + 7]) * scale;

		output[k + 8] = float(input[k + 8]) * scale;
		output[k + 9] = float(input[k + 9]) * scale;
		output[k + 10] = float(input[k + 10]) * scale;
		output[k + 11] = float(input[k + 11]) * scale;
		output[k + 12] = float(input[k + 12]) * scale;
		output[k + 13] = float(input[k + 13]) * scale;
		output[k + 14] = float(input[k + 14]) * scale;
		output[k + 15] = float(input[k + 15]) * scale;
	}
}

void uint8_to_normalized_float_sse(const unsigned char *input16, float *output16, const int size)
{
	const _declspec(align(16)) float scaleVec[] = { scale, scale, scale, scale };

	__asm
	{
		vpxor xmm7, xmm7, xmm7; // xmm7 = zero
		vmovaps xmm6, scaleVec;
	}

	for (int k = 0; k < size; k += 16)
	{
		__asm
		{
			mov esi, input16;
			mov eax, k;

			vmovdqa xmm0, [esi + eax]; // xmm0 = byte {x15,x14,x13,...,x3,x2,x1,x0}

			vpunpcklbw xmm1, xmm0, xmm7; // xmm1 = word {x7,x6,x5,x4,x3,x2,x1,x0}
			vpunpckhbw xmm0, xmm0, xmm7; // xmm2 = word {x15,x14,x13,x12,x11,x10,x9,x8}

			vpunpcklwd xmm2, xmm1, xmm7; // xmm3 = dword{ x3, x2, x1, x0 }
			vpunpckhwd xmm3, xmm1, xmm7; // xmm4 = dword{ x7, x6, x5, x4 }

			vpunpcklwd xmm4, xmm0, xmm7; // xmm5 = dword{ x11, x10, x9, x8 }
			vpunpckhwd xmm5, xmm0, xmm7; // xmm6 = dword{ x15, x14, x13, x12 }

			// Convert to float
			vcvtdq2ps xmm2, xmm2;
			vcvtdq2ps xmm3, xmm3;
			vcvtdq2ps xmm4, xmm4;
			vcvtdq2ps xmm5, xmm5;

			// Scale correctly
			vmulps xmm2, xmm2, xmm6;
			vmulps xmm3, xmm3, xmm6;
			vmulps xmm4, xmm4, xmm6;
			vmulps xmm5, xmm5, xmm6;

			// Store

			mov edi, output16;
			mov eax, k;
			shl eax, 2;

			vmovaps[edi + eax], xmm2;
			vmovaps[edi + eax + 16], xmm3;
			vmovaps[edi + eax + 32], xmm4;
			vmovaps[edi + eax + 48], xmm5;
		}
	}
}

void uint8_to_normalized_float_optimized_sse(const unsigned char *input16, float *output16, const int size)
{
	const float floatVal = float(1 << 15);
	const _declspec(align(16)) float floatBits[] = { floatVal, floatVal, floatVal, floatVal };

	for (int k = 0; k < size; k += 16)
	{
		__asm
		{
			mov esi, input16;
			mov eax, k;

			vmovdqa xmm0, [esi + eax]; // xmm0 = byte {x15,x14,x13,...,x3,x2,x1,x0}

			vpxor xmm7, xmm7, xmm7; // xmm7 = zero

			vpunpcklbw xmm1, xmm0, xmm7; // xmm1 = word {x7,x6,x5,x4,x3,x2,x1,x0}
			vpunpckhbw xmm2, xmm0, xmm7; // xmm2 = word {x15,x14,x13,x12,x11,x10,x9,x8}

			// Scale so that 255 -> 1.0
			vpsrlw xmm3, xmm1, 7;
			vpaddusw xmm1, xmm1, xmm3;

			vpsrlw xmm4, xmm2, 7;
			vpaddusw xmm2, xmm2, xmm4;

			vpunpcklwd xmm3, xmm1, xmm7; // xmm3 = dword{ x3, x2, x1, x0 }
			vpunpckhwd xmm4, xmm1, xmm7; // xmm4 = dword{ x7, x6, x5, x4 }

			vpunpcklwd xmm5, xmm2, xmm7; // xmm5 = dword{ x11, x10, x9, x8 }
			vpunpckhwd xmm6, xmm2, xmm7; // xmm6 = dword{ x15, x14, x13, x12 }

			// Convert to float

			vmovdqa xmm7, floatBits;

			vpor xmm3, xmm3, xmm7;
			vpor xmm4, xmm4, xmm7;
			vpor xmm5, xmm5, xmm7;
			vpor xmm6, xmm6, xmm7;

			vsubps xmm3, xmm3, xmm7;
			vsubps xmm4, xmm4, xmm7;
			vsubps xmm5, xmm5, xmm7;
			vsubps xmm6, xmm6, xmm7;

			// Store

			mov edi, output16;
			mov eax, k;
			shl eax, 2;

			vmovaps[edi + eax], xmm3;
			vmovaps[edi + eax + 16], xmm4;
			vmovaps[edi + eax + 32], xmm5;
			vmovaps[edi + eax + 48], xmm6;
		}
	}
}

void uint8_to_normalized_float_sse_v3(const unsigned char *input16, float *output16, const int size)
{
	const _declspec(align(16)) float scaleVec[] = { scale, scale, scale, scale };

	__asm
	{
		vmovaps xmm7, scaleVec;
	}

	for (int k = 0; k < size; k += 16)
	{
		__asm
		{
			mov esi, input16;
			mov eax, k;

			vpmovzxbd xmm3, [esi + eax];
			vpmovzxbd xmm4, [esi + eax + 4];
			vpmovzxbd xmm5, [esi + eax + 8];
			vpmovzxbd xmm6, [esi + eax + 12];

			// Convert to float
			vcvtdq2ps xmm3, xmm3;
			vcvtdq2ps xmm4, xmm4;
			vcvtdq2ps xmm5, xmm5;
			vcvtdq2ps xmm6, xmm6;

			// Scale
			vmulps xmm3, xmm3, xmm7;
			vmulps xmm4, xmm4, xmm7;
			vmulps xmm5, xmm5, xmm7;
			vmulps xmm6, xmm6, xmm7;

			// Store

			mov edi, output16;
			//mov eax, k;
			shl eax, 2;

			vmovaps[edi + eax], xmm3;
			vmovaps[edi + eax + 16], xmm4;
			vmovaps[edi + eax + 32], xmm5;
			vmovaps[edi + eax + 48], xmm6;
		}
	}
}

void uint8_to_normalized_float_avx(const unsigned char *input, float *output, const int size)
{
	const _declspec(align(32)) float scaleVec[] = { scale, scale, scale, scale, scale, scale, scale, scale };

	__asm
	{
		vmovaps ymm7, scaleVec;
	}

	for (int k = 0; k < size; k += 16)
	{
		__asm
		{
			mov esi, input;
			mov eax, k;

			vpmovzxbd xmm3, [esi + eax];
			vpmovzxbd xmm4, [esi + eax + 4];
			vpmovzxbd xmm5, [esi + eax + 8];
			vpmovzxbd xmm6, [esi + eax + 12];

			vperm2f128 ymm3, ymm3, ymm4, 00100000b;
			vperm2f128 ymm4, ymm5, ymm6, 00100000b;

			// Convert to float
			vcvtdq2ps ymm3, ymm3;
			vcvtdq2ps ymm4, ymm4;

			// Scale
			vmulps ymm3, ymm3, ymm7;
			vmulps ymm4, ymm4, ymm7;

			// Store

			mov edi, output;
			//mov eax, k;
			shl eax, 2;

			vmovaps[edi + eax], ymm3;
			vmovaps[edi + eax + 32], ymm4;
		}
	}

}

void normalized_float_to_uint8(const float *input, unsigned char *output,const int size)
{
	for (int k = 0; k < size; ++k)
	{
		output[k] = Ceng::CE_FloatToInt(input[k] * 255.0f);
		//output[k] = unsigned char(input[k] * 255.0f);
	}
}

void normalized_float_to_uint8_sse(const float *input, unsigned char *output, const int size)
{
	const float fxScale = 255.0f;

	const _declspec(align(16)) float scaleVec[] = { fxScale, fxScale, fxScale, fxScale };

	const _declspec(align(16)) unsigned char shuffle_mask[] = { 0, 4, 8, 12, 1, 5, 9, 13, 2, 6, 10, 14, 3, 7, 11, 15 };

	__asm
	{
		vmovdqa xmm7, scaleVec;
		vmovdqa xmm6, shuffle_mask;
	}

	for (int k = 0; k < size; k += 16)
	{
		__asm
		{
			mov esi, input;
			mov eax, k;
			shl eax, 2;

			vmovaps xmm0, [esi + eax];
			vmovaps xmm1, [esi + eax + 16];
			vmovaps xmm2, [esi + eax + 32];
			vmovaps xmm3, [esi + eax + 48];

			vmulps xmm0, xmm0, xmm7;
			vmulps xmm1, xmm1, xmm7;
			vmulps xmm2, xmm2, xmm7;
			vmulps xmm3, xmm3, xmm7;

			vminps xmm0, xmm0, xmm7;
			vminps xmm1, xmm1, xmm7;
			vminps xmm2, xmm2, xmm7;
			vminps xmm3, xmm3, xmm7;

			vcvttps2dq xmm0, xmm0;
			vcvttps2dq xmm1, xmm1;
			vcvttps2dq xmm2, xmm2;
			vcvttps2dq xmm3, xmm3;

			vpslld xmm1, xmm1, 8;
			vpslld xmm2, xmm2, 16;
			vpslld xmm3, xmm3, 24;

			vpor xmm0, xmm0, xmm1;
			vpor xmm2, xmm2, xmm3;
			vpor xmm0, xmm0, xmm2;

			vpshufb xmm0,xmm0,xmm6;

			mov edi, output;
			mov eax, k;

			vmovdqa[edi + eax], xmm0;
		}
	}
}

void normalized_float_to_uint8_optimized_sse(const float *input, unsigned char *output, const int size)
{
	const float clamp = 1.0f - 1.0f / 256.0f;

	const _declspec(align(16)) float clampVec[] = { clamp, clamp, clamp, clamp };

	const float fxScale = float(1 << 15);

	const _declspec(align(16)) float scaleVec[] = { fxScale, fxScale, fxScale, fxScale };

	const _declspec(align(16)) unsigned char shuffle_mask[] = { 0, 4, 8, 12, 1, 5, 9, 13, 2, 6, 10, 14, 3, 7, 11, 15 };

	__asm
	{
		vmovaps xmm7, clampVec;
		vmovaps xmm6, scaleVec;
	}

	for (int k = 0; k < size; k += 16)
	{
		__asm
		{
			mov esi, input;
			mov eax, k;
			shl eax, 2;

			vmovdqa xmm0, [esi + eax];
			vmovdqa xmm1, [esi + eax + 16];
			vmovdqa xmm2, [esi + eax + 32];
			vmovdqa xmm3, [esi + eax + 48];
	
			vpminud xmm0, xmm0, xmm7;
			vpminud xmm1, xmm1, xmm7;
			vpminud xmm2, xmm2, xmm7;
			vpminud xmm3, xmm3, xmm7;
		
			vaddps xmm0, xmm0, xmm6;
			vaddps xmm1, xmm1, xmm6;
			vaddps xmm2, xmm2, xmm6;
			vaddps xmm3, xmm3, xmm6;

			vpslld xmm1, xmm1, 8;
			vpor xmm0, xmm0, xmm1;

			vpslld xmm3, xmm3, 8;
			vpor xmm2, xmm2, xmm3;

			vpslld xmm2, xmm2, 16;

			vpblendw xmm0, xmm0, xmm2, 10101010b;
		
			vpshufb xmm0, xmm0, shuffle_mask;

			mov edi, output;
			mov eax, k;

			vmovdqa[edi + eax], xmm0;
		}
	}

}

void normalized_float_to_uint8_avx(const float *input, unsigned char *output, const int size)
{
	const float fxScale = 255.0f;

	const _declspec(align(32)) float scaleVec[] = { fxScale, fxScale, fxScale, fxScale, fxScale, fxScale, fxScale, fxScale };

	const _declspec(align(16)) unsigned char shuffle_mask[] = { 0, 4, 8, 12, 1, 5, 9, 13, 2, 6, 10, 14, 3, 7, 11, 15 };

	__asm
	{
		vmovaps ymm7, scaleVec;
		vmovdqa xmm6, shuffle_mask;
	}

	for (int k = 0; k < size; k += 16)
	{
		__asm
		{
			mov esi, input;
			mov eax, k;
			shl eax, 2;

			vmovaps ymm0, [esi + eax];
			vmovaps ymm1, [esi + eax + 32];

			vmulps ymm0, ymm0, ymm7;
			vmulps ymm1, ymm1, ymm7;

			vminps ymm0, ymm0, ymm7;
			vminps ymm1, ymm1, ymm7;
	
			vcvttps2dq ymm0, ymm0;
			vcvttps2dq ymm1, ymm1;

			vmovaps ymm2, ymm1; // lower half to ymm2
			vperm2f128 ymm3, ymm1, ymm1, 00000001b; // upper half to ymm3

			vperm2f128 ymm1, ymm0, ymm0, 00000001b; // upper half to ymm1
	
			vpslld xmm1, xmm1, 8;
			vpslld xmm2, xmm2, 16;
			vpslld xmm3, xmm3, 24;

			vpor xmm0, xmm0, xmm1;
			vpor xmm2, xmm2, xmm3;
			vpor xmm0, xmm0, xmm2;

			vpshufb xmm0, xmm0, xmm6;

			mov edi, output;
			mov eax, k;

			vmovdqa [edi + eax], xmm0;
		}
	}
}
