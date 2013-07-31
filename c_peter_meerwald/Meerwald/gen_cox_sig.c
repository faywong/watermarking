#include "wm.h"

char *progname;

void usage(void) {
  fprintf(stderr, "usage: %s [-a n] [-d n] [-m n] [-n n] [-o file] [-s file] [-S n]\n\n", progname);
  fprintf(stderr, "\t-a n\t\talpha factor (default 0.3)\n");
  fprintf(stderr, "\t-d n\t\tdeviation (default 1.0)\n");
  fprintf(stderr, "\t-h\t\tprint usage\n");
  fprintf(stderr, "\t-m n \t\tmean value (default 0.0)\n");
  fprintf(stderr, "\t-n n\t\twatermark length (default 100)\n");
  fprintf(stderr, "\t-o file\t\toutput file\n");
  fprintf(stderr, "\t-s file\t\tuse signature file's embedding information\n");
  fprintf(stderr, "\t-S n\t\tseed\n");
  exit(0);
}

int main(int argc, char *argv[]) {
  FILE *out = stdout;
  FILE *sig = NULL;

  char output_name[MAXPATHLEN] = "(stdout)";
  char signature_name[MAXPATHLEN];

  int c;
  int n = 100;
  int s = 0;
  double a = 0.3;
  double m = 0.0;
  double d = 1.0;

  progname = argv[0];

  while ((c = getopt(argc, argv, "a:d:h?m:n:o:s:S:")) != EOF) {
    switch (c) {
      case 'a':
        a = atof(optarg);
        if (a <= 0.0) {
          fprintf(stderr, "%s: alpha factor %f out of range\n", progname, a);
          exit(1);
        }
        break;
      case 'd':
        d = atof(optarg);
        if (d <= 0.0) {
          fprintf(stderr, "%s: deviation %f out of range\n", progname, d);
          exit(1);
        }
        break;
      case 'h':
      case '?':
        usage();
        break;
      case 'm':
        m = atof(optarg);
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
      case 'S':
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

    if (sig) {
    char line[32];
    fgets(line, sizeof(line), sig);
    if (strspn(line, "CXSG") >= 4) {
      if (n == 0)
        fscanf(sig, "%d\n", &n);
      else
        fscanf(sig, "%*d\n");
      if (a == 0.0)
        fscanf(sig, "%lf\n", &a);
      else
        fscanf(sig, "%*f\n");
      if (m == 0.0)
        fscanf(sig, "%lf\n", &m);
      else
        fscanf(sig, "%*f\n");
      if (d == 0.0)
        fscanf(sig, "%lf\n", &d);
      else
        fscanf(sig, "%*f\n");
    }
    else {
      fprintf(stderr, "%s: invalid signature file %s\n", progname, signature_name);
      exit(1);
    }
    fclose(sig);
  }

  if (s)
    srandom(s);
  else
    srandom(time(NULL) * getpid());

  fprintf(out, "CXSG\n");
  fprintf(out, "%d\n", n);
  fprintf(out, "%f\n", a);
  fprintf(out, "%f\n", m);
  fprintf(out, "%f\n", d);

  n >>= 1;
  while (n > 0) {
    double x;
    double x1, x2;

    /*
     * Algorithm P (Polar method for normal deviates),
     * Knuth, D., "The Art of Computer Programming", Vol. 2, 3rd Edition, p. 122
     */
    do {
      x1 = 2.0 * ((random() & RAND_MAX) / ((double) RAND_MAX + 1.0)) - 1.0;
      x2 = 2.0 * ((random() & RAND_MAX) / ((double) RAND_MAX + 1.0)) - 1.0;
      x = x1 * x1 + x2 * x2;
    } while (x >= 1.0);
    x1 *= sqrt((-2.0) * log(x) / x);
    x2 *= sqrt((-2.0) * log(x) / x);

    fprintf(out, "%f\n", m + d * x1);
    fprintf(out, "%f\n", m + d * x2);

    n--;
  }

  fclose(out);

  exit(0);
}
