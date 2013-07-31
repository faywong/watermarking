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

  char *binstr;

  int correlation_only = 0;

  int c, i;
  int quality = 0;
  int blocksize = 0;
  int corr = 0, match = 0;
  int verbose = 0;
  int filter = 0;
  int method = 0;
  int level = 0;
  char filter_name[MAXPATHLEN] = "";
  int k;

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
    if (strspn(line, "KD2SG") >= 5) {
      fscanf(sig, "%d\n", &nbit_signature1);
      fscanf(sig, "%d\n", &quality);
      fscanf(sig, "%d\n", &blocksize);
      fscanf(sig, "%d\n", &method);
      fscanf(sig, "%d\n", &filter);
      fscanf(sig, "%[^\n\r]\n", filter_name);
      fscanf(sig, "%d\n", &level);
      fscanf(sig, "%d\n", &seed);
      srandom(seed);
      n_signature1 = NBITSTOBYTES(nbit_signature1);

      binstr = malloc((nbit_signature1 + 1) * sizeof(char));
      fscanf(sig, "%[01]\n", binstr);
      binstr_to_sig1(binstr);
      free(binstr);
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
  if (strspn(line, "KD2WM") >= 5) {
    int max_nbit_signature;
    int min_nbit_signature = -1;
    double pe[100];
    double pe_sum;
    double alpha[100];
    char *w[100];

    fscanf(in, "%d\n", &max_nbit_signature);

    k = 0;
    while (!feof(in) && k < 100) {
      int e;

      fscanf(in, "%d\n", &nbit_signature2);
      w[k] = binstr = malloc(sizeof(char) * (nbit_signature2 + 1));
      fscanf(in, "%[01]\n", binstr);

      binstr_to_sig2(binstr);

      if (nbit_signature2 < min_nbit_signature || min_nbit_signature == -1)
        min_nbit_signature = nbit_signature2;
      e = 0;
      for (i = 0; i < nbit_signature2; i += 2) {
        if (get_signature1_bit(i % nbit_signature1) != get_signature2_bit(i))
          e++;
      }
      if (e > 0)
        pe[k++] = log( (1 - (e / (double) nbit_signature2)) / (e / (double) nbit_signature2));
      else
        pe[k++] = 0;
    }

    pe_sum = 0.0;
    for (i = 0; i < k; i++) {
// fprintf(stderr, "XXX pe[%d] = %f\n", i, pe[i]);
      pe_sum += pe[i];
    }

    for (i = 0; i < k; i++) {
      if (pe_sum != 0)
        alpha[i] = pe[i] / pe_sum;
      else
        alpha[i] = 1.0;
    }

    nbit_signature = min_nbit_signature;
    for (i = 0; i < min_nbit_signature; i++) {
      double s = 0.0;
      int j;

      for (j = 0; j < k; j++) {
        int bit;
//fprintf(stderr, "XXX %d %d\n", i, j);
        binstr_to_sig2(w[j]);
        bit = get_signature2_bit(i) ? 1 : -1;
        s += alpha[j] * bit;
      }
// fprintf(stderr, "YYY %d %f\n", i, s);
      set_signature_bit(i, s > 0 ? 1 : 0);
    }

    free(binstr);
  }
  else {
    fprintf(stderr, "%s: invalid watermark file %s\n", progname, input_name);
    exit(1);
  }

  if (verbose > 0) {
    fprintf(stderr, "signature length: %d\n", nbit_signature1);
    fprintf(stderr, "watermark length: %d\n", nbit_signature);
  }

  for (i = 0; i < nbit_signature; i++)
    if (get_signature1_bit(i % nbit_signature1) == get_signature2_bit(i))
      corr++, match++;
    else
      corr--;

  if (correlation_only)
    fprintf(out, "%lf\n", (double) corr / nbit_signature2);
  else {
    fprintf(stderr, "redundant blocks: %d\n", k);
    fprintf(out, "bit matches: %d/%d\n", match, nbit_signature2);
    fprintf(out, "correlation: %lf\n", (double) corr / nbit_signature2);
  }

  exit(0);
}
