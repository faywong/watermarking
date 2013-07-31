#include "wm.h"
#include "kim_common.h"


// find the largest absolute coefficient of a subband
double find_subband_largest_coeff(Image_tree s, int subband, int verbose) {
  int i, j;
  double max;

  max = 0.0;
  for (i = 5; i < s->image->height-5; i++)
    for (j = 5; j < s->image->width-5; j++) {
      double coeff;

      coeff = fabs(get_pixel(s->image, i, j));
      if (coeff > max) 
        max = coeff;
    }

  if (verbose > 8)
    fprintf(stderr, "  subband %f\n", max);

  return max;
}

// find largest absolute coefficient of the detail subbands (LH, HL, HH) of 
// a decomposition level
double find_level_largest_coeff(Image_tree p, int verbose) {
  double h, v, d;

  h = find_subband_largest_coeff(p->horizontal, HORIZONTAL, verbose);
  v = find_subband_largest_coeff(p->vertical, VERTICAL, verbose);
  d = find_subband_largest_coeff(p->diagonal, DIAGONAL, verbose);

  return MAX(h, MAX(v, d));
}

// calculate the significance threshold given the maximum absolute
// coefficient at a decomposition level
double calc_level_threshold(double max_coeff, int verbose) {
  double threshold;

  threshold = pow(2.0, floor(log(max_coeff) / log(2.0)) - 1.0);

  if (verbose > 7)
    fprintf(stderr, "  max %f, threshold %f\n", max_coeff, threshold);

  return threshold;
}

// calculate an appropriate embedding strength for a given decomposition level
// and a base alpha strength
double calc_level_alpha_detail(double alpha, int maxlevels, int level, int verbose) {
  double level_alpha;

  level_alpha = alpha / pow(2.0, level - 1);

  return level_alpha;
}

