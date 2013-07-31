#include "dwt_util.h"
#include "wang_common.h"

void init_subbands(Image_tree tree) {
  int levels = 0;
  int i;
  Image_tree p = tree;

  // determine # of detail subbands
  while (p->coarse != NULL) {
    levels++;
    p = p->coarse;
  }

  // there are 3 detail subbands per level
  n_subbands = 3 * levels;

  // allocate memory for subband data
  subbands = malloc(n_subbands * sizeof(Subband_data));

  p = tree;
  i = 0;
  while (p->coarse != NULL) {
    subbands[i++] = alloc_subband(HORIZONTAL, p->horizontal);
    subbands[i++] = alloc_subband(VERTICAL, p->vertical);
    subbands[i++] = alloc_subband(DIAGONAL, p->diagonal);
    
    p = p->coarse;
  }

}

Subband_data alloc_subband(int type, Image_tree tree) {
  int i;
  Subband_data p = malloc(sizeof(struct Subband_data_struct));

  p->T = 0.0;
  p->beta = 0.0;
  p->Cmax = 0.0;
  p->tree = tree;
  p->level = tree->level;
  p->width = tree->image->width;
  p->height = tree->image->height;
  p->size = p->height * p->width;
  p->image = tree->image;
  p->type = type;
  
  p->selected = malloc(p->height * sizeof(char *));
  p->selected[0] = calloc(p->size, sizeof(char));
  for (i = 1; i < p->height; i++)
    p->selected[i] = &(p->selected[0][i * p->width]); 

  return p;
}

void set_subband_beta(Subband_data subband, double beta) {
  subband->beta = beta;
}

void set_subbands_beta(double beta) {
  int i;

  for (i = 0; i < n_subbands; i++)
    set_subband_beta(subbands[i], beta);
}

void set_subbands_type_beta(int type, double beta) {
  int i;

  for (i = 0; i < n_subbands; i++)
    if (subbands[i]->type == type)
      set_subband_beta(subbands[i], beta);
}

void calc_subband_threshold(Subband_data subband) {
  double max;
  int i, j;

  max = fabs(get_pixel(subband->image, 0, 0));
  for (i = 0; i < subband->height; i++)
    for (j = 0; j < subband->width; j++) {
      Pixel p = fabs(get_pixel(subband->image, i, j));
      if (p > max)
        max = p;
    }

  subband->Cmax = max;
  subband->T = max / 2.0;
}

void calc_subbands_threshold() {
  int i;

  for (i = 0; i < n_subbands; i++)
    calc_subband_threshold(subbands[i]);
}

Subband_data select_subband() {
  int max = 0;
  int i;

  for (i = 0; i < n_subbands; i++)
    if ((subbands[i]->beta * subbands[i]->T) > (subbands[max]->beta * subbands[max]->T))
      max = i;

  return subbands[max];
}

int subband_coeff_isselected(Subband_data subband, int coeff) {
  return subband->selected[0][coeff];
}

Pixel get_subband_coeff(Subband_data subband, int coeff) {
  return subband->image->data[coeff];
}

void set_subband_coeff(Subband_data subband, int coeff, Pixel data) {
  subband->image->data[coeff] = data;
}

int select_subband_coeff_from(Subband_data subband, int from) {
  int i;

  for (i = from; i < subband->size; i++)
    if (!subband_coeff_isselected(subband, i) &&
      get_subband_coeff(subband, i) > subband->T)
      return i;

  return -1;
}

int select_subband_coeff(Subband_data subband) {
  return select_subband_coeff_from(subband, 0);
}

void mark_subband_coeff(Subband_data subband, int coeff) {
  subband->selected[0][coeff] = 1; 
}

void free_subband(Subband_data subband) {
  free(subband->selected[0]);
  free(subband->selected);
  free(subband);
}

void free_subbands() {
  int i;
  
  for (i = 0; i < n_subbands; i++) 
    free_subband(subbands[i]);

  free(subbands);
}

#define LARGE DBL_MAX

Pixel figure_orig_coeff(double T, double alpha, double beta, Pixel coeff) {
  int p, p_min = 0;
  double dist_min = LARGE;
  double sign = (coeff >= 0) ? 1.0 : -1.0;

  for (p = 1; p < 1.0 / (2.0 * alpha); p++) {
    double dist, delta;

    delta = (1.0 + 2.0 * p * alpha) * T;
    dist = fabs(delta - fabs(coeff));

    if (dist < dist_min) {
      dist_min = dist;
      p_min = p;
    }
  }  

  if (!p_min) p_min = 1;
  return sign * (1.0 + 2.0 * p_min * alpha) * T;
}
