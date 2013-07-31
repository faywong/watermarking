#include "wm.h"
#include "ppm.h"

#define LUMINANCE 1
#define RED 2
#define GREEN 4
#define BLUE 8

char *progname;

void usage(void) {
  fprintf(stderr, "usage: %s -c name [-C] [-h] [-m n] [-o file] [-pP] -i file file\n\n", progname);
  fprintf(stderr, "\t-c name\t\tcolor component (default luminance)\n");
  fprintf(stderr, "\t\t\teg. red, green, blue, luminance\n");
  fprintf(stderr, "\t-C\t\tprint PSNR value only\n");
  fprintf(stderr, "\t-h\t\tprint usage\n");
  fprintf(stderr, "\t-i file\t\toriginal image file\n");
  fprintf(stderr, "\t-m n\t\tmultiplication factor (default 16)\n");
  fprintf(stderr, "\t-o file\t\toutput file\n");
  fprintf(stderr, "\t-p\t\tprint PSNR, RMS and MSE\n");
  fprintf(stderr, "\t-P\t\tonly print PSNR, RMS and MSE, no difference image\n");
  exit(0);
}

int main(int argc, char *argv[]) {

  FILE *in = stdin;
  FILE *out = stdout;
  FILE *orig = NULL;

  pixel **input_image;
  pixel **orig_image;

  char output_name[MAXPATHLEN] = "(stdout)";
  char input_name[MAXPATHLEN] = "(stdin)";
  char orig_name[MAXPATHLEN];

  int in_cols, in_rows, in_format;
  pixval in_maxval;
  int orig_cols, orig_rows, orig_format;
  pixval orig_maxval;
  int cols, rows, format;
  pixval maxval;
  int col, row;

  int c;

  double error = 0.0;
  int print_psnr = 0;
  int print_psnr_only = 0;
  int print_psnr_value_only = 0;

  int m = 16;
  int component = 0;
  int component_default = LUMINANCE;
  int min, max;

  progname = argv[0];

  ppm_init(&argc, argv);

#ifdef __EMX__
  _fsetmode(in, "b");
  _fsetmode(out, "b");
#endif

  while ((c = getopt(argc, argv, "Cc:h?i:m:o:pP")) != EOF) {
    switch (c) {
      case 'C':
        print_psnr_value_only = 1;
        print_psnr_only = 1;
        break;
      case 'c':
        if (!strcasecmp(optarg, "RED") || toupper(*optarg) == 'R')
          component |= RED;
        else if (!strcasecmp(optarg, "GREEN") || toupper(*optarg) == 'G')
          component |= GREEN;
        else if (!strcasecmp(optarg, "BLUE") || toupper(*optarg) == 'B')
          component |= BLUE;
        else if (!strcasecmp(optarg, "LUMINANCE") || toupper(*optarg) == 'L')
          component |= LUMINANCE;
        else
          fprintf(stderr, "%s: unknown color component %s\n", progname, optarg);
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

  if (!component) {
    if (component_default)
      component = component_default;
    else {
      fprintf(stderr, "%s: color component(s) to compare not specified, use -c name option\n", progname);
      exit(1);
    }
  }

  if (component & LUMINANCE && component & (RED | GREEN | BLUE)) {
    fprintf(stderr, "%s: unable to compare luminance AND color component\n", progname);
    exit(1);
  }

  ppm_readppminit(in, &in_cols, &in_rows, &in_maxval, &in_format);

  ppm_readppminit(orig, &orig_cols, &orig_rows, &orig_maxval, &orig_format);

  if (in_cols != orig_cols || in_rows != orig_rows) {
    fprintf(stderr, "%s: input image %s does not match dimensions of original image %s\n", progname, input_name, orig_name);
    exit(1);
  }

  cols = in_cols;
  rows = in_rows;
  format = in_format;
  maxval = in_maxval;

  input_image = ppm_allocarray(cols, rows);

  for (row = 0; row < rows; row++)
      ppm_readppmrow(in, input_image[row], cols, in_maxval, in_format);

  fclose(in);

  orig_image = ppm_allocarray(cols, rows);

  for (row = 0; row < rows; row++)
      ppm_readppmrow(orig, orig_image[row], cols, orig_maxval, orig_format);

  fclose(orig);

  if (component & LUMINANCE)
    min = max = abs(PPM_LUMIN(input_image[0][0]) - PPM_LUMIN(orig_image[0][0]));
  else {
    if (component & RED)
      min = max = abs(PPM_GETR(input_image[0][0]) - PPM_GETR(orig_image[0][0]));
    else if (component & GREEN)
      min = max = abs(PPM_GETG(input_image[0][0]) - PPM_GETG(orig_image[0][0]));
    else if (component & BLUE)
      min = max = abs(PPM_GETB(input_image[0][0]) - PPM_GETB(orig_image[0][0]));
    else
      min = max = 0;
  }

  for (row = 0; row < rows; row++) {
    pixel *pi = input_image[row];
    pixel *po = orig_image[row];

    for (col = 0; col < cols; col++) {
      int diff=0;

      if (component & LUMINANCE) {
        pixval l;
        diff = abs(PPM_LUMIN(*pi) - PPM_LUMIN(*po));
        error += sqr(PPM_LUMIN(*pi) - PPM_LUMIN(*po));
        l = PIXELRANGE(ROUND(abs(PPM_LUMIN(*pi) - PPM_LUMIN(*po)) * m));
        PPM_ASSIGN(*pi, l, l, l);
      }
      else {
        if (component & RED) {
          diff = abs(PPM_GETR(*pi) - PPM_GETR(*po));
          error += sqr(PPM_GETR(*pi) - PPM_GETR(*po));
          PPM_PUTR(*pi, PIXELRANGE(abs(PPM_GETR(*pi) - PPM_GETR(*po)) * m));
        }
        else 
          PPM_PUTR(*pi, 0);
        if (component & GREEN) {
          diff = abs(PPM_GETG(*pi) - PPM_GETG(*po));
          error += sqr(PPM_GETG(*pi) - PPM_GETG(*po));
          PPM_PUTG(*pi, PIXELRANGE(abs(PPM_GETG(*pi) - PPM_GETG(*po)) * m));
        }
        else 
          PPM_PUTG(*pi, 0);
        if (component & BLUE) {
          diff = abs(PPM_GETB(*pi) - PPM_GETB(*po));
          error += sqr(PPM_GETB(*pi) - PPM_GETB(*po));
          PPM_PUTB(*pi, PIXELRANGE(abs(PPM_GETB(*pi) - PPM_GETB(*po)) * m));
        }
        else
          PPM_PUTB(*pi, 0);
      }

      if (diff < min) min = diff;
      if (diff > max) max = diff;
      pi++;
      po++;
    }
  }

  if (!print_psnr_only) {
    ppm_writeppminit(out, cols, rows, maxval, 0);
    for (row = 0; row < rows; row++)
      ppm_writeppmrow(out, input_image[row], cols, maxval, 0);

    fclose(out);
  }

  ppm_freearray(input_image, rows);
  ppm_freearray(orig_image, rows);

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
      fprintf(print, "RMS: %lf\n", rmse);
      fprintf(print, "MSE: %lf\n", mse);
      fprintf(print, "dmin, dmax: %d, %d\n", min, max);
    }
    else
      fprintf(print, "%lf\n", mse > 0.0 ? psnr : 100.0);
  }

  exit(0);
}
