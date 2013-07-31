#ifndef DWT_H
#define DWT_H

#include "wm.h"
#include "pgm.h"
#include "wavelet.h"

#define FILTERG 1
#define FILTERH 2
#define FILTERGi 3
#define FILTERHi 4

void init_dwt(int cols, int rows, const char *filter_name, int filter, int level, int method);
Image_tree fdwt(gray **input);
Image_tree fdwt_wp(gray **input);
void idwt(Image_tree dwts, gray **output);
void idwt_wp(Image_tree dwts, gray **output);
int gen_pollen_filter(double *filter, double alpha, double beta, int which);
void dwt_pollen_filter(double alpha, double beta);
int gen_param_filter(double *filter, int n, double alpha[], int which);
void dwt_param_filter(double alpha[], int param_len[]);
void done_dwt();

#endif
