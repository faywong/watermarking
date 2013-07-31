#include "wm.h"
#include "dct.h"
#include "pgm.h"
#include "sort.h"

char *progname;

void usage(void) {
  fprintf(stderr, "usage: %s [-a n] [-h] [-n n] [-o file] [-s file] [-v n] -i file file\n\n", progname);
  fprintf(stderr, "\t-a n\t\talpha factor (default 0.3)\n");
  fprintf(stderr, "\t-h\t\tprint usage\n");
  fprintf(stderr, "\t-i file\t\toriginal image file\n");
  fprintf(stderr, "\t-n n\t\twatermark length (default 100)\n");
  fprintf(stderr, "\t-o file\t\textracted signature file\n");
  fprintf(stderr, "\t-s file\t\toriginal signature file\n");
  fprintf(stderr, "\t-v n\t\tverbosity level\n");
  exit(0);
}

int main(int argc, char *argv[]) {

  FILE *in = stdin;
  FILE *out = stdout;
  FILE *orig = NULL;
  FILE *sig = NULL;

  gray **input_image;
  gray **orig_image;

  char signature_name[MAXPATHLEN];
  char output_name[MAXPATHLEN] = "(stdout)";
  char input_name[MAXPATHLEN] = "(stdin)";
  char orig_name[MAXPATHLEN];

  int c;
  int i, j;
  int n = 100;

  double a = 0.3;

  int warn_n = 1;
  int warn_a = 1;

  int in_rows, in_cols, in_format;
  gray in_maxval;
  int orig_rows, orig_cols, orig_format;
  gray orig_maxval;
  int rows, cols;
  int row;

  double threshold;
  double *largest;

  double **input_dcts;
  double **orig_dcts;
  
  int verbose = 0;

  progname = argv[0];

  pgm_init(&argc, argv); wm_init2();

  while ((c = getopt(argc, argv, "h?i:n:o:s:v:")) != EOF) {
    switch (c) {
      case 'a':
        a = atof(optarg);
        if (a <= 0.0) {
          fprintf(stderr, "%s: alpha factor %f out of range\n", progname, a);
          exit(1);
        }
        warn_a = 0;
        break;
      case 'h':
      case '?':
        usage();
        break;
      case 'i':
        if ((orig = fopen(optarg, "rb")) == NULL) {
          fprintf(stderr, "%s: unable to open original image file %s\n", progname, optarg);
          exit(1);
        }
        strcpy(orig_name, optarg);
        break;
      case 'n':
        n = atoi(optarg);
        if (n < 1 || n > 1000) {
          fprintf(stderr, "%s: watermark length %d out of range\n", progname, n);
          exit(1);
        }
        warn_n = 0;
        break;
      case 'o':
        if ((out = fopen(optarg, "w")) == NULL) {
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
  
  if (!orig) {
    fprintf(stderr, "%s: original image file not specified, use -i file option\n", progname);
    exit(1);
  }

  if (sig) {
    char line[32];
    fgets(line, sizeof(line), sig);
    if (strspn(line, "CXSG") >= 4) {
      fscanf(sig, "%d\n", &n);
      if (warn_a)
        fscanf(sig, "%lf\n", &a);
      else
        fscanf(sig, "%*f\n");
      fscanf(sig, "%*f\n");
      fscanf(sig, "%*f\n");
    }
    else {
      fprintf(stderr, "%s: invalid signature file %s\n", progname, signature_name);
      exit(1);
    }
    fclose(sig);
  }
  else {
    if (warn_a)
      fprintf(stderr, "%s: warning - alpha factor not specified, using default %f\n", progname, a);
    if (warn_n)
      fprintf(stderr, "%s: warning - watermark length not specified, using default %d\n", progname, n);
  }

  pgm_readpgminit(in, &in_cols, &in_rows, &in_maxval, &in_format);
  pgm_readpgminit(orig, &orig_cols, &orig_rows, &orig_maxval, &orig_format);

  if (in_cols != orig_cols || in_rows != orig_rows) {
    fprintf(stderr, "%s: input image %s does not match dimensions of original image %s\n", progname, input_name, orig_name);
    exit(1);
  }

  cols = in_cols;
  rows = in_rows;

  init_dct_NxN(cols, rows);

  input_image = pgm_allocarray(in_cols, in_rows);

  orig_image = pgm_allocarray(orig_cols, orig_rows);

  for (row = 0; row < in_rows; row++)
    pgm_readpgmrow(in, input_image[row], in_cols, in_maxval, in_format);

  fclose(in);

  for (row = 0; row < orig_rows; row++)
    pgm_readpgmrow(orig, orig_image[row], orig_cols, orig_maxval, orig_format);

  fclose(orig);

  input_dcts = alloc_coeffs(cols, rows);
  orig_dcts = alloc_coeffs(cols, rows);

  fdct_NxN(input_image, input_dcts);
  fdct_NxN(orig_image, orig_dcts);

  largest = malloc((n + 1) * sizeof(double));
  select_largest_coeffs(orig_dcts[0], cols * rows, n+1, largest);
  threshold = largest[0];
  free(largest);

  fprintf(out, "CXWM\n");
  fprintf(out, "%d\n", n);

  j = 0;
  for (i = 0; i < n; i++) {
    double d, o, p;

    while ((o = orig_dcts[j / cols][j % cols]) < threshold) j++;

    p = input_dcts[j / cols][j % cols]; 
 
    d = (p / o - 1.0) / a;
    if (verbose >= 1)
      fprintf(stderr, "input %f orig %f alpha %f d %f\n", p, o, a, d);
    fprintf(out, "%f\n", d);
    j++;
  }

  fclose(out);

  free_coeffs(input_dcts);
  free_coeffs(orig_dcts);

  pgm_freearray(input_image, rows);
  pgm_freearray(orig_image, rows);

  exit(0);
}
