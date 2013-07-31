#include "wm.h"
#include "signature.h"
#include "dwt.h"
#include "pgm.h"

char *progname;

// inverse watermarking transformation, extract embedded bit, check quantization boundaries
double wm_transform(double alpha, double f1, double f2, double f3) {
  double s = alpha * (fabs(f3) - fabs(f1)) / 2.0;
  double l = f1;
  int x;
  
  x = 0;
  while (l  < f2) {
    l += s;
    x++;
  }

  if (fabs(l - s - f2) < fabs(l-f2))
    return (x+1) % 2;
  else 
    return (x) % 2;
}

void usage(void) {
  fprintf(stderr, "usage: %s [-a n] [-e n] [-f n] [-F n] [-h] [-l n] [-o file] [-v n] -s file file\n\n", progname);
  fprintf(stderr, "\t-a n\t\tembedding strength (default 0.05)\n");
  fprintf(stderr, "\t-e n\t\twavelet filtering method\n");
  fprintf(stderr, "\t-f n\t\tfilter number\n");
  fprintf(stderr, "\t-F file\t\tfilter definition file\n");
  fprintf(stderr, "\t-l n\t\tembedding level\n");
  fprintf(stderr, "\t-h\t\tprint usage\n");
  fprintf(stderr, "\t-o file\t\textracted signature file\n");
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

  int c, n;
  int row, col;

  double alpha = 0.0;
  int filter = 0;
  int method = -1;
  int level = 0;
  char filter_name[MAXPATHLEN] = "";

  int seed;
  int verbose = 0;

  gray **image;
  Image_tree dwts, p;

  gray maxval;
  int rows, cols, format;

  progname = argv[0];

  pgm_init(&argc, argv); wm_init2();

  while ((c = getopt(argc, argv, "a:e:f:F:h?l:o:s:v:")) != EOF) {
    switch (c) {
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
        if (level < 1) {
          fprintf(stderr, "%s: embedding level out of range\n", progname);
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
      case 'a':
        alpha = atof(optarg);
        if (alpha <= 0.0) {
          fprintf(stderr, "%s: embedding strength factor %f out of range\n", progname, alpha);
          exit(1);
        }
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
    if (strspn(line, "XESG") >= 4) {
      fscanf(sig, "%d\n", &nbit_signature);
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
      if (level == 0)
        fscanf(sig, "%d\n", &level);
      else
        fscanf(sig, "%*d\n");
      fscanf(sig, "%d\n", &seed);
      srandom(seed);
      n_signature = NBITSTOBYTES(nbit_signature);
      fread(signature, sizeof(char), n_signature, sig);
      fscanf(sig, "\n");
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

  pgm_readpgminit(in, &cols, &rows, &maxval, &format);
  if (verbose > 0) 
    fprintf(stderr, "%s: embedding %d bits with strength %f in\n"
                    "  %d x %d host image, decomposition level %d\n",
                    progname, nbit_signature, alpha, cols, rows, level);

  image = pgm_allocarray(cols, rows);
  for (row = 0; row < rows; row++)
    pgm_readpgmrow(in, image[row], cols, maxval, format);
  fclose(in);

  // decomposition of image
  init_dwt(cols, rows, filter_name, filter, level, method);
#ifdef POLLEN_STUFF
#include "pollen_stuff.c"
#endif
#ifdef PARAM_STUFF
#include "param_stuff.c"
#endif

  dwts = fdwt(image);

  p = dwts;

  // consider each resolution level
  while (p->level < level) 
    // descend one level
    p = p->coarse;

  // repeat binary watermark by sliding a 3-pixel window of approximation image
  n = 0;
  for (row = 0; row < p->image->height; row++) {
    for (col = 0; col < p->image->width - 3; col += 3) {
      double b1, b2, b3;
      double *f1 = &b1, *f2 = &b2, *f3 = &b3;

      // get all three approximation pixels in window
      b1 = get_pixel(p->image, col + 0, row);
      b2 = get_pixel(p->image, col + 1, row);
      b3 = get_pixel(p->image, col + 2, row);

      // bring selected pixels in ascending order
#define SWAP(A, B) {double *t = A; A = B; B = t;}
      if (*f1 > *f2) SWAP(f1, f2);
      if (*f2 > *f3) SWAP(f2, f3);
      if (*f1 > *f2) SWAP(f1, f2);

      set_signature_bit(n, wm_transform(alpha, *f1, *f2, *f3));

      if (verbose > 1)
        fprintf(stderr, "%s: extracting #%d (= %d) at (%d/%d); %f < %f < %f\n",
          progname, n, get_signature_bit(n % nbit_signature), col, row, *f1, *f2, *f3);

      n++;
    }
  }

  fprintf(out, "XEWM\n");  
  fprintf(out, "%d\n", n);
  fwrite(signature, sizeof(char), NBITSTOBYTES(n), out);
  fprintf(out, "\n");
  fclose(out);

  pgm_freearray(image, rows);

  exit(0);
}
