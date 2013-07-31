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

  int c, i;
  int pattern1, pattern2;
  double quality;
  double threshold1, threshold2;
  int blocksize;
  int corr = 0, match = 0;
  int verbose = 0;
  int skipping = 0;

  int correlation_only = 0;

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
    if (strspn(line, "BRSG") >= 4) {
      fscanf(sig, "%d\n", &nbit_signature1);
      fscanf(sig, "%d\n", &skipping);
      fscanf(sig, "%d\n", &pattern1);
      fscanf(sig, "%d\n", &pattern2);
      fscanf(sig, "%lf\n", &quality);
      fscanf(sig, "%lf\n", &threshold1);
      fscanf(sig, "%lf\n", &threshold2);
      fscanf(sig, "%d\n", &blocksize);
      fscanf(sig, "%d\n", &seed);
      srandom(seed);
      n_signature1 = NBITSTOBYTES(nbit_signature1);
      fread(signature1, sizeof(char), n_signature1, sig);
      fscanf(sig, "\n");
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
  if (strspn(line, "BRWM") >= 4) {
    fscanf(in, "%d\n", &nbit_signature2);
    n_signature2 = NBITSTOBYTES(nbit_signature2);
    fread(signature2, sizeof(char), n_signature2, in);
    fscanf(in, "\n");    
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
