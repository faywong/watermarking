#include "wm.h"
#include "signature.h"
#include "bruyn_common.h"

char *progname;

void usage(void) {
  fprintf(stderr, "usage: %s [-b n] [-k] [-n n] [-o file] [-pP n] [-q n] [-s file] [-S n] [-tT n] file\n\n", progname);
  fprintf(stderr, "\t-b n\t\tblock size (default 8)\n");
  fprintf(stderr, "\t-h\t\tprint usage\n");
  fprintf(stderr, "\t-k\t\tdisable block skipping\n");
  fprintf(stderr, "\t-n n\t\twatermark bit length\n");
  fprintf(stderr, "\t-o file\t\toutput file\n");
  fprintf(stderr, "\t-p n\t\tpattern type for zone 1 (default 1, 1.." NPATTERN_USAGE ")\n");
  fprintf(stderr, "\t-P n\t\tpattern type for zone 2 (default 2, 1.." NPATTERN_USAGE ")\n");
  fprintf(stderr, "\t-q n\t\tsignature strength (default 7.0)\n");
  fprintf(stderr, "\t-s file\t\tuse signature file's embedding information\n");
  fprintf(stderr, "\t-S n\t\tseed\n");
  fprintf(stderr, "\t-t n\t\tthreshold for noise (default " THRESHOLD_NOISE_USAGE ")\n");
  fprintf(stderr, "\t-T n\t\tthreshold for slope (default " THRESHOLD_SLOPE_USAGE ")\n");
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
  int i;
  int b = 8;
  int n = 0;
  int nb;
  int s = 0;
  int p1 = 1;
  int p2 = 2;
  double t1 = THRESHOLD_NOISE;
  double t2 = THRESHOLD_SLOPE;
  double q = 7.0;
  int skipping = 0;

  progname = argv[0];
  wm_init();

  while ((c = getopt(argc, argv, "b:h?n:o:p:P:q:s:S:t:T:k")) != EOF) {
    switch (c) {
      case 'b':
        b = atoi(optarg);
        if (b <= 0) {
          fprintf(stderr, "%s: block size %d out of range\n", progname, b);
          exit(1);
        }
        break;
      case 'k':
        skipping = 1;
        break;
      case 'h':
      case '?':
        usage();
        break;
      case 'n':
        n = atoi(optarg);
        if (n < 1 || n > 1000) {
          fprintf(stderr, "%s: watermark length %d out of range\n", progname, n);
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
      case 'p':
        p1 = atoi(optarg);
        if (p1 <= 0 || p1 > NPATTERN) {
          fprintf(stderr, "%s: pattern type out of range\n", progname);
          exit(1);
        }
        break;
      case 'P':
        p2 = atoi(optarg);
        if (p2 <= 0 || p2 > NPATTERN) {
          fprintf(stderr, "%s: pattern type out of range\n", progname);
          exit(1);
        }
        break;
      case 'q':
        q = atof(optarg);
        if (q <= 0.0) {
          fprintf(stderr, "%s: signature strength factor %f out of range\n", progname, q);
          exit(1);
        }
        break;
      case 't':
        t1 = atof(optarg);
        if (t1 <= 0) {
          fprintf(stderr, "%s: noise threshold %f out of range\n", progname, t1);
        }
        break;
      case 'T':
        t2 = atof(optarg);
        if (t2 <= 0) {
          fprintf(stderr, "%s: slope threshold %f out of range\n", progname, t2);
        }
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

  if (argc > 1) {
    usage();
    exit(1);
  }

  if (b % 2 > 0 || b <= 2) {
    fprintf(stderr, "%s: block size has to be even and greater than 2\n", progname);
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

  // read signature file and set options
  // command line options override signature file options
  if (sig) {
    char line[128];
    fgets(line, sizeof(line), sig);
    if (strspn(line, "BRSG") >= 4) {
      if (n == 0)
        fscanf(sig, "%d\n", &n);
      else
        fscanf(sig, "%*d\n");
      if (skipping == 0)
        fscanf(sig, "%d\n", &skipping);
      else
        fscanf(sig, "%*d\n");
      if (p1 == 0)
        fscanf(sig, "%d\n", &p1);
      else
        fscanf(sig, "%*d\n");
      if (p2 == 0)
        fscanf(sig, "%d\n", &p2);
      else
        fscanf(sig, "%*d\n");
      if (q == 0.0)
        fscanf(sig, "%lf\n", &q);
      else
        fscanf(sig, "%*f\n");
      if (t1 == 0.0)
        fscanf(sig, "%lf\n", &t1);
      else
        fscanf(sig, "%*f\n");
      if (t2 == 0.0)
        fscanf(sig, "%lf\n", &t2);
      else
        fscanf(sig, "%*f\n");
      if (b == 0)
        fscanf(sig, "%d\n", &b);
      else
        fscanf(sig, "%*d\n");
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

  if (n > 0) {
    nb = fread(signature, sizeof(char), i = NBITSTOBYTES(n), in);
    if (nb < i) {
      fprintf(stderr, "%s: failed to read all %d signature bits from %s\n", progname, n, input_name);
      exit(1);
    }
  }
  else {
    if (fscanf(in, "%128[^\n\r]", signature) == EOF) {
      fprintf(stderr, "%s: failed to read signature bits from %s\n", progname, input_name);
      exit(1);
    }
    nb = strlen(signature);
    n = NBYTESTOBITS(nb);
    fprintf(stderr, "%s: got %d signature bits\n", progname, n);
  }


  fprintf(out, "BRSG\n");
  fprintf(out, "%d\n", n);
  fprintf(out, "%d\n", skipping);
  fprintf(out, "%d\n", p1);
  fprintf(out, "%d\n", p2);
  fprintf(out, "%f\n", q);
  fprintf(out, "%f\n", t1);
  fprintf(out, "%f\n", t2);
  fprintf(out, "%d\n", b);
  fprintf(out, "%ld\n", random());
  fwrite(signature, sizeof(char), nb, out);
  fprintf(out, "\n");

  fclose(out);

  exit(0);
}
