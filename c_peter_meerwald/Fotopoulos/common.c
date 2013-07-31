#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>
#include "pgm.h"
#include "common.h"

#define IA      16807
#define IM      2147483647
#define AM      (1.0/IM)
#define IQ      127773
#define IR      2836
#define MASK    123459876

static int NN = 0;
static int m = 0;
static double two_over_N = 0;
static double root2_over_rootN = 0;
static double *C = NULL;

static gray maxval;
static int format;

void open_image(FILE *in, int *width, int *height)
{

  pgm_readpgminit(in, width, height, &maxval, &format);
}

void load_image(int **im, FILE *in, int width, int height)
{
  int col, row;
  gray *rowbuf;

  rowbuf = malloc(sizeof(gray) * width);

  for (row = 0; row < height; row++) {
    pgm_readpgmrow(in, rowbuf, width, maxval, format);
    for (col = 0; col < width; col++)
      im[row][col] = rowbuf[col];
  }

  free(rowbuf);
}

void save_image(int **im, FILE *out, int width, int height)
{
  int col, row;
  gray *rowbuf;

  pgm_writepgminit(out, width, height, 255, 0);

  rowbuf = malloc(sizeof(gray) * width);

  for (row = 0; row < height; row++) {
    for (col = 0; col < width; col++)
      rowbuf[col] = im[row][col];
    pgm_writepgmrow(out, rowbuf, width, 255, 0);
  }

  free(rowbuf);
}

int ** imatrix(int nrows, int ncols)
{
  int **m;
  int i, j;
  m = (int **) malloc (nrows * sizeof(int *));
  for (i = 0; i < nrows; i++) {
    m[i] = (int *) malloc (ncols * sizeof(int));
    if (!m[i]) fprintf(stderr, "\nIt's not working");
  }
  for (i = 0; i < nrows; i++)
    for (j = 0; j < ncols; j++)
      m[i][j] = 0;
  return m;
}

void freematrix(int **I, int rows)
{
  int k;
  for (k = 0; k < rows; k++)
    free (I[k]);
}

float ran0(long int *idum)
{
  long int k;
  float ans;
  *idum ^= MASK;
  k = (*idum) / IQ;
  *idum = IA * (*idum - k * IQ) - IR * k;
  if (*idum < 0) *idum += IM;
  ans = AM * (*idum);
  *idum ^= MASK;
  return ans;
}

float gasdev(long int *idum)
{

  float v1;
  v1 = (float) sqrt( -2.0 * log(ran0(idum))) * cos(2 * PI * ran0(idum));
  return v1;
}


void put_image_from_int_2_double(int **i, double *f, int N)
{
  int l, j, k;
  k = 0;
  for (l = 0; l < N; l++)
    for (j = 0; j < N; j++)
      f[k++] = (double) i[l][j];
}

void put_image_from_double_2_int(double *f, int **i, int N)
{
  int l, j, k;
  k = 0;
  for (l = 0; l < N; l++)
    for (j = 0; j < N; j++)
      i[l][j] = (int) f[k++];
}


void bitrev(double *f, int len)
{
  int i, j, m, halflen;
  double temp;

  if (len <= 2) return ;  /* No action necessary if n=1 or n=2 */
  halflen = len >> 1;
  j = 1;
  for (i = 1; i <= len; i++) {
    if (i < j) {
      temp = f[j - 1];
      f[j - 1] = f[i - 1];
      f[i - 1] = temp;
    }
    m = halflen;
    while (j > m) {
      j = j - m;
      m = (m + 1) >> 1;
    }
    j = j + m;
  }
}

void inv_sums(double *f)
{
  int stepsize, stage, curptr, nthreads, thread, step, nsteps;

  for (stage = 1; stage <= m - 1; stage++) {
    nthreads = 1 << (stage - 1);
    stepsize = nthreads << 1;
    nsteps = (1 << (m - stage)) - 1;
    for (thread = 1; thread <= nthreads; thread++) {
      curptr = NN - thread;
      for (step = 1; step <= nsteps; step++) {
        f[curptr] += f[curptr - stepsize];
        curptr -= stepsize;
      }
    }
  }
}

