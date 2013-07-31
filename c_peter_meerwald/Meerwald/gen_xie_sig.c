#include "wm.h"
#include "signature.h"

char *progname;

void usage(void) {
  fprintf(stderr, "usage: %s [-a n] [-e n] [-f n] [-F file] [-l n] [-n n] [-o file] [-s file] [-S n] [-v n] file\n\n", progname);
  fprintf(stderr, "\t-a n\t\tembedding strength (default 0.5)\n");
  fprintf(stderr, "\t-e n\t\twavelet filtering method (default 2)\n");
  fprintf(stderr, "\t-f n\t\tfilter number (default 1)\n");
  fprintf(stderr, "\t-F file\t\tfilter definition file (default 'filter.dat')\n");
  fprintf(stderr, "\t-h\t\tprint usage\n");
  fprintf(stderr, "\t-l n\t\tembedding level (default 5)\n");
  fprintf(stderr, "\t-n n\t\twatermark bit length\n");
  fprintf(stderr, "\t-o file\t\toutput file\n");
  fprintf(stderr, "\t-s file\t\tuse signature file's embedding information\n");
  fprintf(stderr, "\t-S n\t\tseed\n");
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

  int verbose = 0;
  int c;
  int i;
  double a = 0.5;
  int l = 5;
  int n = 0, nb;
  int s = 0;
  int e = 2;
  int f = 1;
  char F[MAXPATHLEN] = "filter.dat";

  progname = argv[0];
  wm_init();

  while ((c = getopt(argc, argv, "a:e:f:F:h?l:n:o:s:S:v:")) != EOF) {
    switch (c) {
      case 'a':
        a = atof(optarg);
        if (a <= 0.0) {
          fprintf(stderr, "%s: embedding strength %f out of range\n", progname, a);
          exit(1);
        }
        break;
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
      case 'S':
        s = atoi(optarg);
        break;
      case 'v':
        verbose = atoi(optarg);
        if (verbose < 0) {
          fprintf(stderr, "%s: verbosity level %d out of range", progname, verbose);
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
      if (n == 0)
        fscanf(sig, "%d\n", &n);
      else
        fscanf(sig, "%*d\n");
      if (a == 0.0)
        fscanf(sig, "%lf\n", &a);
      else
        fscanf(sig, "%*f\n");
      if (e < 0)
        fscanf(sig, "%d\n", &e);
      else
        fscanf(sig, "%*d\n");
      if (f == 0)
        fscanf(sig, "%d\n", &f);
      else
        fscanf(sig, "%*d\n");
      if (!strcmp(F, ""))
        fscanf(sig, "%[^\n\r]\n", F);
      else
        fscanf(sig, "%*[^\n\r]\n");
      if (l == 0)
        fscanf(sig, "%d\n", &l);
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

  fprintf(out, "XESG\n");
  fprintf(out, "%d\n", n);
  fprintf(out, "%f\n", a);
  fprintf(out, "%d\n", e);
  fprintf(out, "%d\n", f);
  fprintf(out, "%s\n", F);
  fprintf(out, "%d\n", l);
  fprintf(out, "%ld\n", random());
  fwrite(signature, sizeof(char), nb, out);
  fprintf(out, "\n");

  fclose(out);

  exit(0);
}
