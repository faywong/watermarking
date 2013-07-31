#include "wm.h"
#include "wm_dwt.h"
#include "pgm.h"
#include "dwt_util.h"
#include "kim_common.h"

char *progname;

void usage(void) {
  fprintf(stderr, "usage: %s [-a n] [-A n] [-e n] [-f n] [-F n] [-h] [-l n] [-o file] [-v n] -s file file\n\n", progname);
  fprintf(stderr, "\t-a n\t\talpha factor/embedding strength for detail subbands\n");
  fprintf(stderr, "\t-a n\t\talpha factor/embedding strength for approximation subband\n");
  fprintf(stderr, "\t-e n\t\twavelet filtering method\n");
  fprintf(stderr, "\t-f n\t\tfilter number\n");
  fprintf(stderr, "\t-F file\t\tfilter definition file\n");
  fprintf(stderr, "\t-h\t\tprint usage\n");
  fprintf(stderr, "\t-l n\t\tdecomposition level\n");
  fprintf(stderr, "\t-o file\t\toutput (watermarked) file\n");
  fprintf(stderr, "\t-s file\t\tsignature to embed in input image\n");
  fprintf(stderr, "\t-v n\t\tverbosity level\n");
  exit(0);
}

int mark_subband(Image_tree s, int name, double alpha, double watermark[], double threshold, int w, int n, int verbose) {
  int i, j;
  double last = 0.0;

  for (i = 5; i < s->image->height-5; i++)
    for (j = 5; j < s->image->width-5; j++) {
      double coeff, newcoeff;

      coeff = get_pixel(s->image, i, j);   
      if (fabs(coeff) > threshold / 1.5 ) {
        newcoeff = coeff - coeff * alpha * watermark[w++ % n];
        set_pixel(s->image, i, j, newcoeff);

        fprintf(stderr, "%s: (%d/%d) %f: %f -> %f; a=%f\n", progname, j, i, watermark[w % n], coeff, newcoeff, alpha);
        w++;
      }
    }

  if (verbose > 5)
    fprintf(stderr, "%s: watermarking %s%d, size %d x %d; embedded %d coeffs. total\n",
      progname, subband_name(name), s->level, s->image->width, s->image->height, w);

  return w;
}

