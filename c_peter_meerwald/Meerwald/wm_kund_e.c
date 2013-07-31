#include "wm.h"
#include "signature.h"
#include "wm_dwt.h"
#include "pgm.h"

char *progname;

void usage(void) {
  fprintf(stderr, "usage: %s [-e n] [-f n] [-F n] [-h] [-o file] [-q n] [-v n] -s file file\n\n", progname);
  fprintf(stderr, "\t-e n\t\twavelet filtering method\n");
  fprintf(stderr, "\t-f n\t\tfilter number\n");
  fprintf(stderr, "\t-F file\t\tfilter definition file\n");
  fprintf(stderr, "\t-h\t\tprint usage\n");
  fprintf(stderr, "\t-o file\t\toutput (watermarked) file\n");
  fprintf(stderr, "\t-q n\t\tquantization/quality factor\n");
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

  int c;
  int row, col;

  int n;

  double quality = 0.0;

  int filter = 0;
  int method = -1;
  int level = 0;
  char filter_name[MAXPATHLEN] = "";

  int seed;
  int verbose = 0;

  gray **image;
  Image_tree dwts;

  gray maxval;
  int rows, cols, colors, format;

  progname = argv[0];

  pgm_init(&argc, argv);

#ifdef __EMX__
  _fsetmode(in, "b");
  _fsetmode(out, "b");
#endif

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
        if ((out = fopen(optarg, "wb")) == NULL) {
          fprintf(stderr, "%s: unable to open output file %s\n", progname, optarg);
          exit(1);
        }
        strcpy(output_name, optarg);
        break;
      case 'q':
        quality = atoi(optarg);
        if (quality <= 0) {
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

  if (argc == 1 && *argv[0] != '-')
    if ((in = fopen(argv[0], "rb")) == NULL) {
      fprintf(stderr, "%s: unable to open input file %s\n", progname, argv[0]);
      exit(1);
    }
    else
      strcpy(input_name, argv[0]);

  if (sig) {
    char line[32];
    fgets(line, sizeof(line), sig);
    if (strspn(line, "KDSG") >= 4) {
      fscanf(sig, "%d\n", &n);
      if (quality == 0.0)
        fscanf(sig, "%lf\n", &quality);
      else
        fscanf(sig, "%*f\n");
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
    fprintf(stderr, "%s: signature file not specified, use -s file option\n", progname);
    exit(1);
  }

  pgm_readpgminit(in, &cols, &rows, &maxval, &format);

  image = pgm_allocarray(cols, rows);

  for (row = 0; row < rows; row++)
    pgm_readpgmrow(in, image[row], cols, maxval, format);

  fclose(in);

  // check watermark dimensions and decomposition level


  // decomposition of image
  init_dwt(cols, rows, filter_name, filter, level, method);
#ifdef POLLEN_STUFF
  {
    double alpha, beta;
    char *alpha_str = getenv("POLLEN_ALPHA"), *beta_str = getenv("POLLEN_BETA");
    
    if (alpha_str && beta_str) {
      alpha = atof(alpha_str);  
      beta = atof(beta_str);
     
      if (alpha < -M_PI || alpha >= M_PI) {
        fprintf(stderr, "%s: pollen - alpha %f out of range\n", progname, alpha);
        exit(1);
      }
      
      if (beta < -M_PI || beta >= M_PI) {
        fprintf(stderr, "%s: pollen - beta %f out of range\n", progname, beta);
        exit(1);
      }
      
      if (verbose > 7)
        fprintf(stderr, "%s: pollen - alpha %f, beta %f\n", progname, alpha, beta);
      
      dwt_pollen_filter(alpha, beta);
    }
  }
#endif

  dwts = fdwt(image);

  // create 'image' from binary watermark 

  // decomposition of watermark
  init_dwt(cols, rows, filter_name, filter, 1, method);
//  dwts = fdwt(watermark);

  // calculate mean value of image and set alpha

  // setup of contrast sensitivity matrix 

  // segment detail images at each level

  // calculate DFT of each segment

  // compute salience for each segment

  // calculate gamma or each detail image

  // embed watermark

  // reconstruction of watermarked image

  idwt(dwts, image);

  pgm_writepgminit(out, cols, rows, maxval, 0);

  for (row = 0; row < rows; row++)
    pgm_writepgmrow(out, image[row], cols, maxval, 0);

  fclose(out);

  pgm_freearray(image, rows);

  exit(0);
}
