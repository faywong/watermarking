#include "frid2_common.h"
#include "signature.h"
#include "wm.h"

extern char *progname;

void embed_low_freq(double **dcts, int cols, int rows, double alpha, int verbose) {
  int n;
  int row, col, dir;

  n = 0;
  row = col = 0;
  dir = 1;
  while (n < nbit_signature) {
    double d, x;
    int embed;
    int out;

    col -= dir;
    row += dir;
    if (col < 0) { dir = -1; col = 0; }
    if (col >= cols) { dir = 1; col = cols - 1; row += 2; }
    if (row < 0) { dir = 1; row = 0; }
    if (row >= rows) { dir = -1; row = rows - 1; col += 2; }

    d = dcts[row][col];
    if (fabs(d) <= 1.0) {
      if (verbose > 3)
        fprintf(stderr, "%s: bit #%d - skipped (%d/%d)\n", progname, n, col, row);
      continue;
    }

    embed = 2 * get_signature_bit(n) - 1;

    x = (d > 0.0) ? 1.0 : -1.0;
    out = 1;
    while (fabs(x) < fabs(d)) {
      x *= FORWARD_STEP(alpha);
      out =- out;
    }

    if (out != embed) {
      if (fabs(d - x) < fabs(d - x * BACKWARD_STEP(alpha)))
        x *= FORWARD_STEP(alpha);
      else
        x *= BACKWARD_STEP(alpha);        
    }

    d =  (x + x * BACKWARD_STEP(alpha)) / 2.0;

    if (verbose > 3)
      fprintf(stderr, "%s: embedding bit #%d (= %d) at (%d/%d): %f -> %f\n", progname, n, get_signature_bit(n), col, row, dcts[row][col], d);

    dcts[row][col] = d;

    n++;
  }  
}        

void embed_med_freq(double **dcts, int cols, int rows, double gamma, int seed, int verbose) {
  // select mid-frequency (30%) coefficients
  int start = (int) (0.35 * rows * cols + 0.5);
  int end = (int) (0.65 * rows * cols + 0.5);

  double *vector;
  int x = 0, y = 0, dir = 1;
  int i, j;    

  vector = malloc((end - start) * sizeof(double));
  for (i = 0; i < (end - start); i++)
    vector[i] = 0.0;
        
  // create pseudo-random vector
  srandom(seed);
  for (i = 0; i < nbit_signature; i++) {
    if (get_signature_bit(i))
      random();
    for (j = 0; j < (end - start); j++)
      vector[j] += (double) (random() & RAND_MAX) / (double) RAND_MAX - 0.5;
    if (!get_signature_bit(i))
      random();
  }

  for (i = 0; i < (end - start); i++)
    vector[i] /= sqrt(nbit_signature);

  for (i = 0; i < end; i++) {
    // zig-zag scan
    x -= dir;
    y += dir;
    if (x < 0) { dir = -1; x = 0; }
    if (x >= cols) { dir = 1; x = cols - 1; y += 2; }
    if (y < 0) { dir = 1; y = 0; }
    if (y >= rows) { dir = -1; y = rows - 1; x += 2; }

    // embed vector
    if ((i - start) >= 0) {
//      fprintf(stderr, "%d/%d: %f -> %f\n", x, y, dcts[y][x], dcts[y][x] + gamma * vector[i - start]);
      dcts[y][x] += gamma * vector[i - start];
    }
  }        

  free(vector);
}

double detect_low_freq(double **dcts, int cols, int rows, double alpha, double beta, int verbose) {
  int n;
  int row, col, dir;
  double sum1, sum2;

  n = 0;
  row = col = 0;
  dir = 1;
  sum1 = sum2 = 0.0;
  while (n < nbit_signature1) {
    double d, x;
    int detect;
    int out;

    col -= dir;
    row += dir;
    if (col < 0) { dir = -1; col = 0; }
    if (col >= cols) { dir = 1; col = cols - 1; row += 2; }
    if (row < 0) { dir = 1; row = 0; }
    if (row >= rows) { dir = -1; row = rows - 1; col += 2; }

    d = dcts[row][col];
    if (fabs(d) <= 1.0) {
      if (verbose > 3)
        fprintf(stderr, "%s: bit #%d - skipped (%d/%d)\n", progname, n, col, row);
      continue;
    }

    detect = 2 * get_signature1_bit(n) - 1;

    x = (d > 0.0) ? 1.0 : -1.0;
    out = 1;
    while (fabs(x) < fabs(d)) {
      x *= FORWARD_STEP(alpha);
      out =- out;
    }

    if (verbose > 3)
      fprintf(stderr, "%s: detected bit #%d (= %d) at (%d/%d): %f\n", progname, n, out > 0 ? 1 : 0, col, row, d);

    set_signature2_bit(n, out > 0 ? 1 : 0);
    sum1 += pow(fabs(d), beta) * out * detect;
    sum2 += pow(fabs(d), beta);

    n++;
  }  

  return sum1 / sum2;
}

double detect_med_freq(double **dcts, int cols, int rows, int seed, int verbose) {
  int i, j, k;
  int start= (int) (0.35 * rows * cols + 0.5);
  int end = (int) (.65 * rows * cols + 0.5);

  int sum, sum1, sum2;
  int x = 0, y = 0, dir = 1;
  double *vector;
  int startx, starty, startdir;
  double corr[2];
  double correlation;

  // locate start positions 
  for (i = 0; i < start; i++) {
    x -= dir;
    y += dir;
    if (x < 0) { dir = -1; x = 0; }
    if (x >= cols) { dir = 1; x = cols - 1; y += 2; }
    if (y < 0) { dir = 1; y = 0; }
    if (y >= rows) { dir = -1; y = rows - 1; x += 2; }
  }		

  // save start positions
  startx = x;		
  starty = y;
  startdir = dir;
  srandom(seed);

  vector = malloc((end - start) * sizeof(double));

  for (i = 0; i < nbit_signature1; i++) {

    for (j = 0; j <= (end - start); j++)
      vector[j] = (double) (random() & RAND_MAX) / (double) RAND_MAX - 0.5;
				
    for (j = 0; j <= 1; j++) {
      x = startx;
      y = starty;
      dir = startdir;
      corr[j] = 0;

      for (k = 0; start + k < end; k++) {   
        x -= dir;
        y += dir;
        if (x < 0) { dir = -1; x = 0; }
        if (x >= cols) { dir = 1; x = cols - 1; y += 2; }
        if (y < 0) { dir = 1; y = 0; }
        if (y >= rows) { dir = -1; y = rows - 1; x += 2; }
        corr[j] += dcts[y][x] * vector[k + j];
      }
    }	

    set_signature2_bit(i,  (corr[0] >= corr[1]) ? 0 : 1);
  }

  sum = 0; sum1 = 0; sum2 = 0;
  for (i = 0; i < nbit_signature1; i++) {
    sum += get_signature1_bit(i) * get_signature2_bit(i);
    sum1 += get_signature1_bit(i) *  get_signature1_bit(i);
    sum2 += get_signature2_bit(i) * get_signature2_bit(i);
  }
  correlation = (double) sum / (sqrt(sum1) * sqrt(sum2));

  return correlation;		
}
