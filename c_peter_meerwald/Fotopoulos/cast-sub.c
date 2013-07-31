/* Watermarking program - Subband DCT Transform based	*/
/* Module	: Casting				*/
/* Author	: Vassilis Fotopoulos			*/
/* Date		: 25/4/2000				*/
/* Revision	: 7.0					*/
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

void add_hor_add_ver(double **in, int N, double **out);
void add_hor_sub_ver(double **in, int N, double **out);
void sub_hor_add_ver(double **in, int N, double **out);
void sub_hor_sub_ver(double **in, int N, double **out);
void band_synthesis(double **ll, double **lh, double **hl, double **hh, int N, double **s);
void watermark(double **i, int N, long key, long int L, long int M, double a);

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
//--------------------------------------------------------
void band_synthesis(double **ll, double **lh, double **hl, double **hh, int N, double **s)
{
  int r, c;
  double b1, b2, b3, b4;
  for (r = 0; r < N; r++)
    for (c = 0; c < N; c++) {
      b1 = ll[r][c] + lh[r][c] + hl[r][c] + hh[r][c]; 	//Reconstruct each
      b2 = ll[r][c] + lh[r][c] - hl[r][c] - hh[r][c]; 	//of the pixels
      b3 = ll[r][c] - lh[r][c] + hl[r][c] - hh[r][c];
      b4 = ll[r][c] - lh[r][c] - hl[r][c] + hh[r][c];
      b1 = (b1 > 255.0) ? 255.0 : b1; 					//Check for positive...
      b1 = (b1 < 0.0) ? 0.0 : b1; 						//or negative core!
      b2 = (b2 > 255.0) ? 255.0 : b2;
      b2 = (b2 < 0.0) ? 0.0 : b2;
      b3 = (b3 > 255.0) ? 255.0 : b3;
      b3 = (b3 < 0.0) ? 0.0 : b3;
      b4 = (b4 > 255.0) ? 255.0 : b4;
      b4 = (b4 < 0.0) ? 0.0 : b4;
      s[2*r][2*c] = b1; 							//Put them back in
      s[2*r][2*c + 1] = b2; 						//the right position
      s[2*r + 1][2*c] = b3;
      s[2*r + 1][2*c + 1] = b4;
    }
}

void watermark(double **i, int N, long key, long int L, long int M, double a)
{
  int row, col, count;
  long int elem, temp, seed;
  double *v;
  v = dvector(N * N);
  put_matrix_2_vector(i, v, N);
  fct2d(v, N, N);
  seed = key;
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
          v[temp] += a * fabs(v[temp]) * gasdev(&seed);
          count++;
        }
      }
    } while (row > 0);
    row = 2 + col;
    col = -1;
  } while (count < L);
  ifct2d(v, N, N);
  put_vector_2_matrix(v, i, N);
  free(v);
}

int main(int argc, char* argv[])
{
  double **i;
  double **o;
  FILE *in;
  FILE *out;
  int N;
  int c;
  long int M1, L1, M2, L2;
  int **image_i;
  double **ll, **lh, **hl, **hh;
  double a_ll = 0.1, a_other = 0.2;
  int wm_length_1 = 10000, wm_length_ll = 10000, coeff_start_1 = 3000,
                                          coeff_start_ll = 3000, wm_key = 123;

  pgm_init(&argc, argv); wm_init();

  while ((c = getopt(argc, argv, "a:b:t:m:s:l:k:")) != EOF) {
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
        case 'k':
        wm_key = atoi(optarg);
        break;
    }
  }
  argc -= optind;
  argv += optind;

  in = stdin;
  out = stdout;
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

  // now the LL band
  M2 = coeff_start_ll;
  L2 = wm_length_ll;

  i = dmatrix(N, N);
  o = dmatrix(N, N);
  ll = dmatrix(N / 2, N / 2);
  lh = dmatrix(N / 2, N / 2);
  hl = dmatrix(N / 2, N / 2);
  hh = dmatrix(N / 2, N / 2);

  matrix_i2d(image_i, i, N);

  //---------------------1o decomposition-------------------
  add_hor_add_ver(i, N, ll);
  add_hor_sub_ver(i, N, lh);
  sub_hor_add_ver(i, N, hl);
  sub_hor_sub_ver(i, N, hh);
  //---------------------Watermark medium frequency bands---
  watermark(lh, N / 2, wm_key, L1, M1, a_other);
  watermark(hl, N / 2, wm_key, L1, M1, a_other);
  watermark(hh, N / 2, wm_key, L1, M1, a_other);
  watermark(ll, N / 2, wm_key, L2, M2, a_ll);
  //---------------------Synthesis stage--------------------
  band_synthesis(ll, lh, hl, hh, N / 2, o);
  //--------------------------------------------------------
  matrix_d2i(o, image_i, N);
  save_image(image_i, out, width, height);

  fclose(in);
  fclose(out);

  exit(EXIT_SUCCESS);
}




