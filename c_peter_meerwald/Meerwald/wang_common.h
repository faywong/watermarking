#ifndef WANG_COMMON_H
#define WANG_COMMON_H

#include "dwt.h"

typedef struct Subband_data_struct {
  double T;
  double Cmax;
  double beta;
  Image_tree tree;
  int level;
  int type;
  int width;
  int height;
  int size;
  Image image;
  char** selected;
} *Subband_data;

Subband_data *subbands;
int n_subbands;

void init_subbands(Image_tree tree);
Subband_data alloc_subband(int type, Image_tree tree);
void free_subband(Subband_data subband);
void free_subbands();

void set_subband_beta(Subband_data subband, double beta);
void set_subbands_beta(double beta);
void set_subbands_type_beta(int type, double beta);

void calc_subband_threshold(Subband_data subband);
void calc_subbands_threshold();

int subband_coeff_isselected(Subband_data subband, int coeff);
Pixel get_subband_coeff(Subband_data subband, int coeff);
void set_subband_coeff(Subband_data subband, int coeff, Pixel data);

Subband_data select_subband();
int select_subband_coeff_from(Subband_data subband, int from);
int select_subband_coeff(Subband_data subband);
void mark_subband_coeff(Subband_data subband, int coeff);

Pixel figure_orig_coeff(double T, double alpha, double beta, Pixel coeff);

#endif