int main(int argc, char *argv[]) {

  FILE *in = stdin;
  FILE *out = stdout;
  FILE *sig = NULL;

  char output_name[MAXPATHLEN] = "(stdout)";
  char input_name[MAXPATHLEN] = "(stdin)";
  char signature_name[MAXPATHLEN];

  int i, c, w;
  int row, col;

  int n;

  double alpha_detail = 0.0;
  double alpha_approx = 0.0;
  int level = 0;

  int filter = 0;
  int method = -1;
  int levels;
  char filter_name[MAXPATHLEN] = "";

  int verbose = 0;

  gray **image;
  Image_tree p, dwts;

  gray maxval;
  int rows, cols, colors, format;

  double *watermark;

  progname = argv[0];

  pgm_init(&argc, argv);

#ifdef __EMX__
  _fsetmode(in, "b");
  _fsetmode(out, "b");
#endif

  while ((c = getopt(argc, argv, "a:A:e:f:F:h?o:l:s:v:")) != EOF) {
    switch (c) {
      case 'a':
        alpha_detail = atof(optarg);
        if (alpha_detail <= 0.0) {
          fprintf(stderr, "%s: alpha factor %f out of range\n", progname, alpha_detail);
          exit(1);
        }
        break;
      case 'A':
        alpha_approx = atof(optarg);
        if (alpha_approx <= 0.0) {
          fprintf(stderr, "%s: alpha factor %f out of range\n", progname, alpha_approx);
          exit(1);
        }
        break;
      case 'l':
        level = atoi(optarg);
        if (level <= 0) {
          fprintf(stderr, "%s: decomposition level %d out of range\n", progname, level);
          exit(1);
        }
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

  if (argc == 1 && *argv[0] != '-')
    if ((in = fopen(argv[0], "rb")) == NULL) {
      fprintf(stderr, "%s: unable to open input file %s\n", progname, argv[0]);
      exit(1);
    }
    else
      strcpy(input_name, argv[0]);

  if (sig) {
    char line[32];
    fgets(line, sizeof(line), sig);
    if (strspn(line, "KISG") >= 4) {
      fscanf(sig, "%d\n", &n);
      if (alpha_detail == 0.0)
        fscanf(sig, "%lf\n", &alpha_detail);
      else
        fscanf(sig, "%*f\n");
      if (alpha_approx == 0.0)
        fscanf(sig, "%lf\n", &alpha_approx);
      else
        fscanf(sig, "%*f\n");
      if (level == 0)
        fscanf(sig, "%d\n", &level);
      else
        fscanf(sig, "%*d\n");
      if (method < 0)
        fscanf(sig, "%d\n", &method);
      else
        fscanf(sig, "%*d\n");
      if (filter == 0)
        fscanf(sig, "%d\n", &filter);
      else
        fscanf(sig, "%*d\n");
      if (!strcmp(filter_name, ""))
        fscanf(sig, "%[^\n\r]\n", filter_name);
      else
        fscanf(sig, "%*[^\n\r]\n");
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

  watermark = malloc(n * sizeof(double));
  for (i = 0; i < n; i++) 
    fscanf(sig, "%lf\n", &watermark[i]);
  fclose(sig);

  pgm_readpgminit(in, &cols, &rows, &maxval, &format);
  image = pgm_allocarray(cols, rows);
  for (row = 0; row < rows; row++)
    pgm_readpgmrow(in, image[row], cols, maxval, format);
  fclose(in);

  // complete decomposition
  levels = find_deepest_level(cols, rows) - 1;
  if (level > levels) {
    fprintf(stderr, "%s: decomposition level %d not possible (max. %d), image size is %d x %d\n", progname, level, levels, cols, rows);
    exit(1);
  }

  // wavelet transform
  init_dwt(cols, rows, filter_name, filter, level, method);
#ifdef POLLEN_STUFF
#include "pollen_stuff.c"
#endif
#ifdef PARAM_STUFF
#include "param_stuff.c"
#endif

  dwts = fdwt(image);

  p = dwts;
  w = 0;

  // process each decomposition level
  while (p->coarse) {
    int current_level;
    double threshold;
    double max_coeff;
    double alpha;

    // get current decomposition level number
    current_level = p->horizontal->level;

    // find largest absolute coefficient in detail subbands of current decomposition level
    max_coeff = find_level_largest_coeff(p, verbose);

    // calculate significance threshold for current decomposition level
    threshold = calc_level_threshold(max_coeff, verbose);

    // calculate embedding strength alpha for current decomposition level
    alpha = calc_level_alpha_detail(alpha_detail, level, current_level, verbose);

    if (verbose > 1)
      fprintf(stderr, "%s: level %d, threshold %f, alpha %f\n", progname, current_level, threshold, alpha);

    // embed watermark sequence into detail subbands of current decomposition level
    w = mark_subband(p->horizontal, HORIZONTAL, alpha, watermark, threshold, w, n, verbose);
    w = mark_subband(p->vertical, VERTICAL, alpha, watermark, threshold, w, n, verbose);
    w = mark_subband(p->diagonal, DIAGONAL, alpha, watermark, threshold, w, n, verbose);

    p = p->coarse;
  }

  // mark approximation image using calculated significance threshold and embedding strength
  w = mark_subband(p, COARSE, alpha_approx, watermark, calc_level_threshold(find_subband_largest_coeff(p, COARSE, verbose), verbose), w, n, verbose);

  free(watermark);
  idwt(dwts, image);

  pgm_writepgminit(out, cols, rows, maxval, 0);
  for (row = 0; row < rows; row++)
    pgm_writepgmrow(out, image[row], cols, maxval, 0);
  fclose(out);

  pgm_freearray(image, rows);

  exit(0);
}
