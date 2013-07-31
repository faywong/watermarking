#include "wm.h"
#include "dwt.h"
#include "pgm.h"

char *progname;

void usage(void) {
  fprintf(stderr, "usage: %s [-a n] [-e n] [-f n] [-F file] [-h] [-l n] [-o file] [-t n] [-v n] -s file file\n\n", progname);
  fprintf(stderr, "\t-a n\t\talpha factor/embedding strength\n");
  fprintf(stderr, "\t-e n\t\twavelet filtering method\n");
  fprintf(stderr, "\t-f n\t\tfilter number\n");
  fprintf(stderr, "\t-F file\t\tfilter definition file\n");
  fprintf(stderr, "\t-h\t\tprint usage\n");
  fprintf(stderr, "\t-l n\t\tdecomposition levels\n");
  fprintf(stderr, "\t-o file\t\toutput (watermarked) file\n");
  fprintf(stderr, "\t-s file\t\tsignature to embed in input image\n");
  fprintf(stderr, "\t-t n\t\tcasting threshold\n");
  fprintf(stderr, "\t-v n\t\tverbosity level\n");
  exit(0);
}

// actual watermarking procedure: embeds a watermark of n normally
// distributed values into a coefficients > threshold t1 of a subband
void wm_subband(Image s, double *w, int n, double a, double t1) {
  int i;

  for (i = 0; i < s->width * s->height; i++) 
    if (fabs(s->data[i]) > t1)
      s->data[i] += (a * fabs(s->data[i]) * w[i % n]);
}

int main(int argc, char *argv[]) {

  FILE *in = stdin;
  FILE *out = stdout;
  FILE *sig = NULL;

  char output_name[MAXPATHLEN] = "(stdout)";
  char input_name[MAXPATHLEN] = "(stdin)";
  char signature_name[MAXPATHLEN];

  int i, c;
  int row;

  int n;

  double alpha = 0.0;
  double t1 = 0.0;

  int level = 0;
  int filter = 0;
  int method = -1;
  char filter_name[MAXPATHLEN] = "";

  int verbose = 0;

  gray **image;
  Image_tree dwts, s;

  gray maxval;
  int rows, cols, format;

  double *watermark;

  progname = argv[0];

  pgm_init(&argc, argv); wm_init();

  while ((c = getopt(argc, argv, "a:e:f:F:h?l:o:s:t:v:")) != EOF) {
    switch (c) {
      case 'a':
        alpha = atof(optarg);
        if (alpha <= 0.0) {
          fprintf(stderr, "%s: alpha factor %f out of range\n", progname, alpha);
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
      case 'l': 
        level = atoi(optarg);
        if (level <= 0) {
          fprintf(stderr, "%s: decomposition level %d out of range\n", progname, level);
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
      case 's':
        if ((sig = fopen(optarg, "r")) == NULL) {
          fprintf(stderr, "%s: unable to open signature file %s\n", progname, optarg);
          exit(1);
        }
        strcpy(signature_name, optarg);
        break;
      case 't':
        t1 = atof(optarg);
        if (t1 <= 0.0) {
          fprintf(stderr, "%s: casting threshold %f out of range\n", progname, t1);
          exit(1);
        }
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
    if (strspn(line, "DGSG") >= 4) {
      fscanf(sig, "%d\n", &n);
      if (level == 0)
        fscanf(sig, "%d\n", &level);
      else
        fscanf(sig, "%*d\n");
      if (alpha == 0.0)
        fscanf(sig, "%lf\n", &alpha);
      else
        fscanf(sig, "%*f\n");
      if (t1 == 0.0)
        fscanf(sig, "%lf\n", &t1);
      else
        fscanf(sig, "%*f\n");
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

  // wavelet transform
  init_dwt(cols, rows, filter_name, filter, level, method);
#ifdef POLLEN_STUFF
#include "pollen_stuff.c"
#endif
#ifdef PARAM_STUFF
#include "param_stuff.c"
#endif

  dwts = fdwt(image);

  // embed watermark in all subbands of a decomposition level
  for (i = 0, s = dwts; i < 3; i++, s = s->coarse) {
    wm_subband(s->horizontal->image, watermark, n, alpha, t1);
    wm_subband(s->vertical->image, watermark, n, alpha, t1);
    wm_subband(s->diagonal->image, watermark, n, alpha, t1);
  }

  free(watermark);

  idwt(dwts, image);

  pgm_writepgminit(out, cols, rows, maxval, 0);

  for (row = 0; row < rows; row++)
    pgm_writepgmrow(out, image[row], cols, maxval, 0);

  fclose(out);

  pgm_freearray(image, rows);

  exit(0);
}
