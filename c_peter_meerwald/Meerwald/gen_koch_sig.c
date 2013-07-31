#include "wm.h"
#include "signature.h"

char *progname;

void usage(void) {
  fprintf(stderr, "usage: %s [-l n] [-n n] [-o file] [-q n] [-s file] [-S n] file\n\n", progname);
  fprintf(stderr, "\t-h\t\tprint usage\n");
  fprintf(stderr, "\t-l n\t\tsignature strength factor (default 5.0)\n");
  fprintf(stderr, "\t-n n\t\twatermark bit length\n");
  fprintf(stderr, "\t-o file\t\toutput file\n");
  fprintf(stderr, "\t-q n\t\tquantization (JPEG quality) factor (default 90)\n");
  fprintf(stderr, "\t-s file\t\tuse signature file's embedding information\n");
  fprintf(stderr, "\t-S n\t\tseed\n");
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
  int n = 0;
  int nb;
  int s = 0;
  int q = 90;
  double l = 5.0;

  progname = argv[0]; wm_init();

  while ((c = getopt(argc, argv, "h?l:n:o:q:s:S:")) != EOF) {
    switch (c) {
      case 'h':
      case '?':
        usage();
        break;
      case 'l':
        l = atof(optarg);
        if (l <= 0.0) {
          fprintf(stderr, "%s: signature strength factor %f out of range\n", progname, l);
          exit(1);
        }
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
      case 'q':
        q = atoi(optarg);
        if (q <= 0 || q > 100) {
          fprintf(stderr, "%s: quantization factor %d out of range\n", progname, q);
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

  if (argc == 1 && *argv[0] != '-') {
  if ((in = fopen(argv[0], "rb")) == NULL) {
    fprintf(stderr, "%s: unable to open input file %s\n", progname, argv[0]);
    exit(1);
  }
  else
    strcpy(input_name, argv[0]);
  }
  
  if (s)
    srandom(s);
  else
    srandom(time(NULL) * getpid());

  if (sig) {
    char line[128];
    fgets(line, sizeof(line), sig);
    if (strspn(line, "KCSG") >= 4) {
      if (n == 0)
        fscanf(sig, "%d\n", &n);
      else
        fscanf(sig, "%*d\n");
      if (l == 0.0)
        fscanf(sig, "%lf\n", &l);
      else
        fscanf(sig, "%*f\n");
      if (q == 0)
        fscanf(sig, "%d\n", &q);
      else
        fscanf(sig, "%*d\n");
    }
    else {
      fprintf(stderr, "%s: invalid signature file %s\n", progname, signature_name);
      exit(1);
    }
    fclose(sig);
  }

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

  fprintf(out, "KCSG\n");
  fprintf(out, "%d\n", n);
  fprintf(out, "%f\n", l);
  fprintf(out, "%d\n", q);
  fprintf(out, "%ld\n", random());
  fwrite(signature, sizeof(char), nb, out);
  fprintf(out, "\n");

  fclose(out);

  exit(0);
}
