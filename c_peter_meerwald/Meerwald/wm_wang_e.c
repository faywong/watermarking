#include "wm.h"
#include "dwt.h"
#include "wang_common.h"
#include "dwt_util.h"
#include "pgm.h"

char *progname;

void usage(void) {
  fprintf(stderr, "usage: %s [-a n] [-b n] [-e n] [-f n] [-F n] [-h] [-o file] [-v n] -s file file\n\n", progname);
  fprintf(stderr, "\t-a n\t\talpha factor/embedding strength\n");
  fprintf(stderr, "\t-b n\t\tsubband weighting factor\n");
  fprintf(stderr, "\t-e n\t\twavelet filtering method\n");
  fprintf(stderr, "\t-f n\t\tfilter number\n");
  fprintf(stderr, "\t-F file\t\tfilter definition file\n");
  fprintf(stderr, "\t-h\t\tprint usage\n");
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

  int i, c, w;
  int row;

  int n;

  double alpha = 0.0;
  double beta = 0.0;

  int filter = 0;
  int method = -1;
  int level = 0;
  char filter_name[MAXPATHLEN] = "";

  int verbose = 0;

  gray **image;
  Image_tree dwts;

  gray maxval;
  int rows, cols, format;

  double *watermark;

  progname = argv[0];

  pgm_init(&argc, argv); wm_init();

  while ((c = getopt(argc, argv, "a:b:e:f:F:h?o:s:v:")) != EOF) {
    switch (c) {
      case 'a':
        alpha = atof(optarg);
        if (alpha <= 0.0) {
          fprintf(stderr, "%s: alpha factor %f out of range\n", progname, alpha);
          exit(1);
        }
        break;
      case 'b':
        beta = atof(optarg);
        if (beta <= 0.0) {
          fprintf(stderr, "%s: beta factor %f out of range\n", progname, beta);
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
    if (strspn(line, "WGSG") >= 4) {
      fscanf(sig, "%d\n", &n);
      if (alpha == 0.0)
        fscanf(sig, "%lf\n", &alpha);
      else
        fscanf(sig, "%*f\n");
      if (beta == 0.0)
        fscanf(sig, "%lf\n", &beta);
      else
        fscanf(sig, "%*f\n");
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
  level = find_deepest_level(cols, rows) - 1;

  // wavelet transform
  init_dwt(cols, rows, filter_name, filter, level, method); 
#ifdef POLLEN_STUFF
#include "pollen_stuff.c"
#endif
#ifdef PARAM_STUFF
#include "param_stuff.c"
#endif

  dwts = fdwt(image);

  // build tree for subband selection, calculate subband thresholds
  init_subbands(dwts);
  set_subbands_type_beta(HORIZONTAL, beta);
  set_subbands_type_beta(VERTICAL, beta);
  calc_subbands_threshold();

  w = 0;
  while (w < n) {
    Subband_data s;

    // select subband with max. threshold
    s = select_subband();
    if (verbose > 1)
      fprintf(stderr, "%s: selected subband %s%d, T=%lf, beta=%lf\n", progname, subband_name(s->type), s->level, s->T, s->beta);
 
    // watermark significant coefficients and set them selected
    // check is entire signature has been embedded
    c = select_subband_coeff(s);
    do {
      double p;
      if (c < 0) 
        // no more significant coefficients in subband
        break;

      p = get_subband_coeff(s, c);
      if (p < s->Cmax) {
        if (verbose > 2)
          fprintf(stderr, "%s: embedding sig. coeff. #%d (= %lf)\n  into %s%d coeff. #%d\n", 
            progname, w, watermark[w], subband_name(s->type), s->level, c);

        p = p + alpha * s->beta * s->T * watermark[w];
        set_subband_coeff(s, c, p);
        w++;
      }
      mark_subband_coeff(s, c);

      // select next significant coefficient
      c = select_subband_coeff_from(s, c);
    } while (w < n);

    // update subband threshold
    s->T /= 2.0;

  }

  free_subbands();

  free(watermark);

  idwt(dwts, image);

  pgm_writepgminit(out, cols, rows, maxval, 0);

  for (row = 0; row < rows; row++)
    pgm_writepgmrow(out, image[row], cols, maxval, 0);

  fclose(out);

  pgm_freearray(image, rows);

  exit(0);
}
