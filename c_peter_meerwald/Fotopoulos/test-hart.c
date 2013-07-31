/* Watermarking program - Hartley Transform based	*/
/* Module	: Testing				*/
/* Author	: Vassilis Fotopoulos			*/
/* Date		: 26/7/1999		      		*/
/* Developed at	: ELLAB                 		*/
/*                Electronics Laboratory            	*/
/*                Department of Physics             	*/
/*                University of Patras - GREECE      	*/
/*		  Copyleft (c) 1999             	*/
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

void read_watermark(double **in, int N, int coeff_start, int wm_length, double wm_alpha)
{
  int row, col, count;
  long int elem, L, M, seed, i;
  double a;
  double z;
  M = coeff_start;
  L = wm_length;
  a = wm_alpha;
  for (i = 1; i <= 1000; i++) {
    seed = i;
    z = 0.0;
    count = 0;
    elem = 0;

    for (row = 0; row < N; row++)
      for (col = 0; col < N; col++) {
        elem++;
        if (elem > M && count < L) {
          z += in[row][col] * gasdev(&seed);
          count++;
        }
      }

    printf("%ld\t%f\n", i, z / L);
  }
  return ;
}

int main(int argc, char* argv[])
{
  FILE *in;
  int **image;
  double **image_i;
  double **image_d;
  int c;
  int N;
  int coeff_start = 5000, wm_length = 10000;
  double wm_alpha = 0.2;
  int width, height;

  pgm_init(&argc, argv); wm_init2();

  while ((c = getopt(argc, argv, "a:s:l:")) != EOF) {
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
    }
  }
  argc -= optind;
  argv += optind;

  in = stdin;

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
  read_watermark(image_d, N, coeff_start, wm_length, wm_alpha);

  freematrix_d(image_i, height);
  freematrix_d(image_d, height);
  fclose(in);

  exit(EXIT_SUCCESS);
}
