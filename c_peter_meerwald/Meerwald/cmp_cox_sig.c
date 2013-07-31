#include "wm.h"

char *progname;

void usage(void) {
  fprintf(stderr, "usage: %s [-h] [-C] [-o file] [-v n] -s file file\n\n", progname);
  fprintf(stderr, "\t-C\t\toutput correlation only\n");
  fprintf(stderr, "\t-h\t\tprint usage\n");
  fprintf(stderr, "\t-o file\t\toutput file\n");
  fprintf(stderr, "\t-s file\t\toriginal signature file\n");
  fprintf(stderr, "\t-v n\t\tverbosity level\n");
  exit(0);
}

int main(int argc, char *argv[]) {

  FILE *in = stdin;
  FILE *out = stdout;
  FILE *sig = NULL;

  char signature_name[MAXPATHLEN];
  char output_name[MAXPATHLEN] = "(stdout)";
  char input_name[MAXPATHLEN] = "(stdin)";

  int c;
  int sig_n, in_n;
  double sig_a;
  double sig_mean, sig_variance;
  double s1, s2, s3;
  char line[32];
 
  int verbose = 0;
  int correlation_only = 0;

  progname = argv[0];

  while ((c = getopt(argc, argv, "h?Co:s:v:")) != EOF) {
    switch (c) {
      case 'h':
      case '?':
        usage();
        break;
      case 'C':
        correlation_only = 1;
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
    if ((in = fopen(argv[0], "r")) == NULL) {
      fprintf(stderr, "%s: unable to open input file %s\n", progname, argv[0]);
      exit(1);
    }
    else
      strcpy(input_name, argv[0]);
  }
  
  if (!sig) {
    fprintf(stderr, "%s: original signature file not specified, use -s file option\n", progname);
    exit(1);
  }

  fgets(line, sizeof(line), sig);
  if (strspn(line, "CXSG") < 4) {
    fprintf(stderr, "%s: original signature file %s invalid\n", progname, signature_name);
    exit(1);
  }

  fgets(line, sizeof(line), in);
  if (strspn(line, "CXWM") < 4) {
    fprintf(stderr, "%s: watermark file %s invalid\n", progname, input_name);
    exit(1);
  }

  fscanf(sig, "%d\n", &sig_n);
  fscanf(in, "%d\n", &in_n);
  if (sig_n != in_n) {
    fprintf(stderr, "%s: watermark length mismatch (original %d, input %d)\n", progname, sig_n, in_n);
    exit(1);
  }
  if (sig_n <= 0 || sig_n > 32000) {
    fprintf(stderr, "%s: invalid original watermark length %d\n", progname, sig_n);
    exit(1);
  }
  if (in_n <= 0 || in_n > 32000) {
    fprintf(stderr, "%s: invalid watermark length %d\n", progname, in_n);
    exit(1);
  }

  fscanf(sig, "%lf\n", &sig_a);

  fscanf(sig, "%lf\n", &sig_mean);
  fscanf(sig, "%lf\n", &sig_variance);

  /*
   * normalized correlation
   * Craver, S., "Can Invisible Watermarks Resolve Rightful Ownership?", IBM Research Report, 1996, p. 5
   */

  s1 = s2 = s3 = 0.0;
  while (in_n > 0) {
    double sig_x, in_x;

    fscanf(sig, "%lf\n", &sig_x);
    fscanf(in, "%lf\n", &in_x);

    if (verbose >= 1) {
      fprintf(stderr, "orig %f input %f\n", sig_x, in_x);  
    }

    s1 += sig_x * in_x;
    s2 += in_x * in_x;
    s3 += sig_x * sig_x;

    in_n--;
  }

  fprintf(out, "%f\n", s1 / sqrt(s2 * s3));

  fclose(sig);
  fclose(out);
  fclose(in);

  exit(0);
}
