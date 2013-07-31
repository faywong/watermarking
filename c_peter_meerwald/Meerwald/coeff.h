#ifndef COEFF_H
#define COEFF_H

#include "wm.h"

double **alloc_coeffs(int cols, int rows);
double **alloc_coeffs_8x8();
void free_coeffs(double **coeffs);
void print_coeffs_8x8(double **coeffs);
void print_coeffs(double **coeffs, int c, int r, int w, int h);

#endif
