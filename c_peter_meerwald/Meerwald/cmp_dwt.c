#include "wm.h"
#include "dwt.h"
#include "coeff.h"
#include "gray.h"
#include "pgm.h"
#include "dwt_util.h"

char *progname;

void usage(void) {
  fprintf(stderr, "usage: %s [-e n] [-f n] [-F file] [-h] [-l n] [-o file] [-pP] [-s name,..] -i file file\n\n", progname);
  fprintf(stderr, "\t-e n\t\twavelet filtering method (default: 2)\n");
  fprintf(stderr, "\t-f n\t\tfilter number (default: 2)\n");
  fprintf(stderr, "\t-F file\t\tfilter definition file (default: filter.dat\n");
  fprintf(stderr, "\t-h\t\tprint usage\n");
  fprintf(stderr, "\t-i file\t\toriginal image file\n");
  fprintf(stderr, "\t-l n\t\tmax. decomposition level (default: 0 = max)\n");
  fprintf(stderr, "\t-m n\t\tmultiplication factor (default: 0 = auto)\n");
  fprintf(stderr, "\t-o file\t\toutput file\n");
  fprintf(stderr, "\t-p\t\tprint PSNR, RMS and MSE\n");
  fprintf(stderr, "\t-P\t\tonly print PSNR, RMS and MSE, no difference image\n");
  fprintf(stderr, "\t-q\t\tprint PSNR, RMS and MSE per subband\n");
  fprintf(stderr, "\t-s name,...\tsubband/level (default: all subbands)\n");
  fprintf(stderr, "\t-v n\t\tverbosity level\n");
  exit(0);
}

void process_subband_var(gray **output, int cols, int rows, Image_tree p, Image_tree q, int type, double min, double max, gray maxval) {
  int col, row, startcol, startrow;

  if (!p || !q) return;

  calc_subband_location(cols, rows, type, p->level, &startcol, &startrow);

  for (row = 0; row < p->image->height; row++)
    for (col = 0; col < p->image->width; col++) {
      double diff = fabs(get_pixel(p->image, col, row) - get_pixel(q->image, col, row));
      output[startrow + row][startcol + col] = PIXELRANGE((double) (diff - min) / (double) (max - min) * maxval);
    }
}

void process_subband_fixed(gray **output, int cols, int rows, Image_tree p, Image_tree q, int type, double m) {
  int col, row, startcol, startrow;

  if (!p || !q) return;

  calc_subband_location(cols, rows, type, p->level, &startcol, &startrow);

  for (row = 0; row < p->image->height; row++)
    for (col = 0; col < p->image->width; col++) {
      double diff = fabs(get_pixel(p->image, col, row) - get_pixel(q->image, col, row));
      output[startrow + row][startcol + col] = PIXELRANGE(diff * m);
    }
}

void print_subband_psnr(int type, int level, double error, int cols, int rows, double min, double max, FILE *print) {
  double mse = error / (double) (cols * rows);
  double rmse = sqrt(mse);
  double psnr = 20.0 * log(255.0 / rmse) / log(10.0);
  int startcol, startrow;

  calc_subband_location(cols << level, rows << level, type, level, &startcol, &startrow);
  fprintf(print, "%s%d (%d x %d) at %d x %d\n", subband_name(type), level, cols, rows, startcol, startrow);
  if (mse > 0.0)
    fprintf(print, "  PSNR: %lf dB\n", psnr);
  else
    fprintf(print, "  PSNR: inf\n");
  fprintf(print, "  RMS: %lf\n", rmse);
  fprintf(print, "  MSE: %lf\n", mse);
  fprintf(print, "  dmin, dmax: %lf, %lf\n", min, max);
}

