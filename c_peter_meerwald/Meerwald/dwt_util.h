#ifndef DWT_UTIL_H
#define DWT_UTIL_H

#include "dwt.h"

#define LL 1
#define LH 2
#define HL 3
#define HH 4

#define COARSE LL
#define HORIZONTAL LH
#define VERTICAL HL
#define DIAGONAL HH

void copy_coeffs_from_dwt(double ** block_coeffs, double ** dwt_coeffs,
  int level, int band, int width, int height);

void copy_coeffs_to_dwt(double ** dwt_coeffs, double ** block_coeffs,
  int level, int band, int width, int height);

char *subband_name(int type);

int subband_in_list(char *list, int type, int level);
int subband_wp_in_list(char *list, char *name);

void calc_subband_location(int cols, int rows, int type, int level, int *col, int *row);
void calc_subband_wp_location(int cols, int rows, char *name, int *col, int *row);
int calc_subband_wp_level(char *name);

Pixel *get_dwt_data(Image_tree dwt, int level, int type);
Image get_dwt_image(Image_tree dwt, int level, int type);
Image_tree get_dwt_subband(Image_tree dwt, int level, int type);
Pixel get_dwt_coeff(Image_tree dwt, int level, int type, int coeff);
Pixel get_dwt_location(Image_tree dwt, int level, int type, int col, int row);

void calc_subband(Image_tree p, Image_tree q, int type, double *min, double *max, double *error);
void calc_subband_wp(Image_tree p, Image_tree q, char *name, double *min, double *max, double *error);


#endif
