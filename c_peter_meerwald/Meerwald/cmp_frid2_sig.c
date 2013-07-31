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

  int correlation_only = 0;

  int c, i;
  int corr1 = 0, match1 = 0;
  int corr2 = 0, match2 = 0;
  int verbose = 0;

  char line[1024];

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
    if (strspn(line, "FR2SG") >= 5) {
      fscanf(sig, "%d\n", &nbit_signature);
      fscanf(sig, "%*f\n");
      fscanf(sig, "%*f\n");
      fscanf(sig, "%*d\n");
      n_signature = NBITSTOBYTES(nbit_signature);
      fread(signature, sizeof(char), n_signature, sig);
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
  if (strspn(line, "FR2WM") >= 5) {
    fscanf(in, "%d\n", &nbit_signature1);
    n_signature1 = NBITSTOBYTES(nbit_signature1);
    fread(signature1, sizeof(char), n_signature1, in);
//    fscanf(in, "\n");    
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
    fprintf(stderr, "signature length: %d\n", nbit_signature);
    fprintf(stderr, "watermark length (low. freq.): %d\n", nbit_signature1);
    fprintf(stderr, "watermark length (med. freq.): %d\n", nbit_signature2);
  }

  for (i = 0; i < nbit_signature; i++) {
    if (get_signature_bit(i) == get_signature1_bit(i))
      corr1++, match1++;
    else
      corr1--;
    if (get_signature_bit(i) == get_signature2_bit(i))
      corr2++, match2++;
    else
      corr2--;
  }

  if (correlation_only)
    fprintf(out, "%lf\n", (double) (corr1 + corr2) / (nbit_signature1 + nbit_signature2));
  else {
    fprintf(out, "bit matches (low freq.): %d/%d\n", match1, nbit_signature1);
    fprintf(out, "correlation (low. freq.): %lf\n", (double) corr1 / nbit_signature1);
    fprintf(out, "bit matches (med. freq.): %d/%d\n", match2, nbit_signature2);
    fprintf(out, "correlation (med. freq.): %lf\n", (double) corr2 / nbit_signature2);
    fprintf(out, "total correlation: %lf\n", (double) (corr1 + corr2) / (nbit_signature1 + nbit_signature2));
  }

  exit(0);
}
