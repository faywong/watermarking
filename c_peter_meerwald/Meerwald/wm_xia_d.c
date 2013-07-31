#include "wm.h"
#include "dwt.h"
#include "pgm.h"
#include "dwt_util.h"

char *progname;

void usage(void) {
  fprintf(stderr, "usage: %s [-a n] [-e n] [-f n] [-F file] [-h] [-l n] [-o file] [-v n] -s file -i file file\n\n", progname);
  fprintf(stderr, "\t-a n\t\talpha factor\n");
  fprintf(stderr, "\t-e n\t\twavelet filtering method\n");
  fprintf(stderr, "\t-f n\t\tfilter number\n");
  fprintf(stderr, "\t-F file\t\tfilter definition file\n");
  fprintf(stderr, "\t-h\t\tprint usage\n");
  fprintf(stderr, "\t-i file\t\toriginal image file\n");
  fprintf(stderr, "\t-l n\t\tdecomposition level\n");
  fprintf(stderr, "\t-o file\t\tfile for extracted watermark\n");
  fprintf(stderr, "\t-s file\t\toriginal signature file\n");
  fprintf(stderr, "\t-v n\t\tverbosity level\n");
  exit(0);
}

int extract_subband(Image_tree s, Image_tree t, int name, double alpha, double watermark[], int n, int verbose) {
  int i, j;
  int w;

  w = 0;
  for (i = 5; i < s->image->height-5; i++)
    for (j = 5; j < s->image->width-5; j++) {
      double input_coeff, orig_coeff;

      input_coeff = get_pixel(s->image, i, j);
      orig_coeff = get_pixel(t->image, i, j);
      if (fabs(orig_coeff) > 20.0)
        watermark[w++] = (input_coeff - orig_coeff) / (alpha * pow(fabs(orig_coeff), 1.0 / 1.2));
    }

  if (verbose > 3)
    fprintf(stderr, "%s: extracting %s%d, size %d x %d; extracted %d coeffs.\n",
      progname, subband_name(name), s->level, s->image->width, s->image->height, w);
      
  return w;
}

void write_mark(FILE *out, double watermark[], int n) {
  int i;

  fprintf(out, "%d\n", n);
  for (i = 0; i < n; i++) 
    fprintf(out, "%f\n", watermark[i]);
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
  int n = 0;
  int method = -1;
  int filter = 0;
  char filter_name[MAXPATHLEN] = "";

  int level = 0, levels;
  double alpha = 0.0;

  int in_rows, in_cols, in_format;
  gray in_maxval;
  int orig_rows, orig_cols, orig_format;
  gray orig_maxval;
  int rows, cols;
  int row;

  double *watermark;

  Image_tree input_dwts, orig_dwts, p, q;

  int verbose = 0;

  progname = argv[0];

  pgm_init(&argc, argv); wm_init2();

  while ((c = getopt(argc, argv, "a:e:f:F:h?i:l:o:s:v:")) != EOF) {
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
      case 'i':
        if ((orig = fopen(optarg, "rb")) == NULL) {
          fprintf(stderr, "%s: unable to open original image file %s\n", progname, optarg);
          exit(1);
        }
        strcpy(orig_name, optarg);
        break;
      case 'l':
        level = atoi(optarg);  
        if (level <= 0) { 
          fprintf(stderr, "%s: decomposition level %d out of range\n", progname, level);
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
    if (strspn(line, "XASG") >= 4) {
      fscanf(sig, "%d\n", &n);
      if (alpha == 0.0)
        fscanf(sig, "%lf\n", &alpha);
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
    fclose(sig);
  }
  else {
    fprintf(stderr, "%s: signature file not specified, use -s file option\n", progname);
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

  input_image = pgm_allocarray(in_cols, in_rows);
  orig_image = pgm_allocarray(orig_cols, orig_rows);

  for (row = 0; row < in_rows; row++) {
    pgm_readpgmrow(in, input_image[row], in_cols, in_maxval, in_format);
    pgm_readpgmrow(orig, orig_image[row], orig_cols, orig_maxval, orig_format);
  }

  fclose(in);
  fclose(orig);

  // complete decomposition
  levels = find_deepest_level(cols, rows) - 1;
  if (level > levels) {
    fprintf(stderr, "%s: decomposition level %d not possible (max. %d), image size is %d x %d\n", progname, level, levels, cols, rows);
    exit(1);
  }

  init_dwt(cols, rows, filter_name, filter, level, method);
#ifdef POLLEN_STUFF
#include "pollen_stuff.c"
#endif
#ifdef PARAM_STUFF
#include "param_stuff.c"
#endif

  input_dwts = fdwt(input_image);
  orig_dwts = fdwt(orig_image);

  watermark = malloc((rows * cols / 4.0) * sizeof(double));
  if (!watermark) {
    fprintf(stderr, "%s: malloc() failed\n\n", progname);
    exit(1);
  }

  fprintf(out, "XAWM\n");
  fprintf(out, "%d\n", n);
  fprintf(out, "%d\n", level * 3);

  p = input_dwts;
  q = orig_dwts;
  while (p->coarse && q->coarse) { 
    int size;
    
    size = extract_subband(p->horizontal, q->horizontal, HORIZONTAL, alpha, watermark, n, verbose);
    write_mark(out, watermark, size);
    size = extract_subband(p->vertical, q->vertical, VERTICAL, alpha, watermark, n, verbose);
    write_mark(out, watermark, size);
    size = extract_subband(p->diagonal, q->diagonal, DIAGONAL, alpha, watermark, n, verbose);
    write_mark(out, watermark, size);
        
    p = p->coarse;
    q = q->coarse;
  }

  fclose(out);

  free(watermark);

  pgm_freearray(input_image, rows);
  pgm_freearray(orig_image, rows);

  exit(0);
}