//--------------------------------------------------------
//Foreign code - FCT from Bath Univerity
//--------------------------------------------------------
void rarrwrt(double f[], int n)
{
  int i;

  for (i = 0; i <= n - 1; i++) {
    fprintf(stderr, "%4d : %f\n", i, f[i]);
  }
}

/* fast DCT based on IEEE signal proc, 1992 #8, yugoslavian authors. */

void fwd_sums(double *f)
{
  int stepsize, stage, curptr, nthreads, thread, step, nsteps;

  for (stage = m - 1; stage >= 1; stage--) {
    nthreads = 1 << (stage - 1);
    stepsize = nthreads << 1;
    nsteps = (1 << (m - stage)) - 1;
    for (thread = 1; thread <= nthreads; thread++) {
      curptr = nthreads + thread - 1;
      for (step = 1; step <= nsteps; step++) {
        f[curptr] += f[curptr + stepsize];
        curptr += stepsize;
      }
    }
  }
}

void scramble(double *f, int len)
{
  double temp;
  int i, ii1, ii2, halflen, qtrlen;

  halflen = len >> 1;
  qtrlen = halflen >> 1;
  bitrev(f, len);
  bitrev(&f[0], halflen);
  bitrev(&f[halflen], halflen);
  ii1 = len - 1;
  ii2 = halflen;
  for (i = 0; i <= qtrlen - 1; i++) {
    temp = f[ii1];
    f[ii1] = f[ii2];
    f[ii2] = temp;
    ii1--;
    ii2++;
  }
}

void unscramble(double *f, int len)
{
  double temp;
  int i, ii1, ii2, halflen, qtrlen;

  halflen = len >> 1;
  qtrlen = halflen >> 1;
  ii1 = len - 1;
  ii2 = halflen;
  for (i = 0; i <= qtrlen - 1; i++) {
    temp = f[ii1];
    f[ii1] = f[ii2];
    f[ii2] = temp;
    ii1--;
    ii2++;
  }
  bitrev(&f[0], halflen);
  bitrev(&f[halflen], halflen);
  bitrev(f, len);
}

void initcosarray(int length)
{
  int i, group, base, item, nitems, halfN;
  double factor;

  m = -1;
  do {
    m++;
    NN = 1 << m;
    if (NN > length) {
      fprintf(stderr, "ERROR in FCT-- length %d not a power of 2\n", length);
      exit(1);
    }
  } while (NN < length);
  if (C != NULL) free(C);
  C = (double *)calloc(NN, sizeof(double));
  if (C == NULL) {
    fprintf(stderr, "Unable to allocate C array\n");
    exit(1);
  }
  halfN = NN / 2;
  two_over_N = 2.0 / (double)NN;
  root2_over_rootN = sqrt(2.0 / (double)NN);
  for (i = 0; i <= halfN - 1; i++) C[halfN + i] = 4 * i + 1;
  for (group = 1; group <= m - 1; group++) {
    base = 1 << (group - 1);
    nitems = base;
    factor = 1.0 * (1 << (m - group));
    for (item = 1; item <= nitems; item++) C[base + item - 1] = factor * C[halfN + item - 1];
  }

  //printf("before taking cos, C array =\n"); rarrwrt(C,N);
  for (i = 1; i <= NN - 1; i++) C[i] = 1.0 / (2.0 * cos(C[i] * PI / (2.0 * NN)));
  //printf("After taking cos, Carray = \n"); rarrwrt(C,N);
}


void inv_butterflies(double *f)
{
  int stage, ii1, ii2, butterfly, ngroups, group, wingspan, increment, baseptr;
  double Cfac, T;

  for (stage = 1; stage <= m; stage++) {
    ngroups = 1 << (m - stage);
    wingspan = 1 << (stage - 1);
    increment = wingspan << 1;
    for (butterfly = 1; butterfly <= wingspan; butterfly++) {
      Cfac = C[wingspan + butterfly - 1];
      baseptr = 0;
      for (group = 1; group <= ngroups; group++) {
        ii1 = baseptr + butterfly - 1;
        ii2 = ii1 + wingspan;
        T = Cfac * f[ii2];
        f[ii2] = f[ii1]-T;
        f[ii1] = f[ii1] + T;
        baseptr += increment;
      }
    }
  }
}

