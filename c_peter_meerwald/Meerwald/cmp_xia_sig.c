#include "wm.h"

char *progname;

void usage(void) {
  fprintf(stderr, "usage: %s [-h] [-C] [-o file] [-v n] -s file file\n\n", progname);
  fprintf(stderr, "\t-C\t\toutput correlation only\n");
  fprintf(stderr, "\t-h\t\tprint usage\n");
  fprintf(stderr, "\t-o file\t\toutput file\n");
  fprintf(stderr, "\t-v n\t\tverbosity level\n");
  fprintf(stderr, "\t-s file\t\toriginal signature file\n");
  exit(0);
}

int main(int argc, char *argv[]) {

  FILE *in = stdin;
  FILE *out = stdout;
  FILE *sig = NULL;

  char signature_name[MAXPATHLEN];
  char output_name[MAXPATHLEN] = "(stdout)";
  char input_name[MAXPATHLEN] = "(stdin)";

  int c, i, j, n;
  int in_level;
  double *cumul_watermark, *orig_watermark;
  int *cumul_watermark_count;
  int sig_n, in_n;
  double sig_a;
  int sig_l;
  int sig_e, sig_f;
  double s1, s2, s3;
  double correlation, maxcorrelation;
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
  if (strspn(line, "XASG") < 4) {
    fprintf(stderr, "%s: original signature file %s invalid\n", progname, signature_name);
    exit(1);
  }

  fgets(line, sizeof(line), in);
  if (strspn(line, "XAWM") < 4) {
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
  if (in_n != sig_n) {
    fprintf(stderr, "%s: invalid watermark length %d, does not match signature length\n", progname, in_n);
    exit(1);
  }

  fscanf(sig, "%lf\n", &sig_a);
  fscanf(sig, "%d\n", &sig_l);
  fscanf(sig, "%d\n", &sig_e);
  fscanf(sig, "%d\n", &sig_f);
  fscanf(sig, "%*[^\n\r]\n");

  orig_watermark = malloc(sig_n * sizeof(double));
  for (i = 0; i < sig_n; i++)
    fscanf(sig, "%lf\n", &orig_watermark[i]);
  fclose(sig);

  fscanf(in, "%d\n", &in_level);

  cumul_watermark = malloc(in_n * sizeof(double));
  cumul_watermark_count = malloc(in_n * sizeof(int));

  for (i = 0; i < in_n; i++) {
    cumul_watermark_count[i] = 0;
    cumul_watermark[i] = 0.0;
  }

  /*
   * normalized correlation
   * Craver, S., "Can Invisible Watermarks Resolve Rightful Ownership?", IBM Research Report, 1996, p. 5
   */

  maxcorrelation = -10000.0;
  for (i = 0; i < in_level; i++) {
    fscanf(in, "%d\n", &n);

    s1 = s2 = s3 = 0.0;
    for (j = 0; j < n; j++) {
      double in_x, sig_x;

      sig_x = orig_watermark[j % sig_n];
      fscanf(in, "%lf\n", &in_x);

      s1 += sig_x * in_x;
      s2 += in_x * in_x;
      s3 += sig_x * sig_x;

      if (verbose > 2)
        fprintf(stderr, "%s: level %d; orig %f input %f\n", progname, i, sig_x, in_x);

      cumul_watermark[j % in_n] += in_x;
      cumul_watermark_count[j % in_n]++;      
    }

    correlation = s1 / sqrt(s2 * s3);
    if (correlation > maxcorrelation)
      maxcorrelation = correlation;

    if (!correlation_only)
      fprintf(out, "%s: correlation subband %d: %f\n", progname, i, correlation);
  }

  s1 = s2 = s3 = 0.0;
  for (i = 0; i < in_n; i++) {
    double in_x, sig_x;

    if (cumul_watermark_count[i] <= 0) continue;
    in_x = cumul_watermark[i] / (double) cumul_watermark_count[i];
    sig_x = orig_watermark[i];

    s1 += sig_x * in_x;
    s2 += in_x * in_x;
    s3 += sig_x * sig_x;
  }

  correlation = s1 / sqrt(s2 * s3);
  if (!correlation_only)
    fprintf(out, "%s: cumultative correlation: %f\n", progname, correlation);
      
  if (correlation > maxcorrelation)
    maxcorrelation = correlation;
      
  if (!correlation_only)  
    fprintf(out, "%s: max. correlation: %f\n", progname, maxcorrelation);
  else
    fprintf(out, "%f\n", maxcorrelation);

  fclose(out);
  fclose(in);

  free(orig_watermark);
  free(cumul_watermark);
  free(cumul_watermark_count);

  exit(0);
}
