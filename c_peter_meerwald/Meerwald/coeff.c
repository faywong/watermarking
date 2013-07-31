#include "wm.h"
#include "coeff.h"

double **alloc_coeffs(int cols, int rows) {
  double **p;
  int i;

  p = (double **)malloc(rows * sizeof(double *));
  if (!p) {
#ifdef DEBUG
   fprintf(stderr, "alloc_coeffs(): malloc() failed\n");
   exit(1);
#else
   return NULL;
#endif
  }
  p[0] = malloc(rows * cols * sizeof(double));
  if (!p[0]) {
#ifdef DEBUG
    fprintf(stderr, "alloc_coeffs(): malloc() failed\n");
    exit(1);
#else
    free(p);
    return NULL;
#endif
  }
  for (i = 1; i < rows; i++) {
    p[i] = &(p[0][i * cols]);
  }

  return p;
}

void free_coeffs(double **coeffs) {
  free(coeffs[0]);
  free(coeffs);
}

double **alloc_coeffs_8x8() {
  return alloc_coeffs(8, 8);
}

void print_coeffs_8x8(double **coeffs) {
 int i, j;

  for (i = 0; i < 8; i++) {
    for (j = 0; j < 8; j++)
      fprintf(stderr, "%8.2f ", coeffs[i][j]);
    fprintf(stderr, "\n");
  }
}

void print_coeffs(double **coeffs, int c, int r, int w, int h) {
  int i, j;
  double *p;

#ifdef DEBUG
  if (!coeffs) {
    fprintf(stderr, "print_coeffs(): NULL pixels\n");
  }
  if (w <= 0 || h <= 0 || c < 0 || r < 0) {
    fprintf(stderr, "print_coeffs(): block dimension out of range\n");
  }
#endif

  for (j = r; j < r + h; j++) {
    p = &coeffs[j][c];
    for (i = 0; i < w; i++)
      fprintf(stderr, "%8.2f ", *(p++));
    fprintf(stderr, "\n");
  }
}
