/* Watermarking program - Subband DCT Transform based	*/
/* Module	: Testing				*/
/* Author	: Vassilis Fotopoulos			*/
/* Date		: 25/4/2000				*/
/* Revision	: 6.0					*/
/* Developed at	: ELLAB                  		*/
/*                Electronics Laboratory           	*/
/*                Department of Physics             	*/
/*                University of Patras - GREECE      	*/
/*		  Copyleft (c) 1999	    		*/
/*------------------------------------------------------*/
/*	pseudorandom noise generator's code is		*/
/*	taken from "Numerical Recipes in C"		*/
/*	FCT implementation from University of Bath	*/
/*------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>
#include <getopt.h>
#include "common.h"
#include "pgm.h"

//--------------------------------------------------------
void add_hor_add_ver(double **in, int N, double **out);
void add_hor_sub_ver(double **in, int N, double **out);
void sub_hor_add_ver(double **in, int N, double **out);
void sub_hor_sub_ver(double **in, int N, double **out);
double detect_mark(double *i, int N, long key, long int L, long int M, double a);

//--------------------------------------------------------
double cu[1024];
double cv[1024];
int height, width;
//--------------------------------------------------------
void add_hor_add_ver(double **in, int N, double **out)
{
  double **temp;
  int r, c;
  temp = dmatrix(N, N);
  for (r = 0; r < N; r++)
    for (c = 0; c < N / 2; c++)
      temp[r][c] = (in[r][2 * c] + in[r][2 * c + 1]) / 2;
  for (c = 0; c < N / 2; c++)
    for (r = 0; r < N / 2; r++)
      out[r][c] = (temp[2 * r][c] + temp[2 * r + 1][c]) / 2;
  freematrix_d(temp, N);
}
//--------------------------------------------------------
void add_hor_sub_ver(double **in, int N, double **out)
{
  double **temp;
  int r, c;
  temp = dmatrix(N, N);
  for (r = 0; r < N; r++)
    for (c = 0; c < N / 2; c++)
      temp[r][c] = (in[r][2 * c] + in[r][2 * c + 1]) / 2;
  for (c = 0; c < N / 2; c++)
    for (r = 0; r < N / 2; r++)
      out[r][c] = (temp[2 * r][c] - temp[2 * r + 1][c]) / 2;
  freematrix_d(temp, N);
}
//--------------------------------------------------------
void sub_hor_add_ver(double **in, int N, double **out)
{
  double **temp;
  int r, c;
  temp = dmatrix(N, N);
  for (r = 0; r < N; r++)
    for (c = 0; c < N / 2; c++)
      temp[r][c] = (in[r][2 * c] - in[r][2 * c + 1]) / 2;
  for (c = 0; c < N / 2; c++)
    for (r = 0; r < N / 2; r++)
      out[r][c] = (temp[2 * r][c] + temp[2 * r + 1][c]) / 2;
  freematrix_d(temp, N);
}
//--------------------------------------------------------
void sub_hor_sub_ver(double **in, int N, double **out)
{
  double **temp;
  int r, c;
  temp = dmatrix(N, N);
  for (r = 0; r < N; r++)
    for (c = 0; c < N / 2; c++)
      temp[r][c] = (in[r][2 * c] - in[r][2 * c + 1]) / 2;
  for (c = 0; c < N / 2; c++)
    for (r = 0; r < N / 2; r++)
      out[r][c] = (temp[2 * r][c] - temp[2 * r + 1][c]) / 2;
  freematrix_d(temp, N);
}

//---------------------------------------------------------
double detect_mark(double *i, int N, long key, long int L, long int M, double a)
{
  int row, col, count;
  long int elem, temp, seed;
  double z;


  seed = key;
  z = 0.0;
  count = 0;
  elem = 0;
  row = 2;
  col = -1;
  do {
    do {
      row--;
      col++;
      elem++;
      if (col < N) {
        if (elem > M) {
          temp = row * N + col;
          z += i[temp] * gasdev(&seed);
          count++;
        }
      }
    } while (row > 0);
    row = 2 + col;
    col = -1;
  } while (count < L);

  return (z / L);
}

int main(int argc, char* argv[])
{
  double **i;
  FILE *in;
  int N;
  long int key, M1, L1, M2, L2;
  double **ll, **lh, **hl, **hh;
  double *v1, *v2, *v3, *v99;
  double m1, m2, m3, detect_value, m99;
  int ** image_i;
  int c;
  int wm_length_1 = 10000, wm_length_ll = 10000, coeff_start_1 = 3000, coeff_start_ll = 3000;
  double a_ll = 0.1, a_other = 0.2;

  pgm_init(&argc, argv); wm_init2();

  while ((c = getopt(argc, argv, "a:b:t:m:s:l:")) != EOF) {
    switch (c) {
        case 'a':
        a_ll = atof(optarg);
        break;
        case 'b':
        a_other = atof(optarg);
        break;
        case 't':
        coeff_start_1 = atoi(optarg);
        break;
        case 'm':
        wm_length_1 = atoi(optarg);
        break;
        case 's':
        coeff_start_ll = atoi(optarg);
        break;
        case 'l':
        wm_length_ll = atoi(optarg);
        break;
    }
  }
  argc -= optind;
  argv += optind;

  in = stdin;
  open_image(in, &width, &height);
  image_i = imatrix(height, width);
  load_image(image_i, in, width, height);

  if (height == width)
    N = height;
  else {
    fprintf(stderr, "Cannot Proccess non-square images!\n");
    exit( -11);
  }
  // starting coeff. for 1st level decomp.
  M1 = coeff_start_1;
  // number of coeffs to alter
  L1 = wm_length_1;
  // alpha parameter

  // now the LL band
  M2 = coeff_start_ll;
  L2 = wm_length_ll;


  i = dmatrix(N, N);
  ll = dmatrix(N / 2, N / 2);
  lh = dmatrix(N / 2, N / 2);
  hl = dmatrix(N / 2, N / 2);
  hh = dmatrix(N / 2, N / 2);
  v1 = dvector(N * N / 4);
  v2 = dvector(N * N / 4);
  v3 = dvector(N * N / 4);
  v99 = dvector(N * N / 4);

  matrix_i2d(image_i, i, N);

  //---------------------1o decomposition-------------------
  add_hor_add_ver(i, N, ll);
  add_hor_sub_ver(i, N, lh);
  sub_hor_add_ver(i, N, hl);
  sub_hor_sub_ver(i, N, hh);
  //---------------------Detect Watermark from all bands----
  put_matrix_2_vector(lh, v1, N / 2);
  put_matrix_2_vector(hl, v2, N / 2);
  put_matrix_2_vector(hh, v3, N / 2);
  put_matrix_2_vector(ll, v99, N / 2);
  fct2d(v1, N / 2, N / 2);
  fct2d(v2, N / 2, N / 2);
  fct2d(v3, N / 2, N / 2);
  fct2d(v99, N / 2, N / 2);
  for (key = 1; key <= 1000; key++) {
    m1 = detect_mark(v1, N / 2, key, L1, M1, a_other);
    m2 = detect_mark(v2, N / 2, key, L1, M1, a_other);
    m3 = detect_mark(v3, N / 2, key, L1, M1, a_other);
    m99 = detect_mark(v99, N / 2, key, L2, M2, a_ll);
    detect_value = (m1 + m2 + m3 + m99) / 4;
    printf("%ld\t%f\t%f\t%f\t%f\t%f\n", key, m1, m2, m3, m99, detect_value);
  }
  //--------------------------------------------------------
  free(v1);
  free(v2);
  free(v3);
  free(v99);
  freematrix_d(ll, N / 2);
  freematrix_d(hl, N / 2);
  freematrix_d(lh, N / 2);
  freematrix_d(hh, N / 2);
  fclose(in);

  exit(EXIT_SUCCESS);
}