void fwd_butterflies(double *f)
{
  int stage, ii1, ii2, butterfly, ngroups, group, wingspan, increment, baseptr;
  double Cfac, T;

  for (stage = m; stage >= 1; stage--) {
    ngroups = 1 << (m - stage);
    wingspan = 1 << (stage - 1);
    increment = wingspan << 1;
    for (butterfly = 1; butterfly <= wingspan; butterfly++) {
      Cfac = C[wingspan + butterfly - 1];
      baseptr = 0;
      for (group = 1; group <= ngroups; group++) {
        ii1 = baseptr + butterfly - 1;
        ii2 = ii1 + wingspan;
        T = f[ii2];
        f[ii2] = Cfac * (f[ii1]-T);
        f[ii1] = f[ii1] + T;
        baseptr += increment;
      }
    }
  }
}

void ifct_noscale(double *f, int length)
{
  if (length != NN) initcosarray(length);
  f[0] *= INVROOT2;
  inv_sums(f);
  bitrev(f, NN);
  inv_butterflies(f);
  unscramble(f, NN);
}

void fct_noscale(double *f, int length)
{
  if (length != NN) initcosarray(length);
  scramble(f, NN);
  fwd_butterflies(f);
  bitrev(f, NN);
  fwd_sums(f);
  f[0] *= INVROOT2;
}

void ifct_defn_scaling(double *f, int length)
{
  ifct_noscale(f, length);
}

void fct_defn_scaling(double *f, int length)
{
  int i;

  fct_noscale(f, length);
  for (i = 0; i <= NN - 1; i++) f[i] *= two_over_N;
}

void ifct(double *f, int length)
{
  /* CALL THIS FOR INVERSE 1D DCT DON-MONRO PREFERRED SCALING */
  int i;

  if (length != NN) initcosarray(length);   /* BGS patch June 1997 */
  for (i = 0; i <= NN - 1; i++) f[i] *= root2_over_rootN;
  ifct_noscale(f, length);
}

void fct(double *f, int length)
{
  /* CALL THIS FOR FORWARD 1D DCT DON-MONRO PREFERRED SCALING */
  int i;

  fct_noscale(f, length);
  for (i = 0; i <= NN - 1; i++) f[i] *= root2_over_rootN;
}

/****************************************************************
    2D FAST DCT SECTION
****************************************************************/

static double *g = NULL;
static double two_over_sqrtncolsnrows = 0.0;
static int ncolsvalue = 0;
static int nrowsvalue = 0;

void initfct2d(int nrows, int ncols)
{
  if ((nrows <= 0) || (ncols < 0)) {
    fprintf(stderr, "FCT2D -- ncols=%d or nrows=%d is <=0\n", nrows, ncols);
    exit(1);
  }
  if (g != NULL) free(g);
  g = (double *)calloc(nrows, sizeof(double));
  if (g == NULL) {
    fprintf(stderr, "FCT2D -- Unable to allocate g array\n");
    exit(1);
  }
  ncolsvalue = ncols;
  nrowsvalue = nrows;
  two_over_sqrtncolsnrows = 2.0 / sqrt(ncols * 1.0 * nrows);
}

void fct2d(double f[], int nrows, int ncols)
/* CALL THIS FOR FORWARD 2d DCT DON-MONRO PREFERRED SCALING */
{
  int u, v;

  if ((ncols != ncolsvalue) || (nrows != nrowsvalue)){
    initfct2d(nrows, ncols);
  }
  for (u = 0; u <= nrows - 1; u++){
    fct_noscale(&f[u*ncols], ncols);
  }
  for (v = 0; v <= ncols - 1; v++){
    for (u = 0; u <= nrows - 1; u++) {
      g[u] = f[u * ncols + v];
    }
    fct_noscale(g, nrows);
    for (u = 0; u <= nrows - 1; u++) {
      f[u*ncols + v] = g[u] * two_over_sqrtncolsnrows;
    }
  }
}

