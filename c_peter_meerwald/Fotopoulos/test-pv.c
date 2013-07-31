/* Watermarking program - Fast Cosine Transform based	*/
/* Module	: Testing				*/
/* Author	: Vassilis Fotopoulos			*/
/* Date		: 21/7/1999				*/
/* Developed at	: ELLAB                  		*/
/*                Electronics Laboratory            	*/
/*                Department of Physics             	*/
/*                University of Patras - GREECE      	*/
/*		  Copyleft (c) 1999	    		*/
/*		  Testing Program       		*/
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

void read_watermark(double *in, int N, int start_coeff, int wm_length, double wm_alpha)
{
  int row, col, count;
  long int elem, L, M, temp, seed, i;
  double a;
  double z;
  M = start_coeff;
  L = wm_length;
  a = wm_alpha;
  for (i = 1; i <= 1000; i++) {
    seed = i;
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
            z += in[temp] * gasdev(&seed);
            count++;
          }
        }
      } while (row > 0);
      row = 2 + col;
      col = -1;
    } while (count < L);
    printf("%ld\t%f\n", i, z / L);
  }
  return ;
}
//--------------------------------------------------------
void initialize_constants(void)
{
  int i;
  cu[0] = cv[0] = 0.7071068;
  for (i = 1; i < 1024; i++)
    cu[i] = cv[i] = 1.0;
}

int main(int argc, char* argv[])
{
  FILE *in;
  int **image_i;
  double *image_f = NULL;
  int N;
  int c;
  int coeff_start = 5000, wm_length = 10000;
  double wm_alpha = 0.2;

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
  read_watermark(image_f, N, coeff_start, wm_length, wm_alpha);
  fclose(in);
  freematrix(image_i, height);
  free(image_f);

  exit(EXIT_SUCCESS);
}
