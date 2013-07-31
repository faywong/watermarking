#ifndef KIM_COMMON_H
#define KIM_COMMON_H

#include "dwt.h"
#include "dwt_util.h"

double find_subband_largest_coeff(Image_tree p, int subband, int verbose);
double find_level_largest_coeff(Image_tree p, int verbose);

double calc_level_threshold(double max_coeff, int verbose);
double calc_level_alpha_detail(double alpha, int maxlevels, int level, int verbose);

#endif
