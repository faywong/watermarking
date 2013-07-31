#include "wm.h"
#include "dct.h"
#include "pgm.h"
#include "signature.h"
#include "frid2_common.h"

char *progname;

void usage(void) {
  fprintf(stderr, "usage: %s [-a n] [-g n] [-n n] [-h] [-o file] [-v n] -s file file\n\n", progname);
  fprintf(stderr, "\t-a n\t\talpha factor low freq. embedding strength\n");
  fprintf(stderr, "\t-g n\t\tgamma factor med freq. embedding strength\n");
  fprintf(stderr, "\t-h\t\tprint usage\n");
  fprintf(stderr, "\t-n n\t\tnormalisation factor (default 1024.0)\n");
  fprintf(stderr, "\t-o file\t\toutput (watermarked) file\n");
  fprintf(stderr, "\t-s file\t\tsignature to embed in input image\n");
  fprintf(stderr, "\t-v n\t\tverbosity level\n");
  exit(0);
}

int main(int argc, char *argv[]) {

  FILE *in = stdin;
  FILE *out = stdout;
  FILE *sig = NULL;

  char output_name[MAXPATHLEN] = "(stdout)";
  char input_name[MAXPATHLEN] = "(stdin)";
  char signature_name[MAXPATHLEN];

  int c;
  int row, col;
  double normalization = 1024.0;
  int seed, format;

  double alpha = 0.0;
  double gamma = 0.0;

  gray **image;
  double **coeffs;

  double mean, derivation;
  double mult_factor;

  int rows, cols;
  int verbose = 0;
  gray maxval;

  progname = argv[0];

  pgm_init(&argc, argv); wm_init();

  while ((c = getopt(argc, argv, "a:g:n:h?o:s:v:")) != EOF) {
    switch (c) {
      case 'a':
        alpha = atof(optarg);
        if (alpha <= 0.0) {
          fprintf(stderr, "%s: alpha factor %f out of range\n", progname, alpha);
          exit(1);
        }
        break;
      case 'g':
        gamma = atof(optarg);
        if (gamma <= 0.0) {
          fprintf(stderr, "%s: gamma factor %f out of range\n", progname, alpha);
          exit(1);
        }
        break;
      case 'n':
        normalization = atof(optarg);
        if (normalization < 0) {
          fprintf(stderr, "%s: normalisation factor %f out of range\n", progname, normalization);
          exit(1);
        }
        break;
      case 'h':
      case '?':
        usage();
        break;
      case 'o':
        if ((out = fopen(optarg, "wb")) == NULL) {
          fprintf(stderr, "%s: unable to open output file %s\n", progname, optarg);
          exit(1);
        }
        strcpy(output_name, optarg);
        break;
      case 's':
        if ((sig = fopen(optarg, "r")) == NULL) {
          fprintf(stderr, "%s: unable to open signature file %s\n", progname, optarg);
          exit(1);
        }
        strcpy(signature_name, optarg);
        break;
      case 'v':
        verbose = atoi(optarg);
        if (verbose < 0) {
          fprintf(stderr, "%s: verbosity level %d out of range\n", progname, verbose);
          exit(1);
        }
        break;
    }
  }

  argc -= optind;
  argv += optind;

  if (argc > 1) {
    usage();
    exit(1);
  }

  if (argc == 1 && *argv[0] != '-') {
    if ((in = fopen(argv[0], "rb")) == NULL) {
      fprintf(stderr, "%s: unable to open input file %s\n", progname, argv[0]);
      exit(1);
    }
    else
      strcpy(input_name, argv[0]);
  }
  
  if (sig) {
    char line[32];
    fgets(line, sizeof(line), sig);
    if (strspn(line, "FR2SG") >= 5) {
      fscanf(sig, "%d\n", &nbit_signature);
      if (alpha == 0.0)
        fscanf(sig, "%lf\n", &alpha);
      else
        fscanf(sig, "%*f\n");
      if (gamma == 0.0)
        fscanf(sig, "%lf\n", &gamma);
      else
        fscanf(sig, "%*f\n");
      
      fscanf(sig, "%d\n", &seed);
      n_signature = NBITSTOBYTES(nbit_signature);
      fread(signature, sizeof(char), n_signature, sig);
      fscanf(sig, "\n");
      srandom(seed);
    }
    else {
      fprintf(stderr, "%s: invalid signature file %s\n", progname, signature_name);
      exit(1);
    }
    fclose(sig);
  }
  else {
    fprintf(stderr, "%s: signature file not specified, use -s file option\n", progname);
    exit(1);
  }

  pgm_readpgminit(in, &cols, &rows, &maxval, &format);

  if (rows == cols)
    init_dct_NxN(cols, rows);
  else
    init_dct_NxM(cols, rows);

  image = pgm_allocarray(cols, rows);
  for (row = 0; row < rows; row++)
    pgm_readpgmrow(in, image[row], cols, maxval, format);
  fclose(in);

  // calculate mean value
  mean = 0.0;
  for (row = 0; row < rows; row++)
    for (col = 0; col < cols; col++)
      mean += image[row][col];

  mean /= cols * rows;

  // calculate derivation
  derivation = 0.0;
  for (row = 0; row < rows; row++)
    for (col = 0; col < cols; col++)
      derivation += sqr(image[row][col] - mean);
    
  derivation = sqrt(derivation / (cols * rows - 1));
  mult_factor = normalization / (sqrt(cols * rows) * derivation); 

  if (verbose > 5)
    fprintf(stderr, "%s: mean %f, derivation %f, mult_factor %f\n", progname, mean, derivation, mult_factor);

  // normalize image
  coeffs = alloc_coeffs(cols, rows);
  for (row = 0; row < rows; row++)
    for (col = 0; col < cols; col++)
      coeffs[row][col] = (image[row][col] - mean) * mult_factor;
    
  if (cols == rows)
    fdct_inplace_NxN(coeffs);
//  else
//    fdct_NxM(image, dcts);    

  embed_low_freq(coeffs, cols, rows, alpha, verbose);
  embed_med_freq(coeffs, cols, rows, gamma, seed, verbose);
    
  if (cols == rows)
    idct_inplace_NxN(coeffs);
//  else
//    idct_NxM(dcts, image);

  for (row = 0; row < rows; row++)
    for (col = 0; col < cols; col++)
      image[row][col] = PIXELRANGE(coeffs[row][col] / mult_factor + mean + 0.5);
  
  free_coeffs(coeffs);

  pgm_writepgminit(out, cols, rows, maxval, 0);
  for (row = 0; row < rows; row++)
    pgm_writepgmrow(out, image[row], cols, maxval, 0);

  fclose(out);

  pgm_freearray(image, rows);

  exit(0);
}
