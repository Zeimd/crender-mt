#pragma once

void normalized_float_to_uint8(const float* input, unsigned char* output, const int inputSize);

void normalized_float_to_uint8_sse(const float* input, unsigned char* output, const int inputSize);

void normalized_float_to_uint8_optimized_sse(const float* input, unsigned char* output, const int inputSize);

void normalized_float_to_uint8_avx(const float* input, unsigned char* output, const int inputSize);

void uint8_to_normalized_float(const unsigned char* input, float* output, const int inputSize);

void uint8_to_normalized_float_strip4(const unsigned char* input, float* output, const int inputSize);

void uint8_to_normalized_float_strip8(const unsigned char* input, float* output, const int inputSize);

void uint8_to_normalized_float_strip16(const unsigned char* input, float* output, const int inputSize);

void uint8_to_normalized_float_sse(const unsigned char* input16, float* output16, const int inputSize);

void uint8_to_normalized_float_optimized_sse(const unsigned char* input16, float* output16, const int inputSize);

void uint8_to_normalized_float_sse_v3(const unsigned char* input16, float* output16, const int inputSize);

void uint8_to_normalized_float_avx(const unsigned char* input16, float* output16, const int inputSize);

//****************************************************************************
// sorting algorithm

void float_sort4_selection_sort(float* input4, const int size);

void float_sort4_insertion_sort(float* input4, const int size);

void float_sort4_stdlib_qsort(float* input4, const int size);

void float_sort4_minmax_stl(float* input4, const int size);

// Works for floating point values [+0,inf[
void float_sort4_minmax_stl_int_punning(float* input4, const int size);

void float_sort4_minmax_x87_cmov(float* input4, const int size);

// Doesn't store immediates to RAM
void float_sort4_minmax_x87_cmov_v2(float* input4, const int size);

// Scalar SSE version of minmax sort
void float_sort4_minmax_sse_scalar(float* input4, const int size);

// Vectorized version of minmax sort
void float_sort4_minmax_sse(float* input4, const int size);

// Alternative version of SSE minmax sort that uses sign flips
// to perform min() and max() at the same time
void float_sort4_minmax_sse_v2(float* input4, const int size);

