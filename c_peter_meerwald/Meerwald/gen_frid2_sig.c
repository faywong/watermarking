#include "wm.h"
#include "signature.h"

char *progname;

void usage(void) {
  fprintf(stderr, "usage: %s [-a n] [-g n] [-o file] [-s n] file\n\n", progname);
  fprintf(stderr, "\t-a n\t\talpha factor (default 0.25)\n");
  fprintf(stderr, "\t-g n\t\tgamma factor (default 1.0)\n");
  fprintf(stderr, "\t-s n\t\tseed (default 0)\n");
  fprintf(stderr, "\t-h\t\tprint usage\n");
  fprintf(stderr, "\t-n n\t\twatermark length (default 100)\n");
  fprintf(stderr, "\t-o file\t\toutput file\n");
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
  int n = 100, nb;
  double a = 0.25;
  double g = 1.0;
  int s = 0;

  progname = argv[0];

#ifdef __EMX__
  _fsetmode(in, "b");
  _fsetmode(out, "b");
#endif

  while ((c = getopt(argc, argv, "a:g:h?n:o:s:S:")) != EOF) {
    switch (c) {
      case 'a':
        a = atof(optarg);
        if (a <= 0.0) {
          fprintf(stderr, "%s: alpha factor %f out of range\n", progname, a);
          exit(1);
        }
        break;
      case 'g':
        g = atof(optarg);
        if (g <= 0.0) {
          fprintf(stderr, "%s: gamma factor %f out of range\n", progname, a);
          exit(1);
        }
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

  if (!s)
    s = time(NULL) * getpid();
  srandom(s);

  if (sig) {
    char line[128];
    fgets(line, sizeof(line), sig);
    if (strspn(line, "FR2SG") >= 5) {
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

  fprintf(out, "FR2SG\n");
  fprintf(out, "%d\n", n);
  fprintf(out, "%f\n", a);
  fprintf(out, "%f\n", g);
  fprintf(out, "%d\n", s);
  fwrite(signature, sizeof(char), nb, out);
  fprintf(out, "\n");

  fclose(out);

  exit(0);
}
