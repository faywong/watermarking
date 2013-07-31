#ifndef SORT_H
#define SORT_H

#include "wm.h"
#include "pgm.h"

void sort_grays(gray a[], int n);
void select_largest_grays(gray array[], int n, int m, gray largest[]);

void sort_coeffs(double a[], int n);
void select_largest_coeffs(double array[], int n, int m, double largest[]);

#endif
