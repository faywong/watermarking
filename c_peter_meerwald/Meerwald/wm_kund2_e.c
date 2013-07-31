#include "wm.h"
#include "signature.h"
#include "dwt.h"
#include "pgm.h"

char *progname;

void usage(void) {
  fprintf(stderr, "usage: %s [-e n] [-f n] [-F n] [-h] [-l n] [-o file] [-q n] [-v n] -s file file\n\n", progname);
  fprintf(stderr, "\t-a n\t\toverall embedding strength\n");
  fprintf(stderr, "\t-e n\t\twavelet filtering method\n");
  fprintf(stderr, "\t-f n\t\tfilter number\n");
  fprintf(stderr, "\t-F file\t\tfilter definition file\n");
  fprintf(stderr, "\t-l n\t\tembedding level\n");
  fprintf(stderr, "\t-h\t\tprint usage\n");
  fprintf(stderr, "\t-o file\t\toutput (watermarked) file\n");
  fprintf(stderr, "\t-q n\t\tsignature strength (default 4)\n");
  fprintf(stderr, "\t-s file\t\tsignature to embed in input image\n");
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

  char *binstr;

  int r, c, n;
  int row, col;

  int quality = 0;
  int blocksize = 0;
  int filter = 0;
  int method = -1;
  int level = 0;
  char filter_name[MAXPATHLEN] = "";

  double alpha = 0.0;
  int seed;
  int verbose = 0;

  gray **image;
  Image_tree dwts, p;

  gray maxval;
  int rows, cols, format;

  progname = argv[0];

  pgm_init(&argc, argv);
  wm_init();

  while ((c = getopt(argc, argv, "a:e:f:F:h?l:o:q:s:v:")) != EOF) {
    switch (c) {
      case 'a':
        alpha = atof(optarg);
        break;
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
        if ((out = fopen(optarg, "wb")) == NULL) {
          fprintf(stderr, "%s: unable to open output file %s\n", progname, optarg);
          exit(1);
        }
        strcpy(output_name, optarg);
        break;
      case 'q':
        quality = atoi(optarg);
        if (quality < 1) {
          fprintf(stderr, "%s: quality factor %d out of range\n", progname, quality);
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
    char line[1024];
    fgets(line, sizeof(line), sig);
    if (strspn(line, "KD2SG") >= 5) {
      fscanf(sig, "%d\n", &nbit_signature);
      if (quality == 0)
        fscanf(sig, "%d\n", &quality);
      else
        fscanf(sig, "%*d\n");
      if (blocksize == 0)
        fscanf(sig, "%d\n", &blocksize);
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
      n_signature = NBITSTOBYTES(nbit_signature);
      binstr = malloc((nbit_signature + 1) * sizeof(char));
      fscanf(sig, "%[01]\n", binstr);
      binstr_to_sig(binstr);
      free(binstr);
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

  pgm_readpgminit(in, &cols, &rows, &maxval, &format);

  if (verbose > 0) 
    fprintf(stderr, "%s: embedding %d bits with quality %d in\n"
                    "  %d x %d host image, up to decomposition level %d\n",
                    progname, nbit_signature, quality, cols, rows, level);

  image = pgm_allocarray(cols, rows);

  for (row = 0; row < rows; row++)
    pgm_readpgmrow(in, image[row], cols, maxval, format);

  fclose(in);

  // decomposition of image
  init_dwt(cols, rows, filter_name, filter, level, method);
#ifdef POLLEN_STUFF
#include "pollen_stuff.c"
#endif
#ifdef PARAM_STUFF
#include "param_stuff.c"
#endif

  dwts = fdwt(image);

  p = dwts;

  // consider each resolution level
  while (p->coarse->level < level) {
    int lwidth = p->vertical->image->width;
    int lheight =  p->vertical->image->height;
    int l = p->vertical->level;
    int bx, by;
    int nblock;
    int bits_per_level;

    nblock = 0;
    bits_per_level = 0;
    for (bx = 0; bx < lwidth; bx += blocksize) {
      for (by = 0; by < lheight; by += blocksize) {
        int bw = MIN(bx + blocksize, lwidth);
        int bh = MIN(by + blocksize, lheight);
        int STEP;
        float MIN_DIFF;

    // start to embed signature from beginning at each level
    // get width and height of detail images at current level 


    if (verbose > 1) 
      fprintf(stderr, "%s: embedding at level %d now, block %d, size %d x %d at %d, %d\n",
        progname, l, nblock, bw, bh, bx, by);

    MIN_DIFF = ROUND(120.0 * alpha);
    STEP = ROUND(alpha * (-15.555) +  10.777);
// fprintf(stderr, "XXX %d %f\n", STEP, MIN_DIFF);

    n = 0;
    // consider each coefficient at resolution level
    for (row = by; row < bh - STEP; row += STEP)
      for (col = bx; col < bw - STEP; col += STEP) {
          double h, v, d;
          double *f1 = &h, *f2 = &v, *f3 = &d;
          double delta;

          // key-dependant coefficient selection
          r = row + 1+random() % (STEP-2);
          c = col + 1+random() % (STEP-2);

          // get coefficient values, one from each detail image
          h = get_pixel(p->horizontal->image, c, r);
          v = get_pixel(p->vertical->image, c, r);
          d = get_pixel(p->diagonal->image, c, r);

          // order pointer to coefficient values such that f1 <= f2 <= f3
#define SWAP(A, B) {double *t = A; A = B; B = t;}
          if (*f1 > *f2) SWAP(f1, f2);
          if (*f2 > *f3) SWAP(f2, f3);
          if (*f1 > *f2) SWAP(f1, f2);

          if (verbose > 2) 
            fprintf(stderr, "%s: embedding bit #%d (= %d) at (%d/%d),\n",
                            progname, n, get_signature_bit(n % nbit_signature), c, r);
          if (verbose > 5) 
            fprintf(stderr, "  h=%lf, v=%lf, d=%lf\n", h, v, d);
          if (verbose > 2)
            fprintf(stderr, "  f1=%lf, f2=%lf", *f1, *f2);

          if ((*f3 - *f1) < MIN_DIFF) {
            double adj = (MIN_DIFF - (*f3 - *f1)) / 2.0;
            *f1 -= adj;
            *f3 += adj;            
          }

          // calculate delta, the width of the bins
          delta = (*f3 - *f1) / (double) (2 * quality - 1);
          
          // set middle coefficient to closest appropriate bin, 
          // according to watermark bit
          bits_per_level++;
          if (quality == 1)
            *f2 = get_signature_bit(n % nbit_signature) ? *f3 : *f1;
          else {
            double l = get_signature_bit(n % nbit_signature) ? *f1 + delta : *f1;
            while ((l + 2 * delta) < *f2) l += 2 * delta;
            *f2 = (*f2 - l) < (l + 2 * delta - *f2) ? l : l + 2 * delta;
          }

          if (verbose > 2) 
            fprintf(stderr, " -> %lf, f3=%lf\n", *f2, *f3);

          // write pixels, one of them modified
          set_pixel(p->horizontal->image, c, r, h);
          set_pixel(p->vertical->image, c, r, v);
          set_pixel(p->diagonal->image, c, r, d);

          n++;

        }
      }
    }

    if (verbose > 1) 
      fprintf(stderr, "%s: embedded %d bits at level %d\n", 
        progname, bits_per_level, l);

    // descend one level
    p = p->coarse;
  }

  idwt(dwts, image);

  pgm_writepgminit(out, cols, rows, maxval, 0);

  for (row = 0; row < rows; row++)
    pgm_writepgmrow(out, image[row], cols, maxval, 0);

  fclose(out);

  pgm_freearray(image, rows);

  exit(0);
}
