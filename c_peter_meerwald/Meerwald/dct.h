#ifndef DCT_H
#define DCT_H

#include "wm.h"
#include "coeff.h"
#include "pgm.h"

extern int N;
extern int M;

void init_dct_NxM(int width, int height);
void fdct_NxM(gray **pixels, double **dcts);
void idct_NxM(double **dcts, gray **pixels);

void init_dct_NxN(int width, int height);
void fdct_NxN(gray **pixels, double **dcts);
void idct_NxN(double **dcts, gray **pixels);
void fdct_inplace_NxN(double **coeffs);
void idct_inplace_NxN(double **coeffs);

/*
 * 'NJPEG' defines the JPEG's DCT block size (8x8)
 */
#define NJPEG 8

void init_quantum_8x8(int quality);
void init_quantum_JPEG_lumin(int quality);
void init_quantum_JPEG_chromin(int quality);
void quantize_8x8(double **transform);
void dequantize_8x8(double **transform);
void init_dct_8x8();
void fdct_8x8(gray **input, double **output);
void fdct_block_8x8(gray **input, int col, int row, double **output);
void idct_8x8(double **input, gray **output);
void idct_block_8x8(double **input, gray **output, int col, int row);
int is_middle_frequency_coeff_8x8(int coeff);

#endif