void ifct2d(double f[], int nrows, int ncols)
/* CALL THIS FOR INVERSE 2d DCT DON-MONRO PREFERRED SCALING */
{
  int u, v;

  if ((ncols != ncolsvalue) || (nrows != nrowsvalue)){
    initfct2d(nrows, ncols);
  }
  for (u = 0; u <= nrows - 1; u++){
    ifct_noscale(&f[u*ncols], ncols);
  }
  for (v = 0; v <= ncols - 1; v++){
    for (u = 0; u <= nrows - 1; u++) {
      g[u] = f[u * ncols + v];
    }
    ifct_noscale(g, nrows);
    for (u = 0; u <= nrows - 1; u++) {
      f[u*ncols + v] = g[u] * two_over_sqrtncolsnrows;
    }
  }
}

void matmul(double **a, double **b, double **r, int N)
{
  int i, j, k;

  for (i = 0; i < N; i++)
    for (j = 0; j < N; j++) {
      r[i][j] = 0.0;
      for (k = 0; k < N; k++)
        r[i][j] += a[i][k] * b[k][j];
    }
}

void hartley(double **in, double **out, int N)
{
  int k, n;
  double **h;

  h = dmatrix(N, N);
  //Building up the transformation matrix
  for (k = 0; k < N; k++)
    for (n = 0; n < N; n++)
      h[k][n] = (cos(2 * PI * k * n / N) + sin(2 * PI * k * n / N)) / sqrt(N);

  // Now we have to multiply the input with the transformation matrix
  matmul(h, in, out, N);
  freematrix_d(h, N);
  return ;
}

double ** dmatrix(int nrows, int ncols)
{
  double **m;
  int i, j;
  m = (double **) malloc (nrows * sizeof(double *));
  for (i = 0; i < nrows; i++) {
    m[i] = (double *) malloc (ncols * sizeof(double));
    if (!m[i]) printf("\nIt's not working");
  }
  for (i = 0; i < nrows; i++)
    for (j = 0; j < ncols; j++)
      m[i][j] = 0.0;
  return m;
}

void freematrix_d(double **I, int rows)
{
  int k;
  for (k = 0; k < rows; k++)
    free (I[k]);
}

void matrix_i2d(int **i, double **d, int N)
{
  int x, y;
  for (x = 0; x < N; x++)
    for (y = 0; y < N; y++)
      d[y][x] = i[y][x];
}

void matrix_d2i(double **d, int **i, int N)
{
  int x, y;
  for (x = 0; x < N; x++)
    for (y = 0; y < N; y++)
      i[y][x] = d[y][x];
}

double * dvector(long int N)
{
  double *m;
  m = (double *) malloc (N * sizeof(double));
  if (!m) printf("\nIt's not working");
  return m;
}

void put_matrix_2_vector(double **i, double *f, int N)
{
  int l, j, k;
  k = 0;
  for (l = 0; l < N; l++)
    for (j = 0; j < N; j++)
      f[k++] = i[l][j];
}

void put_vector_2_matrix(double *f, double **i, int N)
{
  int l, j, k;
  k = 0;
  for (l = 0; l < N; l++)
    for (j = 0; j < N; j++)
      i[l][j] = f[k++];
}

void set_in_binary() {
#if defined(EMX)
  _fsetmode(in, "b");
#elif defined(MINGW)
  setmode(STDIN_FILENO, O_BINARY);
#endif
}

void set_out_binary() {
#if defined(EMX)  
  _fsetmode(out, "b");
#elif defined(MINGW)
  setmode(STDOUT_FILENO, O_BINARY);
#endif
}
 
void wm_init2() {
  set_out_binary();
}

void wm_init1() {
  set_in_binary();
}

void wm_init() {
  set_in_binary();
  set_out_binary();
}

