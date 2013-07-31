#include "wm.h"
#include "dwt_util.h"
#include <ctype.h>

void copy_coeffs_from_dwt(double ** block_coeffs, double ** dwt_coeffs,
int level, int band, int width, int height) {
  int i, j;
  int size = width >> level;
  int h = (band > 2) ? size : 0;
  int w = (band & 1) ? 0 : size;

  for (i = 0; i < size; i++)
    for (j = 0; j < size; j++)
      block_coeffs[i][j] = dwt_coeffs[h + i][w + j];
}

void copy_coeffs_to_dwt(double ** dwt_coeffs, double ** block_coeffs,
int level, int band, int width, int height) {
  int i, j;
  int size = width >> level;
  int h = (band > 2) ? size : 0;
  int w = (band & 1) ? 0 : size;

  for (i = 0; i < size; i++)
    for (j = 0; j < size; j++)
      dwt_coeffs[h + i][w + j] = block_coeffs[i][j];
}

char *subband_name(int type) {
  switch (type) {
    case LL: return "LL";
    case HL: return "HL";
    case LH: return "LH";
    case HH: return "HH";
    default: return "XX";
  }
}

int subband_in_list(char *list, int type, int level) {
  return 1;
}

int subband_wp_in_list(char *list, char *name) {
  return 1;
}

int calc_subband_wp_level(char *name){
  return strlen(name);
}

void calc_subband_location(int cols, int rows, int type, int level, int *col, int *row) {
  *col = *row = 0;

  if (level <= 0 || level > find_deepest_level(cols, rows) - 1) return;

  switch (type) {
    case LL:
      break;
    case HL:
      *col = 0;
      *row = rows >> level;
      break;
    case LH:
      *col =  cols >> level;
      *row = 0;
      break;
    case HH:
      *col = cols >> level;
      *row = rows >> level;
      break;
    default:
      break;
  }
}

void calc_subband_wp_location(int cols, int rows, char *name, int *col, int *row) {
  char *p = name;
  int level = 0;
  *col = *row = 0;

  while (*p) {
    level++;
    switch (toupper(*p)) {
      case 'A':
        break;
      case 'H':
        *col += (cols >> level);
        break;
      case 'V':
        *row += (rows >> level);
        break;
      case 'D':
        *col += (cols >> level);
        *row += (rows >> level);
        break;
      default:
        break;
    }
    p++;
  }
}

Pixel *get_dwt_data(Image_tree dwt, int level, int type) {
  return get_dwt_image(dwt, level, type)->data;
}

Image get_dwt_image(Image_tree dwt, int level, int type) {
  return get_dwt_subband(dwt, level, type)->image;
}

Image_tree get_dwt_subband(Image_tree dwt, int level, int type) {
  while (--level)
    dwt = dwt->coarse;

  switch (type) {
    case LL: 
      return dwt->coarse;
    case HL: 
      return dwt->vertical;
    case LH: 
      return dwt->horizontal;
    case HH: 
      return dwt->diagonal;
  }

  return NULL;
}

Pixel get_dwt_coeff(Image_tree dwt, int level, int type, int coeff) {
  return get_dwt_data(dwt, level, type)[coeff];
}

Pixel get_dwt_location(Image_tree dwt, int level, int type, int col, int row) {
  return get_pixel(get_dwt_image(dwt, level, type), col, row);
}

static void calc__subband(Image_tree p, Image_tree q, double *min, double *max, double *error) {
  int i;
  
  if (!p || !q) return;
  
  *error = 0;
  *min = *max = fabs(p->image->data[0] - q->image->data[0]);
  for (i = 0; i < p->image->size; i++) {
    double diff = fabs(p->image->data[i] - q->image->data[i]);
    
    *error += sqr(diff);
    if (diff < *min) *min = diff;
    if (diff > *max) *max = diff;
  }
}

void calc_subband(Image_tree p, Image_tree q, int type, double *min, double *max, double *error) {
  calc__subband(p, q, min, max, error);
}

void calc_subband_wp(Image_tree p, Image_tree q, char *name, double *min, double *max, double *error) {
  calc__subband(p, q, min, max, error);
}
