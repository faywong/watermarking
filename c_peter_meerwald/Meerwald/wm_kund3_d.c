#include "wm.h"
#include "signature.h"
#include "dwt.h"
#include "pgm.h"

char *progname;

void usage(void) {
  fprintf(stderr, "usage: %s [-e n] [-f n] [-F n] [-h] [-l n] [-o file] [-q n] [-s file] [-v n] file\n\n", progname);
  fprintf(stderr, "\t-e n\t\twavelet filtering method\n");
  fprintf(stderr, "\t-f n\t\tfilter number\n");
  fprintf(stderr, "\t-F file\t\tfilter definition file\n");
  fprintf(stderr, "\t-h\t\tprint usage\n");
  fprintf(stderr, "\t-l n\t\tembedding level\n");
  fprintf(stderr, "\t-o file\t\textracted signature file\n");
  fprintf(stderr, "\t-q n\t\tsignature strength\n");
  fprintf(stderr, "\t-s file\t\toriginal signature file\n");
  fprintf(stderr, "\t-v n\t\tverbosity level\n");
  exit(0);
}

int main(int argc, char *argv[]) {

  FILE *in = stdin;
  FILE *out = stdout;
  FILE *sig = NULL;

  gray **input_image;

  char signature_name[MAXPATHLEN];
  char output_name[MAXPATHLEN] = "(stdout)";
  char input_name[MAXPATHLEN] = "(stdin)";

  int r, c;
  int quality = 0;
  int seed = 0;
  int n = 0;
  int method = -1;
  int filter = 0;
  char filter_name[MAXPATHLEN] = "";

  char *binstr;

  int level = 0;

  int in_rows, in_cols, in_format;
  gray in_maxval;
  int rows, cols;
  int row, col;

  Image_tree dwts, p;

  int verbose = 0;

  progname = argv[0];

  pgm_init(&argc, argv);
  wm_init();

  while ((c = getopt(argc, argv, "e:f:F:h?l:o:q:s:v:")) != EOF) {
    switch (c) {
      case 'e':
        method = atoi(optarg);
        if (method < 0) {
          fprintf(stderr, "%s: wavelet filtering method %d out of range\n", progname, method);
          exit(1);
        }
        break;
      case 'f':
        filter = atoi(optarg);
        if (filter <= 0) {
          fprintf(stderr, "%s: filter number %d out of range\n", progname, filter);
          exit(1);
        }
        break;
      case 'F':
        strcpy(filter_name, optarg);
        break;
      case 'h':
      case '?':
        usage();
        break;
      case 'l':
        level = atoi(optarg);
        if (level < 1) {
          fprintf(stderr, "%s: embedding level out of range\n", progname);
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
      case 'q':
        quality = atoi(optarg);
        if (quality < 1) {
          fprintf(stderr, "%s: quality level %d out of range\n", progname, quality);
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
    if (strspn(line, "KD3SG") >= 5) {
      fscanf(sig, "%d\n", &nbit_signature);
      if (quality == 0)
        fscanf(sig, "%d\n", &quality);
      else
        fscanf(sig, "%*d\n");
      if (method < 0)
        fscanf(sig, "%d\n", &method);
      else
        fscanf(sig, "%*d\n");
      if (filter == 0)
        fscanf(sig, "%d\n", &filter);
      else
        fscanf(sig, "%*d\n");
      if (!strcmp(filter_name, ""))
        fscanf(sig, "%[^\n\r]\n", filter_name);
      else
        fscanf(sig, "%*[^\n\r]\n");
      if (level == 0)
        fscanf(sig, "%d\n", &level);
      else
        fscanf(sig, "%*d\n");
      fscanf(sig, "%d\n", &seed);
      srandom(seed);
      nbit_signature2 = nbit_signature;
      n_signature = n_signature2 = NBITSTOBYTES(nbit_signature2);
      binstr = malloc((nbit_signature2 + 1) * sizeof(char));
      fscanf(sig, "%[01]\n", binstr);
      binstr_to_sig2(binstr);
      free(binstr);
      init_signature_bits();
    }
    else {
      fprintf(stderr, "%s: invalid signature file %s\n", progname, signature_name);
      exit(1);
    }
    fclose(sig);
  }
  else {
    fprintf(stderr, "%s: signature file not specified, use -s file option\n", progname);
    exit(1);
  }

  pgm_readpgminit(in, &in_cols, &in_rows, &in_maxval, &in_format);

  cols = in_cols;
  rows = in_rows;

  if (verbose > 0)
    fprintf(stderr, "%s: extracting %d bits with quality %d from\n"
                    "  %d x %d host image, decomposition level %d\n",
                    progname, nbit_signature, quality, cols, rows, level);

  input_image = pgm_allocarray(in_cols, in_rows);

  for (row = 0; row < in_rows; row++)
    pgm_readpgmrow(in, input_image[row], in_cols, in_maxval, in_format);

  fclose(in);

  init_dwt(cols, rows, filter_name, filter, level, method);
/*#ifdef POLLEN_STUFF
#include "pollen_stuff.c"
#endif
#ifdef PARAM_STUFF
#include "param_stuff.c"
#endif*/

  dwts = fdwt(input_image);

 p = dwts;

#define STEP 3

  // consider each resolution level
  while (p->coarse->level < level)
   // descend one level
    p = p->coarse;

    // start to extracting watermark from beginning at each level
  {
    // get width and height of detail images at current level
    int lwidth = p->vertical->image->width;
    int lheight =  p->vertical->image->height;
  
    if (verbose > 1)
      fprintf(stderr, "%s: extracting at level %d now, size %d x %d\n",
        progname, p->coarse->level, lwidth, lheight);

    // consider each coefficient at resolution level
    for (row = 0; row < lwidth - STEP; row += STEP)
      for (col = 0; col < lheight - STEP; col += STEP) {
          double h, v, d;
          double *f1 = &h, *f2 = &v, *f3 = &d;
          double delta;

        // key-dependant coefficient selection
        r = row + 1 + random() % (STEP-2);
        c = col + 1 + random() % (STEP-2);

         // get coefficient values, one from each detail image
          h = get_pixel(p->horizontal->image, c, r);
          v = get_pixel(p->vertical->image, c, r);
          d = get_pixel(p->diagonal->image, c, r);

          // order pointer to coefficient values such that f1 <= f2 <= f3
#define SWAP(A, B) {double *t = A; A = B; B = t;}
          if (*f1 > *f2) SWAP(f1, f2);
          if (*f2 > *f3) SWAP(f2, f3);
          if (*f1 > *f2) SWAP(f1, f2);

          // calculate delta, the width of the bins
          delta = (*f3 - *f1) / (double) (2 * quality - 1);

          // set middle coefficient to closest appropriate bin,
          // according to watermark bit
          if (quality == 1) 
            set_signature_bit(n, (*f3 - *f2) < (*f2 - *f1));
          else {
            double l = *f1;
            int i = 0;
            while ((l + delta) < *f2) {
              l += delta; 
              i++;
            }
            if (i % 2)
              set_signature_bit(n, (l + delta - *f2) > (*f2 - l));
            else
              set_signature_bit(n, (l + delta - *f2) < (*f2 - l));
          }

          if (verbose > 2)
            fprintf(stderr, "%s: extracted bit #%d (= %d =? %d) at (%d/%d),\n"
                            "  f1=%lf, f2=%lf, f3=%lf\n", progname, n,
                            get_signature_bit(n), get_signature2_bit(n),
                            c, r, *f1, *f2, *f3);
          n++;
        }
  }

  fprintf(out, "KD3WM\n");
  fprintf(out, "%d\n", n);
  nbit_signature = n;
  binstr = malloc(sizeof(char) * (nbit_signature + 1));
  sig_to_binstr(binstr);
  fprintf(out, "%s\n", binstr);
  free(binstr);
  fclose(out);

  pgm_freearray(input_image, rows);

  exit(0);
}
