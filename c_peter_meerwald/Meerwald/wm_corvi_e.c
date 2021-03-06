#include "wm.h"
#include "dwt.h"
#include "pgm.h"

char *progname;

void usage(void) {
  fprintf(stderr, "usage: %s [-a n] [-e n] [-f n] [-F n] [-h] [-o file] [-v n] -s file file\n\n", progname);
  fprintf(stderr, "\t-a n\t\talpha factor/embedding strength\n");
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

  int c;
  int row, col;

  int n;

  double alpha = 0.0;

  int filter = 0;
  int method = -1;
  int level = 0;
  char filter_name[MAXPATHLEN] = "";

  int verbose = 0;

  gray **image;
  Image_tree dwts;

  gray maxval;
  int rows, cols, format;

  progname = argv[0];

  pgm_init(&argc, argv); wm_init();

  while ((c = getopt(argc, argv, "a:e:f:F:h?o:s:v:")) != EOF) {
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
    if (strspn(line, "CVSG") >= 4) {
      fscanf(sig, "%d\n", &n);
      if (alpha == 0.0)
        fscanf(sig, "%lf\n", &alpha);
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

  pgm_readpgminit(in, &cols, &rows, &maxval, &format);

  image = pgm_allocarray(cols, rows);

  for (row = 0; row < rows; row++)
    pgm_readpgmrow(in, image[row], cols, maxval, format);

  fclose(in);

  level = 0;
  row = rows;
  col = cols;
  while (n < row * col / 4.0 && row >= 2 && col >= 2) {
    row /= 2;
    col /= 2;
    level++;
  }

  if (verbose >= 2) {
    fprintf(stderr, "%s: embedding into coarse image (x %d/y %d) at level %d\n", progname, col, row, level);
  }

  init_dwt(cols, rows, filter_name, filter, level, method);
#ifdef POLLEN_STUFF
#include "pollen_stuff.c"
#endif
#ifdef PARAM_STUFF
#include "param_stuff.c"
#endif

  dwts = fdwt(image);

  {
    Image_tree p = dwts;
    Image img;
    double med;

    while (!p->image)
      p = p->coarse;

    img = p->image;

    med = 0.0;
    for (row = 0; row < img->height; row++)
      for (col = 0; col < img->width; col++)
        med += get_pixel(img, col, row);

    med /= (double) (img->height * img->width);

    row = 0;
    col = 0;
    while (n > 0) {
      double pix;
      double g;

      fscanf(sig, "%lf\n", &g);

      pix = get_pixel(img, col, row);
      pix = med + (pix - med) * (1.0 + alpha * g);
      set_pixel(img, col, row, pix);

      if (++col == img->width) { col = 0; row++; }
      n--;
    }
  }

  fclose(sig);

  idwt(dwts, image);

  pgm_writepgminit(out, cols, rows, maxval, 0);

  for (row = 0; row < rows; row++)
    pgm_writepgmrow(out, image[row], cols, maxval, 0);

  fclose(out);

  pgm_freearray(image, rows);

  exit(0);
}
