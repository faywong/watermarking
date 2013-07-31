#include "wm.h"

char *progname;

void usage(void) {
  fprintf(stderr, "usage: %s [-h] [-s file] [-C] [-o file] [-v] file\n\n", progname);
  fprintf(stderr, "\t-C\t\toutput correlation only\n");
  fprintf(stderr, "\t-h\t\tprint usage\n");
  fprintf(stderr, "\t-o file\t\toutput file\n");
  fprintf(stderr, "\t-s file\t\tignored\n");
  exit(0);
}

int main(int argc, char *argv[]) {

  FILE *in = stdin;
  FILE *out = stdout;

  char output_name[MAXPATHLEN] = "(stdout)";
  char input_name[MAXPATHLEN] = "(stdin)";

  int c, i, n, ok;
  int levels;
  double alpha;
  double diff;
  char line[32];

  int correlation_only = 0;
  int verbose = 0;
	
  progname = argv[0];

  while ((c = getopt(argc, argv, "h?Co:v:s:")) != EOF) {
    switch (c) {
      case 'h':
      case '?':
        usage();
        break;
      case 'C':
        correlation_only = 1;
        break;
      case 's':
        break;
      case 'o':
        if ((out = fopen(optarg, "w")) == NULL) {
          fprintf(stderr, "%s: unable to open output file %s\n", progname, optarg);
          exit(1);
        }
        strcpy(output_name, optarg);
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
  
  fgets(line, sizeof(line), in);
  if (strspn(line, "DGWM") < 4) {
    fprintf(stderr, "%s: watermark file %s invalid\n", progname, input_name);
    exit(1);
  }

  fscanf(in, "%d\n", &levels);
  fscanf(in, "%lf\n", &alpha);

  n = 3 * levels;
  ok = 0;
  diff = 0.0;
  for (i = 0; i < levels; i++) {
    int m;
    double z, v;

    // HL subband
    fscanf(in, "%d %lf %lf\n", &m, &z, &v);
    if (verbose && !correlation_only) {
      if (m)
        fprintf(out, "%f %f\n",  z /  (double) m, (v * alpha) / (double) (1.0 * m));
      else
        fprintf(out, "0.0 0.0\n");
    }
    if (m) {
      ok += (z > v * alpha / (double) 1.0) ? 1 : 0;
      diff += ((z - v * alpha) / (double) (1.0 * m)); 
    }
    else
      n--;

    // LH subband
    fscanf(in, "%d %lf %lf\n", &m, &z, &v);
    if (verbose && !correlation_only) {
      if (m)
        fprintf(out, "%f %f\n", z / (double) m, (v * alpha) / (double) (1.0 * m));
      else
        fprintf(out, "0.0 0.0\n");
    }
    if (m) {
      ok += (z > v * alpha / (double) 1.0) ? 1 : 0;
      diff += ((z - v * alpha) / (double) (1.0 * m)); 
    }
    else
      n--;

    // HH subband
    fscanf(in, "%d %lf %lf\n", &m, &z, &v);
    if (verbose && !correlation_only) {
      if (m)
        fprintf(out, "%f %f\n", z / (double) m, (v * alpha) / (double) (1.0 * m));
      else
        fprintf(out, "0.0 0.0\n");
    }
    
    if (m) {
      ok += (z > v * alpha / (double) 1.0) ? 1 : 0;
      diff += ((z - v * alpha) / (double) (1.0 * m)); 
    }
    else
      n--;
  }

  if (!correlation_only)
    fprintf(out, "%d/%d, diff %f\n", ok, n, diff);
  fprintf(out, "%f\n", (double) ok / (double) n);

  fclose(out);
  fclose(in);

  exit(0);
}
