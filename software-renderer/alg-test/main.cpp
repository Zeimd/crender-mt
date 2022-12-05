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

#include "alg-test.h"

#include <ceng/datatypes/aligned-buffer.h>


void round_trip_test(const unsigned char *input);

void accuracy_test(float *exact, float *test, const int size);

int sort_test();

int uint8_to_normalized_float_tests();

int main()
{
	sort_test();

	//uint8_to_normalized_float_tests();

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


int sort_test()
{
	const int test_size = 4 * 4;

	float* input = (float*)Ceng::AlignedMalloc(test_size * sizeof(float), 64);

	float* temp_input = (float*)Ceng::AlignedMalloc(test_size * sizeof(float), 64);

	float* correct_output = (float*)Ceng::AlignedMalloc(test_size * sizeof(float), 64);


	std::cout << "*******************************************************" << std::endl;
	std::cout << "sort test" << std::endl;

	std::cout << "randomizing input" << std::endl;
	for (int k = 0; k < test_size; ++k)
	{
		input[k] = (rand() % 257) / 256.0f;
	}

	input[0] = 4.0;
	input[1] = 3.0;
	input[2] = 2.0;
	input[3] = 1.0;

	std::cout << "Input dump:" << std::endl;

	int groups = test_size / 4;

	for (int i = 0; i < groups; i++)
	{
		std::cout << "-------------------------------------" << std::endl;
		for (int k = 0; k < 4; k++)
		{
			std::cout << "input[" << k << "] = " << input[4*i+k] << std::endl;
		}
	}

	std::cout << "-------------------------------------" << std::endl;

	double start, end;
	double duration, base_duration;

	start = Timer();

	memcpy(correct_output, input, test_size * sizeof(float));

	float_sort4_stdlib_qsort(correct_output, test_size);

	std::cout << "correct output dump:" << std::endl;

	for (int i = 0; i < groups; i++)
	{
		std::cout << "-------------------------------------" << std::endl;
		for (int k = 0; k < 4; k++)
		{
			std::cout << "out[" << k << "] = " << correct_output[4 * i + k] << std::endl;
		}
	}

	std::cout << "-------------------------------------" << std::endl;

	end = Timer();

	base_duration = end - start;

	std::cout << "stdlib quicksort: " << base_duration << std::endl;

	start = Timer();

	memcpy(temp_input, input, test_size * sizeof(float));

	float_sort4_sse(temp_input, test_size);

	end = Timer();

	duration = end - start;

	std::cout << "SSE: " << duration << ", ratio = " << duration / base_duration <<  std::endl;

	std::cout << "correctness check:" << std::endl;

	int errorCount = 0;

	for (int i = 0; i < groups; i++)
	{
		for (int k = 0; k < 4; k++)
		{
			if (temp_input[4 * i + k] != correct_output[4 * i + k])
			{
				std::cout << "Mismatch: " << k << ": found = " << temp_input[4 * i + k] << ", expected = " << correct_output[4 * i + k] << std::endl;
				errorCount++;
			}
		}
	}

	if (errorCount == 0)
	{
		std::cout << "No errors" << std::endl;
	}

	Ceng::AlignedFree(input);
	Ceng::AlignedFree(temp_input);
	Ceng::AlignedFree(correct_output);

	return 0;
}


typedef void (*uint8_to_normalized_float_callback)(const unsigned char* input, float* output, const int inputSize);

double uint8_to_normalized_float_speed_test(const char* name, uint8_to_normalized_float_callback callback, 
	const unsigned char* input, float* output, const int inputSize, double base_duration)
{
	double start_time, end_time;
	double duration;

	start_time = Timer();

	(*callback)(input, output, inputSize);

	end_time = Timer();

	duration = end_time - start_time;

	std::cout << name << ": " << duration << " , ratio = " << duration / base_duration << std::endl;

	return duration;
}

typedef void (*normalized_float_to_uint8_callback)(const float* input, unsigned char* output, const int inputSsize);

double normalized_float_to_uint8_speed_test(const char* name, normalized_float_to_uint8_callback callback,
	const float* input, unsigned char* output, const int inputSize, double base_duration)
{
	double start_time, end_time;
	double duration;

	start_time = Timer();

	(*callback)(input, output, inputSize);

	end_time = Timer();

	duration = end_time - start_time;

	std::cout << name <<": " << duration << " , ratio = " << duration / base_duration << std::endl;

	return duration;
}

int uint8_to_normalized_float_tests()
{
	_declspec(align(32)) unsigned char input_sse[256];

	for (int k = 0; k < 256; ++k)
	{
		input_sse[k] = unsigned char(k);
	}

	float results[256];

	std::cout << "C version:" << std::endl;

	uint8_to_normalized_float(input_sse, results, 256);

	_declspec(align(32)) unsigned char round_trip[256];

	normalized_float_to_uint8(results, round_trip, 256);

	//round_trip_test(round_trip);

	//_declspec(align(32)) unsigned char sse_round_trip[256];

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

	//_declspec(align(32)) float results_avx[256];

	/*
	uint8_to_normalized_float_avx(input_sse, results_avx, 256);

	accuracy_test(results, results_avx, 256);

	normalized_float_to_uint8_avx(results_avx, sse_round_trip, 256);

	round_trip_test(sse_round_trip);
	*/
	//return 0;

	double base_duration;

	const int test_size = 16 * 10000000;

	float* speed_input = (float*)Ceng::AlignedMalloc(test_size * sizeof(float), 64);

	unsigned char* speed_result = (unsigned char*)Ceng::AlignedMalloc(test_size * sizeof(unsigned char), 64);

	std::cout << "randomizing input" << std::endl;
	for (int k = 0; k < test_size; ++k)
	{
		speed_input[k] = (rand() % 257) / 256.0f;
		speed_result[k] = 0;
	}

	std::cout << "*******************************************************" << std::endl;
	std::cout << "speed test: normalized float -> uint8" << std::endl;

	base_duration = normalized_float_to_uint8_speed_test("Scalar C", normalized_float_to_uint8, speed_input, speed_result, test_size, 1.0);

	normalized_float_to_uint8_speed_test("SSE v1", normalized_float_to_uint8_sse, speed_input, speed_result, test_size, base_duration);
	normalized_float_to_uint8_speed_test("SSE v2", normalized_float_to_uint8_optimized_sse, speed_input, speed_result, test_size, base_duration);
	normalized_float_to_uint8_speed_test("AVX", normalized_float_to_uint8_avx, speed_input, speed_result, test_size, base_duration);

	std::cout << "*******************************************************" << std::endl;
	std::cout << "speed test: uint8 -> normalized float" << std::endl;

	base_duration = uint8_to_normalized_float_speed_test("Scalar C", uint8_to_normalized_float, speed_result, speed_input, test_size, 1.0);

	uint8_to_normalized_float_speed_test("scalar , 4x strip", uint8_to_normalized_float_strip4, speed_result, speed_input, test_size, base_duration);
	uint8_to_normalized_float_speed_test("scalar , 8x strip", uint8_to_normalized_float_strip8, speed_result, speed_input, test_size, base_duration);
	uint8_to_normalized_float_speed_test("scalar , 16x strip", uint8_to_normalized_float_strip16, speed_result, speed_input, test_size, base_duration);

	uint8_to_normalized_float_speed_test("SSE_v1", uint8_to_normalized_float_sse, speed_result, speed_input, test_size, base_duration);
	uint8_to_normalized_float_speed_test("SSE_v2", uint8_to_normalized_float_optimized_sse, speed_result, speed_input, test_size, base_duration);
	uint8_to_normalized_float_speed_test("SSE_v3", uint8_to_normalized_float_sse_v3, speed_result, speed_input, test_size, base_duration);
	uint8_to_normalized_float_speed_test("AVX", uint8_to_normalized_float_avx, speed_result, speed_input, test_size, base_duration);

	Ceng::AlignedFree(speed_input);
	Ceng::AlignedFree(speed_result);

	return 0;
}