int main(int argc, char *argv[]) {
  FILE *in = stdin;
  FILE *out = stdout;
  FILE *orig = NULL;
  FILE *print;

  gray **input_image;
  gray **orig_image;
  Image_tree input_dwts, p;
  Image_tree orig_dwts, q;
  gray **output;

  char output_name[MAXPATHLEN] = "(stdout)";
  char input_name[MAXPATHLEN] = "(stdin)";
  char orig_name[MAXPATHLEN];
  char *subband_list = NULL;

  int in_cols, in_rows, in_format;
  gray in_maxval;
  int orig_cols, orig_rows, orig_format;
  gray orig_maxval;
  int cols, rows, format;
  gray maxval;
  int row;

  int no_orig = 0;
  int m = 0;
  int c;
  int maxlevel = 0;

  int filter = 1;
  int method = 2; 
  int levels;
  char filter_name[MAXPATHLEN] = "filter.dat";

  double error = 0.0;
  int print_psnr = 0;
  int print_psnr_subband = 0;
  int print_psnr_only = 0;

  double min, max;

  int verbose = 0;

  progname = argv[0];

  pgm_init(&argc, argv); wm_init();

  while ((c = getopt(argc, argv, "e:f:F:h?i:l:m:o:pPqs:v:")) != EOF) {
    switch (c) {
      case 'h':
      case '?':
        usage();
        break;
      case 'e':
        method = atoi(optarg);
        if (method < 0) {
          fprintf(stderr, "%s: wavelet filtering method %d out of range\n", progname, method);
          exit(1);
        }
        break;
      case 'f':
        filter = atoi(optarg);
        if (filter <= 0) {
          fprintf(stderr, "%s: filter number %d out of range\n", progname, filter);
          exit(1);
        }
        break;
      case 'F':
        strcpy(filter_name, optarg);
        break;
      case 'i':
        if (!strcmp(optarg, "-")) {
          no_orig = 1;
          strcpy(orig_name, "(zero)");
        }
        else {
          if ((orig = fopen(optarg, "rb")) == NULL) {
            fprintf(stderr, "%s: unable to open original image file %s\n", progname, optarg);
            exit(1);
          }
          strcpy(orig_name, optarg);
        }
        break;
      case 'l':
        maxlevel = atoi(optarg);
        if (maxlevel < 0) {
          fprintf(stderr, "%s: decomposition level %d out of range\n", progname, maxlevel);
          exit(1);
        }
        break;
      case 'm':
        m = atoi(optarg);
        if (m < -1) {
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
      case 'q':
        print_psnr = 1;
        print_psnr_subband = 1;
      case 's':
        subband_list = optarg;
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

  print = print_psnr_only ? out : stderr;

  if (argc == 1 && *argv[0] != '-') {
    if ((in = fopen(argv[0], "rb")) == NULL) {
      fprintf(stderr, "%s: unable to open input file %s\n", progname, argv[0]);
      exit(1);
    }
    else
      strcpy(input_name, argv[0]);
  }
  
  if (!orig && !no_orig) {
    fprintf(stderr, "%s: original image file not specified, using zero image\n", progname);
    strcpy(orig_name, "(zero)");
    no_orig = 1;
  }

  pgm_readpgminit(in, &in_cols, &in_rows, &in_maxval, &in_format);

  if (!no_orig) {
    pgm_readpgminit(orig, &orig_cols, &orig_rows, &orig_maxval, &orig_format);
    if (in_cols != orig_cols || in_rows != orig_rows) {
      fprintf(stderr, "%s: input image %s does not match dimensions of original image %s\n", progname, input_name, orig_name);
      exit(1);
    }
  }

  cols = in_cols;
  rows = in_rows;
  format = in_format;
  maxval = in_maxval;

  input_image = pgm_allocarray(cols, rows);
  orig_image = pgm_allocarray(cols, rows);

  output = alloc_grays(cols, rows);

  if (no_orig) {
    for (row = 0; row < rows; row++) {
      pgm_readpgmrow(in, input_image[row], cols, maxval, format);
      bzero(orig_image[row], sizeof(gray) * cols);
    }
  }
  else {
    for (row = 0; row < rows; row++) {
      pgm_readpgmrow(in, input_image[row], in_cols, in_maxval, in_format);
      pgm_readpgmrow(orig, orig_image[row], orig_cols, orig_maxval, orig_format);
    }
  }

  fclose(in);
  if (!no_orig)
    fclose(orig);

  // complete decomposition
  levels = find_deepest_level(cols, rows) - 1;
  if (!maxlevel) maxlevel = levels;
  if (maxlevel > levels) {
    fprintf(stderr, "%s: decomposition level %d not possible (max. %d), image size is %d x %d\n", progname, maxlevel, levels, cols, rows);
    exit(1);
  }

  init_dwt(cols, rows, filter_name, filter, maxlevel, method);

  input_dwts = fdwt(input_image);
  orig_dwts = fdwt(orig_image);

  p = input_dwts;
  q = orig_dwts;  
  min = 10000000.0;
  max = 0.0;
  error = 0.0;
  while (p->coarse && q->coarse) {
    double localmin, localmax, localerror;

    if (subband_in_list(subband_list, HORIZONTAL, p->horizontal->level)) {
      calc_subband(p->horizontal, q->horizontal, HORIZONTAL, &localmin, &localmax, &localerror);
      if (m == -1) process_subband_var(output, cols, rows, p->horizontal, q->horizontal, HORIZONTAL, localmin, localmax, maxval);
      if (localmin < min) min = localmin;
      if (localmax > max) max = localmax;
      error += localerror;
      if (print_psnr_subband)
        print_subband_psnr(HORIZONTAL, p->horizontal->level, error, p->horizontal->image->width, p->horizontal->image->height, localmin, localmax, print);
    }
    else if (verbose > 5)
      fprintf(stderr, "%s: subband %s%d skipped\n", progname, subband_name(HORIZONTAL), p->horizontal->level);

    if (subband_in_list(subband_list, VERTICAL, p->vertical->level)) {
      calc_subband(p->vertical, q->vertical, VERTICAL, &localmin, &localmax, &localerror);
      if (m == -1) process_subband_var(output, cols, rows, p->vertical, q->vertical, VERTICAL, localmin, localmax, maxval);
      if (localmin < min) min = localmin;
      if (localmax > max) max = localmax;
      error += localerror;
      if (print_psnr_subband)
        print_subband_psnr(VERTICAL, p->vertical->level, error, p->vertical->image->width, p->vertical->image->height, localmin, localmax, print);
    }
    else if (verbose > 5)
      fprintf(stderr, "%s: subband %s%d skipped\n", progname, subband_name(VERTICAL), p->vertical->level);

    if (subband_in_list(subband_list, DIAGONAL, p->diagonal->level)) {
      calc_subband(p->diagonal, q->diagonal, DIAGONAL, &localmin, &localmax, &localerror);
      if (m == -1) process_subband_var(output, cols, rows, p->diagonal, q->diagonal, DIAGONAL, localmin, localmax, maxval);
      if (localmin < min) min = localmin;
      if (localmax > max) max = localmax;
      error += localerror;
      if (print_psnr_subband)
        print_subband_psnr(DIAGONAL, p->vertical->level, error, p->diagonal->image->width, p->diagonal->image->height, localmin, localmax, print);
    }
    else if (verbose > 5)
      fprintf(stderr, "%s: subband %s%d skipped\n", progname, subband_name(DIAGONAL), p->diagonal->level);

    p = p->coarse;
    q = q->coarse;

    if (!p->coarse) {
      if (subband_in_list(subband_list, COARSE, p->level)) {
        calc_subband(p, q, COARSE, &localmin, &localmax, &localerror);
        if (m == -1) process_subband_var(output, cols, rows, p, q, COARSE, localmin, localmax, maxval);
        if (localmin < min) min = localmin;
        if (localmax > max) max = localmax;
        error += localerror;
        if (print_psnr_subband)
          print_subband_psnr(COARSE, p->level, error, p->image->width, p->image->height, localmin, localmax, print);
      }
      else if (verbose > 5)
        fprintf(stderr, "%s: subband %s%d skipped\n", progname, subband_name(COARSE), p->level);
    }
  }

  p = input_dwts;
  q = orig_dwts;
  while (p->coarse && q->coarse) {
    if (m > 0) {
      process_subband_fixed(output, cols, rows, p->horizontal, q->horizontal, HORIZONTAL, m);    
      process_subband_fixed(output, cols, rows, p->vertical, q->vertical, VERTICAL, m);    
      process_subband_fixed(output, cols, rows, p->diagonal, q->diagonal, DIAGONAL, m);    
    }
    else if (m == 0) {
      process_subband_var(output, cols, rows, p->horizontal, q->horizontal, HORIZONTAL, min, max, maxval);    
      process_subband_var(output, cols, rows, p->vertical, q->vertical, VERTICAL, min, max, maxval);    
      process_subband_var(output, cols, rows, p->diagonal, q->diagonal, DIAGONAL, min, max, maxval);    
    }

    p = p->coarse;
    q = q->coarse;

    if (!p->coarse) {
      if (m > 0) 
        process_subband_fixed(output, cols, rows, p, q, COARSE, m);
      else if (m == 0) 
        process_subband_var(output, cols, rows, p, q, COARSE, min, max, maxval);
    }
  }

  if (!print_psnr_only) {
    pgm_writepgminit(out, cols, rows, maxval, 0);
    for (row = 0; row < rows; row++)
      pgm_writepgmrow(out, output[row], cols, maxval, 0);

    fclose(out);
  }

 pgm_freearray(input_image, rows);
 pgm_freearray(orig_image, rows);
 free_grays(output);

 if (print_psnr || print_psnr_only) {
    double mse = error / (double) (cols * rows);
    double rmse = sqrt(mse);
    double psnr = 20.0 * log(255.0 / rmse) / log(10.0);
    if (mse > 0.0)
      fprintf(print, "PSNR: %lf dB\n", psnr);
    else
      fprintf(print, "PSNR: inf\n");
    fprintf(print, "RMS: %lf\n", rmse);
    fprintf(print, "MSE: %lf\n", mse);
    fprintf(print, "dmin, dmax: %lf, %lf\n", min, max);
  }

  exit(0);
}
