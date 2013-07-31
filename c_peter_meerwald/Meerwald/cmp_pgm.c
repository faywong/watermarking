#include "wm.h"
#include "pgm.h"

char *progname;

void usage(void) {
  fprintf(stderr, "usage: %s [-C] [-h] [-m n] [-o file] [-pP] -i file file\n\n", progname);
  fprintf(stderr, "\t-C\t\tprint PSNR value only\n");
  fprintf(stderr, "\t-h\t\tprint usage\n");
  fprintf(stderr, "\t-i file\t\toriginal image file\n");
  fprintf(stderr, "\t-m n\t\tmultiplication factor (default auto)\n");
  fprintf(stderr, "\t-o file\t\toutput file for difference image\n");
  fprintf(stderr, "\t-p\t\tprint PSNR, RMS and MSE\n");
  fprintf(stderr, "\t-P\t\tonly print PSNR, RMS and MSE, no difference image\n");
  exit(0);
}

int main(int argc, char *argv[]) {

  FILE *in = stdin;
  FILE *out = stdout;
  FILE *orig = NULL;

  gray **input_image;
  gray **orig_image;

  char output_name[MAXPATHLEN] = "(stdout)";
  char input_name[MAXPATHLEN] = "(stdin)";
  char orig_name[MAXPATHLEN];

  int in_cols, in_rows, in_format;
  gray in_maxval;
  int orig_cols, orig_rows, orig_format;
  gray orig_maxval;
  int cols, rows, format;
  gray maxval;
  int col, row;

  int c;

  int m = 0;
  int min, max;

  int print_psnr = 0;
  int print_psnr_only = 0;
  int print_psnr_value_only = 0;
  double error = 0.0;

  progname = argv[0];

  pgm_init(&argc, argv); wm_init();

  while ((c = getopt(argc, argv, "h?i:m:o:pPC")) != EOF) {
    switch (c) {
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
      case 'm':
        m = atoi(optarg);
        if (m <= 0) {
          fprintf(stderr, "%s: multiplication factor %d out of range\n", progname, m);
          exit(1);
        }
        break;
      case 'o':
        if ((out = fopen(optarg, "wb")) == NULL) {
          fprintf(stderr, "%s: unable to open output file %s\n", progname, optarg);
          exit(1);
        }
        strcpy(output_name, optarg);
        break;
      case 'p':
        print_psnr = 1;
        break;
      case 'P':
        print_psnr_only = 1;
        break;
      case 'C':
        print_psnr_value_only = 1;
        print_psnr_only = 1;
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

  pgm_readpgminit(in, &in_cols, &in_rows, &in_maxval, &in_format);

  pgm_readpgminit(orig, &orig_cols, &orig_rows, &orig_maxval, &orig_format);

  if (in_cols != orig_cols || in_rows != orig_rows) {
    fprintf(stderr, "%s: input image %s does not match dimensions of original image %s\n", progname, input_name, orig_name);
    exit(1);
  }

  cols = in_cols;
  rows = in_rows;
  format = in_format;
  maxval = in_maxval;

  input_image = pgm_allocarray(cols, rows);
  orig_image = pgm_allocarray(cols, rows);

  for (row = 0; row < rows; row++) {
      pgm_readpgmrow(in, input_image[row], cols, in_maxval, in_format);
      pgm_readpgmrow(orig, orig_image[row], cols, orig_maxval, orig_format);
  }

  fclose(in);
  fclose(orig);

  min = max = abs(input_image[0][0] - orig_image[0][0]);

  for (row = 0; row < rows; row++) {
    gray *pi = input_image[row];
    gray *po = orig_image[row];

    for (col = 0; col < cols; col++) {
      int diff = abs(*pi - *po);
      error += sqr(diff);
      if (diff < min) min = diff;
      if (diff > max) max = diff;

      pi++;
      po++;
    }
  }

  for (row = 0; row < rows; row++) {
    gray *pi = input_image[row];
    gray *po = orig_image[row];

    for (col = 0; col < cols; col++) {
      int diff = abs(*pi - *po);
      if (m > 0)
        *pi = PIXELRANGE(diff * m);
      else
        *pi = PIXELRANGE((double) (diff - min) / (double) (max - min) * maxval);

      pi++;
      po++;
    }
  }

  if (!print_psnr_only) {
    pgm_writepgminit(out, cols, rows, maxval, 0);
    for (row = 0; row < rows; row++)
      pgm_writepgmrow(out, input_image[row], cols, maxval, 0);

    fclose(out);
  }

  pgm_freearray(input_image, rows);
  pgm_freearray(orig_image, rows);

  if (print_psnr || print_psnr_only) {
    double mse = error / (double) (cols * rows);
    double rmse = sqrt(mse);
    double psnr = 20.0 * log(255.0 / rmse) / log(10.0);
    FILE *print = print_psnr_only ? out : stderr;
    if (!print_psnr_value_only) {
      if (mse > 0.0) 
        fprintf(print, "PSNR: %lf dB\n", psnr);
      else 
        fprintf(print, "PSNR: inf\n");
      fprintf(print, "RMSE: %lf\n", rmse);
      fprintf(print, "MSE: %lf\n", mse);
      fprintf(print, "dmin, dmax: %d, %d\n", min, max);
    }
    else
      fprintf(print, "%lf\n", mse > 0.0 ? psnr : 100.0);
  }

  exit(0);
}
