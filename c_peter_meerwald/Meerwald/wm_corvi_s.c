#include "wm.h"
#include "wm_dwt.h"
#include "pgm.h"

char *progname;

void usage(void) {
  fprintf(stderr, "usage: %s [-a n] [-h] [-n n] [-o file] [-q n] [-s file] [-v n] -i file file\n\n", progname);
  fprintf(stderr, "\t-a n\t\talpha factor\n");
  fprintf(stderr, "\t-e n\t\twavelet filtering method\n");
  fprintf(stderr, "\t-f n\t\tfilter number\n");
  fprintf(stderr, "\t-F file\t\tfilter definition file\n");
  fprintf(stderr, "\t-h\t\tprint usage\n");
  fprintf(stderr, "\t-i file\t\toriginal image file\n");
  fprintf(stderr, "\t-n n\t\twatermark length\n");
  fprintf(stderr, "\t-o file\t\textracted signature file\n");
  fprintf(stderr, "\t-q n\t\tquantization/quality factor\n");
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
  int i;
  int quantization = 0;
  int n = 0;
  int method = -1;
  int filter = 0;
  char filter_name[MAXPATHLEN] = "";

  int level;
  double alpha = 0.0;

  int in_rows, in_cols, in_format;
  gray in_maxval;
  int orig_rows, orig_cols, orig_format;
  gray orig_maxval;
  int rows, cols;
  int row, col;

  Image_tree input_dwts;
  Image_tree orig_dwts;

  int verbose = 0;

  progname = argv[0];

  pgm_init(&argc, argv); wm_init2();

  while ((c = getopt(argc, argv, "a:e:f:F:h?i:n:o:s:v:")) != EOF) {
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
      case 'n':
        n = atoi(optarg);
        if (n < 1 || n > 1000) {
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

  if (!orig) {
    fprintf(stderr, "%s: original image file not specified, use -i file option\n", progname);
    exit(1);
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
      if (quantization == 0)
        fscanf(sig, "%d\n", &quantization);
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

  for (row = 0; row < in_rows; row++)
    pgm_readpgmrow(in, input_image[row], in_cols, in_maxval, in_format);

  fclose(in);

  for (row = 0; row < orig_rows; row++)
    pgm_readpgmrow(orig, orig_image[row], orig_cols, orig_maxval, orig_format);

  fclose(orig);

  level = 0;
  row = rows;
  col = cols;
  while (n < row * col / 4.0 && row >= 2 && col >= 2) {
    row /= 2;
    col /= 2;
    level++;
  }

  if (verbose >= 2) {
    fprintf(stderr, "%s: extracting from coarse image (x %d/y %d) at level %d\n", progname, col, row, level);
  }

  init_dwt(cols, rows, filter_name, filter, level, method);
  input_dwts = fdwt(input_image);
  orig_dwts = fdwt(orig_image);

  fprintf(out, "CVSG\n");
  fprintf(out, "%d\n", n);
  fprintf(out, "%f\n", alpha);
  fprintf(out, "%d\n", quantization);
  fprintf(out, "%d\n", method);
  fprintf(out, "%d\n", filter);
  fprintf(out, "%s\n", filter_name);

  {
    Image_tree p = input_dwts;
    Image_tree q = orig_dwts;
    Image input_img;
    Image orig_img;
    double input_med;
    double orig_med;

    while (!p->image)
      p = p->coarse;

    while (!q->image)
      q = q->coarse;

    input_img = p->image;
    orig_img = q->image;

    input_med = 0.0;
    for (row = 0; row < input_img->height; row++)
      for (col = 0; col < input_img->width; col++)
        input_med += get_pixel(input_img, col, row);
    input_med /= input_img->height * input_img->width;

    orig_med = 0.0;
    for (row = 0; row < orig_img->height; row++)
      for (col = 0; col < orig_img->width; col++)
        orig_med += get_pixel(orig_img, col, row);
    orig_med /= orig_img->height * orig_img->width;

    row = 0;
    col = 0;
    while (n > 0) {
      Pixel input_pix;
      Pixel orig_pix;
      double x;

      input_pix = get_pixel(input_img, col, row);
      orig_pix = get_pixel(orig_img, col, row);

      x = ((input_pix - orig_pix) / (orig_pix - orig_med)) / alpha;

      fprintf(out, "%f\n", x);

      if (++col == orig_img->width) { col = 0; row++; }
      n--;
    }
  }

  fclose(out);

  pgm_freearray(input_image, rows);
  pgm_freearray(orig_image, rows);

  exit(0);
}
