/* Watermarking program - Fast Cosine Transform based	*/
/* Module	: Casting				*/
/* Author	: Vassilis Fotopoulos			*/
/* Date		: 21/7/1999				*/
/* Revision	: 2.01a					*/
/* Developed at	: ELLAB                  		*/
/*                Electronics Laboratory           	*/
/*                Department of Physics             	*/
/*                University of Patras - GREECE      	*/
/*		  Copyleft (c) 1999	    		*/
/*  		  (without Visual Masking)              */
/*------------------------------------------------------*/
/*	pseudorandom noise generator's code is		*/
/*	taken from "Numerical Recipes in C"		*/
/*	FCT implementation from the University of Bath	*/
/*------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>
#include <getopt.h>
#include "common.h"
#include "pgm.h"

double cu[1024];
double cv[1024];
int height, width;

//--------------------------------------------------------
void add_watermark(double *in, int N, int coeff_start, int wm_length, int wm_key, double wm_alpha)
{
  int row, col, count;
  long int elem, L, M, temp, seed;
  double a;
  count = 0;
  elem = 0;
  row = 2;
  col = -1;
  M = coeff_start;
  L = wm_length;
  seed = wm_key;
  a = wm_alpha;
  do {
    do {
      row--;
      col++;
      elem++;
      if (col < N) {
        if (elem > M) {
          temp = row * N + col;
          in[temp] += a * fabs(in[temp]) * gasdev(&seed);
          count++;
        }
      }
    } while (row > 0);
    row = 2 + col;
    col = -1;
  } while (count < L);
}
//--------------------------------------------------------
void initialize_constants(void)
{
  int i;
  cu[0] = cv[0] = 0.7071068;
  for (i = 1; i < 1024; i++)
    cu[i] = cv[i] = 1.0;
}

//--------------------------------------------------------
int main(int argc, char* argv[])
{
  FILE *in, *out;
  int **image_i;
  double *image_f = NULL;
  int N;
  int c;
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
  image_i = imatrix(height, width);
  load_image(image_i, in, width, height);

  if (height == width)
    N = height;
  else {
    fprintf(stderr, "Cannot Proccess non-square images!\n");
    exit( -11);
  }

  initialize_constants();
  image_f = (double *)calloc(N * N, sizeof(double));
  if (image_f == NULL) {
    printf("Unable to allocate the float array\n");
    exit(1);
  }

  put_image_from_int_2_double(image_i, image_f, N);
  fct2d(image_f, N, N);
  add_watermark(image_f, N, coeff_start, wm_length, wm_key, wm_alpha);
  ifct2d(image_f, N, N);
  put_image_from_double_2_int(image_f, image_i, N);
  save_image(image_i, out, width, height);
  freematrix(image_i, height);
  free(image_f);
  fclose(in);
  fclose(out);

  exit(EXIT_SUCCESS);
}
