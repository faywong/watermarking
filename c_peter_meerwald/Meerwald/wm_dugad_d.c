#include "wm.h"
#include "dwt.h"
#include "dwt_util.h"
#include "pgm.h"

char *progname;

void usage(void) {
  fprintf(stderr, "usage: %s [-a n] [-e n] [-f n] [-F file] [-h] [-l n] [-n n] [-o file] [-v n] [-t n] -s file file\n\n", progname);
  fprintf(stderr, "\t-a n\t\talpha factor\n");
  fprintf(stderr, "\t-e n\t\twavelet filtering method\n");
  fprintf(stderr, "\t-f n\t\tfilter number\n");
  fprintf(stderr, "\t-F file\t\tfilter definition file\n");
  fprintf(stderr, "\t-h\t\tprint usage\n");
  fprintf(stderr, "\t-l n\t\tdecomposition levels\n");
  fprintf(stderr, "\t-n n\t\twatermark length\n");
  fprintf(stderr, "\t-o file\t\tfile for extracted watermark\n");
  fprintf(stderr, "\t-s file\t\toriginal signature file\n");
  fprintf(stderr, "\t-t n\t\tdetection threshold\n");
  fprintf(stderr, "\t-v n\t\tverbosity level\n");
  exit(0);
}

void wm_subband(Image s, double *w, int n, double t2, int *m, double *z, double *v) {
  int i;
  
  *m = 0;
  *z = 0.0;
  *v = 0.0;
  for (i = 0; i < s->width * s->height; i++)
    if (s->data[i] > t2) {
      (*z) += (s->data[i] * w[i % n]);
      (*v) += fabs(s->data[i]);
      (*m)++;
    }
}

int main(int argc, char *argv[]) {

  FILE *in = stdin;
  FILE *out = stdout;
  FILE *sig = NULL;

  gray **input_image;

  char signature_name[MAXPATHLEN];
  char output_name[MAXPATHLEN] = "(stdout)";
  char input_name[MAXPATHLEN] = "(stdin)";

  int c;
  int i;
  int n = 0;
  int method = -1;
  int level = 0;
  int filter = 0;
  char filter_name[MAXPATHLEN] = "";

  double alpha = 0.0;
  double t2 = 0.0;

  int in_rows, in_cols, in_format;
  gray in_maxval;
  int rows, cols;
  int row;

  double *watermark;

  Image_tree dwts, s;

  int verbose = 0;

  progname = argv[0];

  pgm_init(&argc, argv); wm_init2();

  while ((c = getopt(argc, argv, "a:e:f:F:h?l:n:o:s:v:t")) != EOF) {
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

      case 'n':
        n = atoi(optarg);
        if (n < 1 || n > 32000) {
          fprintf(stderr, "%s: watermark length %d out of range\n", progname, n);
          exit(1);
        }
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
      case 't':
        t2 = atof(optarg);
        if (t2 <= 0.0) {  
          fprintf(stderr, "%s: detection threshold %f out of range\n", progname, t2); 
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
      fscanf(sig, "%*f\n");
      if (t2 == 0.0)
        fscanf(sig, "%lf\n", &t2);
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

  pgm_readpgminit(in, &in_cols, &in_rows, &in_maxval, &in_format);

  cols = in_cols;
  rows = in_rows;

  input_image = pgm_allocarray(in_cols, in_rows);

  for (row = 0; row < in_rows; row++) {
    pgm_readpgmrow(in, input_image[row], in_cols, in_maxval, in_format);
  }

  fclose(in);

  init_dwt(cols, rows, filter_name, filter, level, method);
#ifdef POLLEN_STUFF
#include "pollen_stuff.c"
#endif
#ifdef PARAM_STUFF
#include "param_stuff.c"
#endif

  dwts = fdwt(input_image);

  fprintf(out, "DGWM\n");
  fprintf(out, "%d\n", level);
  fprintf(out, "%f\n", alpha);

  for (i = 0, s = dwts; i < level; i++, s = s->coarse) {
    int m;
    double z, v;

    wm_subband(s->horizontal->image, watermark, n, t2, &m, &z, &v);
    fprintf(out, "%d %f %f\n", m, z, v);
    wm_subband(s->vertical->image, watermark, n, t2, &m, &z, &v);
    fprintf(out, "%d %f %f\n", m, z, v);
    wm_subband(s->diagonal->image, watermark, n, t2, &m, &z, &v);
    fprintf(out, "%d %f %f\n", m, z, v);
  }

  fclose(out);

  free(watermark);

  pgm_freearray(input_image, rows);

  exit(0);
}
