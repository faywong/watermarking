#include "wm.h"
#include "dct.h"
#include "pgm.h"
#include "sort.h"

char *progname;

void usage(void) {
  fprintf(stderr, "usage: %s [-a n] [-h] [-o file] -s file file\n\n", progname);
  fprintf(stderr, "\t-a n\t\talpha factor/embedding strength\n");
  fprintf(stderr, "\t-h\t\tprint usage\n");
  fprintf(stderr, "\t-o file\t\toutput (watermarked) file\n");
  fprintf(stderr, "\t-s file\t\tsignature to embed in input image\n");
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
  int row;
  int i,j;

  int n;

  double alpha = 0.0;
  double threshold;

  double *largest;
  gray **input_image;
  gray **output_image;
  double **dcts;

  gray maxval;
  int rows, cols, format;

  progname = argv[0];

  pgm_init(&argc, argv); wm_init();

  while ((c = getopt(argc, argv, "a:h?o:s:")) != EOF) {
    switch (c) {
      case 'a':
        alpha = atof(optarg);
        if (alpha <= 0.0) {
          fprintf(stderr, "%s: alpha factor %f out of range\n", progname, alpha);
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
    if (strspn(line, "CXSG") >= 4) {
      fscanf(sig, "%d\n", &n);
      if (alpha == 0.0)
        fscanf(sig, "%lf\n", &alpha);
      else
        fscanf(sig, "%*f\n");
      fscanf(sig, "%*f\n");
      fscanf(sig, "%*f\n");
    }
    else {
      fprintf(stderr, "%s: invalid signature file %s\n", progname, signature_name);
      exit(1);
    }
  }
  else {
    fprintf(stderr, "%s: signature file not specified, use -s file option\n", progname);
    exit(1);
  }

  pgm_readpgminit(in, &cols, &rows, &maxval, &format);

  init_dct_NxN(cols, rows);

  dcts = alloc_coeffs(cols, rows);
  input_image = pgm_allocarray(cols, rows);

  for (row = 0; row < rows; row++)
    pgm_readpgmrow(in, input_image[row], cols, maxval, format);

  fclose(in);

  output_image = pgm_allocarray(cols, rows);

  fdct_NxN(input_image, dcts);

  largest = malloc((n + 1) * sizeof(double));
  select_largest_coeffs(dcts[0], cols * rows, n+1, largest);
  threshold = largest[0];
  free(largest);

  j = 0;
  for (i = 0; i < n; i++) {
    double v;

    while (dcts[j / cols][j % cols] < threshold) j++;

    fscanf(sig, "%lf\n", &v);
    dcts[j / cols][j % cols] *= (1.0 + alpha * v);
    j++;
  }

  idct_NxN(dcts, output_image);
  free_coeffs(dcts);

  pgm_writepgminit(out, cols, rows, maxval, 0);

  for (row = 0; row < rows; row++)
    pgm_writepgmrow(out, output_image[row], cols, maxval, 0);

  fclose(out);

  fclose(sig);

  pgm_freearray(output_image, rows);
  pgm_freearray(input_image, rows);

  exit(0);
}
