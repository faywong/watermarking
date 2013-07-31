/* Watermarking program - Hartley Transform based	*/
/* Module	: Casting				*/
/* Author	: Vassilis Fotopoulos			*/
/* Date		: 26/7/1999				*/
/* Revision	: 2.01a					*/
/* Developed at	: ELLAB                  		*/
/*                Electronics Laboratory           	*/
/*                Department of Physics             	*/
/*                University of Patras - GREECE      	*/
/*		  Copyleft (c) 1999			*/
/*------------------------------------------------------*/
/*	pseudorandom noise generator's code is		*/
/*	taken from "Numerical Recipes in C"		*/
/*------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>
#include <getopt.h>
#include "common.h"
#include "pgm.h"

int height, width;

void add_watermark(double **in, int N, int coeff_start, int wm_length, int wm_key, double wm_alpha)
{
  int row, col, count;
  long int elem, L, M, seed;
  double a;
  count = 0;
  elem = 0;
  M = coeff_start;
  L = wm_length;
  seed = wm_key;
  a = wm_alpha;
  for (row = 0; row < N; row++)
    for (col = 0; col < N; col++) {
      elem++;
      if (elem > M && count < L) {
        in[row][col] += a * fabs(in[row][col]) * gasdev(&seed);
        count++;
      }
    }

}

//--------------------------------------------------------
int main(int argc, char* argv[])
{
  FILE *in, *out;
  int **image;
  double **image_i;
  double **image_d;
  int c;
  int N;
  int coeff_start = 5000, wm_length = 10000, wm_key = 123;
  double wm_alpha = 0.2;

  pgm_init(&argc, argv); wm_init();

  while ((c = getopt(argc, argv, "a:s:l:k:")) != EOF) {
    switch (c) {
        case 'a':
        wm_alpha = atof(optarg);
        break;
        case 's':
        coeff_start = atoi(optarg);
        break;
        case 'l':
        wm_length = atoi(optarg);
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
  image = imatrix(height, width);
  load_image(image, in, width, height);

  if (height == width)
    N = height;
  else {
    fprintf(stderr, "Cannot Proccess non-square images!\n");
    exit( -11);
  }

  image_i = dmatrix(height, width);
  image_d = dmatrix(height, width);
  if (image_d == NULL) {
    fprintf(stderr, "Unable to allocate the double array\n");
    exit(1);
  }
  matrix_i2d(image, image_i, N);
  hartley(image_i, image_d, N);
  add_watermark(image_d, N, coeff_start, wm_length, wm_key, wm_alpha);
  hartley(image_d, image_i, N);
  matrix_d2i(image_i, image, N);
  save_image(image, out, width, height);

  freematrix_d(image_i, height);
  freematrix_d(image_d, height);
  fclose(in);
  fclose(out);
  exit(EXIT_SUCCESS);
}
