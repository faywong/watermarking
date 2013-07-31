#include "wm.h"
#include "signature.h"

char *progname;

void usage(void) {
  fprintf(stderr, "usage: %s [-e n] [-f n] [-F file] [-l n] [-n n] [-o file] [-q n] [-s n] file\n\n", progname);
  fprintf(stderr, "\t-h\t\tprint usage\n");
  fprintf(stderr, "\t-e n\t\twavelet filtering method (default 2)\n");
  fprintf(stderr, "\t-f n\t\tfilter number (default 1)\n");
  fprintf(stderr, "\t-F file\t\tfilter definition file (default 'filter.dat')\n");
  fprintf(stderr, "\t-l n\t\tembedding level (default 1)\n");
  fprintf(stderr, "\t-n n\t\twatermark bit length\n");
  fprintf(stderr, "\t-o file\t\toutput file\n");
  fprintf(stderr, "\t-q n\t\tsignature strength (default 1.0)\n");
  fprintf(stderr, "\t-s n\t\tseed\n");
  exit(0);
}

int main(int argc, char *argv[]) {
  FILE *in = stdin;
  FILE *out = stdout;

  char output_name[MAXPATHLEN] = "(stdout)";
  char input_name[MAXPATHLEN] = "(stdin)";

  int c;
  int i;
  int l = 1;
  int n = 0, nb;
  int s = 0;
  double q = 1.0;
  int e = 2;
  int f = 1;
  char F[MAXPATHLEN] = "filter.dat";

  progname = argv[0];

#ifdef __EMX__
  _fsetmode(in, "b");
  _fsetmode(out, "b");
#endif

  while ((c = getopt(argc, argv, "e:f:F:h?l:n:o:q:s:")) != EOF) {
    switch (c) {
      case 'e':
        e = atoi(optarg);
        if (e < 0) {
          fprintf(stderr, "%s: wavelet filtering method %d out of range\n", progname, e);
        }
        break;
      case 'f':
        f = atoi(optarg);
        if (f <= 0) {
          fprintf(stderr, "%s: filter number %d out of range\n", progname, f);
          exit(1);
        }
        break;
      case 'F':
        strcpy(F, optarg);
        break;
      case 'h':
      case '?':
        usage();
        break;
      case 'l':
        l = atoi(optarg);
        if (l < 1) {
          fprintf(stderr, "%s: embedding level out of range\n", progname);
          exit(1);
        }
        break;
      case 'n':
        n = atoi(optarg);
        if (n < 1 || n > 1000) {
          fprintf(stderr, "%s: watermark length %d out of range\n", progname, n);
          exit(1);
        }
        if (n % 4 != 0 || (int) sqrt(n / 4) * (int) sqrt(n / 4) != n) {
          fprintf(stderr, "%s: watermark length not divisible by 4 or not a square number\n", progname);
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
      case 'q':
        q = atof(optarg);
        if (q <= 0.0) {
          fprintf(stderr, "%s: signature strength factor %f out of range\n", progname, q);
          exit(1);
        }
        break;
      case 's':
        s = atoi(optarg);
        break;
    }
  }

  argc -= optind;
  argv += optind;

  if (argc > 0) {
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

  if (s)
    srandom(s);
  else
    srandom(time(NULL) * getpid());

  if (n > 0) {
    nb = fread(signature, sizeof(char), i = NBITSTOBYTES(n), in);
    if (nb < i) {
      fprintf(stderr, "%s: failed to read all %d signature bits from %s\n", progname, n, input_name);
      exit(1);
    }
  }
  else {
    int n_square;
    if (fscanf(in, "%128[^\n\r]", signature) == EOF) {
      fprintf(stderr, "%s: failed to read signature bits from %s\n", progname, input_name);
      exit(1);
    }
    nb = strlen(signature);
    n = NBYTESTOBITS(nb);
    n_square = (int) sqrt(n) * (int) sqrt(n);
    fprintf(stderr, "%s: got %d signature bits, truncated to %d\n", progname, n, n_square);
    n = n_square;
    nb = NBITSTOBYTES(n);
    if (n < 1) {
      fprintf(stderr, "%s: watermark length %d out of range\n", progname, n);
      exit(1);
    }
  }

  fprintf(out, "KDSG\n");
  fprintf(out, "%d\n", n);
  fprintf(out, "%d\n", e);
  fprintf(out, "%d\n", f);
  fprintf(out, "%s\n", F);
  fprintf(out, "%d\n", l);
  fprintf(out, "%f\n", q);
  fprintf(out, "%d\n", random());
  fwrite(signature, sizeof(char), nb, out);
  fprintf(out, "\n");

  fclose(out);

  exit(0);
}
