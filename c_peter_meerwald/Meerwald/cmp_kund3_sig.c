#include "wm.h"
#include "signature.h"

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

  char *binstr1;
  char *binstr2;

  int correlation_only = 0;

  int c, i;
  int quality = 0;
  int corr = 0, match = 0;
  int verbose = 0;
  int filter = 0;
  int method = 0;
  int level = 0;
  char filter_name[MAXPATHLEN] = "";

  int seed;
  char line[32];

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
  
  if (sig) {
    fgets(line, sizeof(line), sig);
    if (strspn(line, "KD3SG") >= 5) {
      fscanf(sig, "%d\n", &nbit_signature1);
      fscanf(sig, "%d\n", &quality);
      fscanf(sig, "%d\n", &method);
      fscanf(sig, "%d\n", &filter);
      fscanf(sig, "%[^\n\r]\n", filter_name);
      fscanf(sig, "%d\n", &level);
      fscanf(sig, "%d\n", &seed);
      srandom(seed);
      n_signature1 = NBITSTOBYTES(nbit_signature1);

      binstr1 = malloc((nbit_signature1 + 1) * sizeof(char));
      fscanf(sig, "%[01]\n", binstr1);
      binstr_to_sig1(binstr1);
      free(binstr1);
    }
    else {
      fprintf(stderr, "%s: invalid signature file %s\n", progname, signature_name);
      exit(1);
    }
    fclose(sig);
  }
  else {
    fprintf(stderr, "%s: original signature file not specified, use -s file option\n", progname);
    exit(1);
  }

  fgets(line, sizeof(line), in);
  if (strspn(line, "KD3WM") >= 5) {
    fscanf(in, "%d\n", &nbit_signature2);
    n_signature2 = NBITSTOBYTES(nbit_signature2);
    binstr2 = malloc((nbit_signature2 + 1) * sizeof(char));
    fscanf(in, "%[01]\n", binstr2);
    binstr_to_sig2(binstr2);
    free(binstr2);
  }
  else {
    fprintf(stderr, "%s: invalid watermark file %s\n", progname, input_name);
    exit(1);
  }

  if (verbose > 0) {
    fprintf(stderr, "signature length: %d\n", nbit_signature1);
    fprintf(stderr, "watermark length: %d\n", nbit_signature2);
  }

  for (i = 0; i < nbit_signature2; i++)
    if (get_signature1_bit(i % nbit_signature1) == get_signature2_bit(i))
      corr++, match++;
    else
      corr--;

  if (correlation_only)
    fprintf(out, "%lf\n", (double) corr / nbit_signature2);
  else {
    fprintf(out, "bit matches: %d/%d\n", match, nbit_signature2);
    fprintf(out, "correlation: %lf\n", (double) corr / nbit_signature2);
  }

  exit(0);
}
