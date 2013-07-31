#include "wm.h"
#include "signature.h"
#include "coord.h"
#include "gray.h"
#include "sort.h"
#include "bruyn_common.h"
#include "pgm.h"

char *progname;

// prints out program's parameters
void usage(void) {
  fprintf(stderr, "usage: %s [-b n] [-h] [-k] [-n n] [-o file] [-pP n] [-q n] [-tT n] [-v n] -s file file\n", progname);
  fprintf(stderr, "\t-b n\t\tblock size\n");
  fprintf(stderr, "\t-h\t\tprint usage\n");
  fprintf(stderr, "\t-k\t\tdisable block skipping\n");
  fprintf(stderr, "\t-n n\t\tnumber of signature bits to detect\n");
  fprintf(stderr, "\t-o file\t\textracted signature file\n");
  fprintf(stderr, "\t-p n\t\tpattern type for zone 1\n");
  fprintf(stderr, "\t-P n\t\tpattern type for zone 2\n");
  fprintf(stderr, "\t-q n\t\tsignature strength\n");
  fprintf(stderr, "\t-s file\t\tembedded signature\n");
  fprintf(stderr, "\t-t n\t\tthreshold for noise\n");
  fprintf(stderr, "\t-T n\t\tthreshold for slope\n");
  fprintf(stderr, "\t-v n\t\tverbosity level\n");
  exit(0);
}

int main(int argc, char *argv[]) {
  FILE *in = stdin;
  FILE *out = stdout;
  FILE *sig = NULL;

  gray** image;
  gray **block;
  gray **zone;
  gray **category1, **category2;
  gray maxval;
  double *slope;
  int rows, cols, format;
  int c;
  int i, j;
  int n;
  int col, row;
  int n_block;

  char signature_name[MAXPATHLEN];
  char input_name[MAXPATHLEN] = "(stdin)";
  char output_name[MAXPATHLEN] = "(stdout)";

  double quality = 0.0;
  double threshold_noise = 0.0;
  double threshold_slope = 0.0;
  int pattern1 = 0;
  int pattern2 = 0;
  int blocksize = 0;
  int seed;

  int verbose = 0;
  int skipping = 0;

  struct coords *coords;

  progname = argv[0];

  pgm_init(&argc, argv); wm_init2();

  // parse command line and set options
  while ((c = getopt(argc, argv, "b:h?n:o:p:P:q:s:t:T:v:k")) != EOF) {
    switch (c) {
      case 'h':
      case '?':
        usage();
        break;
      case 'k':  
        skipping = 1;
        break;
      case 'n':
        nbit_signature = atoi(optarg);
        if (nbit_signature <= 0 || nbit_signature > NBITSIGNATURE) {
          fprintf(stderr, "%s: invalid signature length %d\n", progname, nbit_signature);
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
      case 'p':
        pattern1 = atoi(optarg);
        if (pattern1 <= 0 || pattern1 > NPATTERN) {
          fprintf(stderr, "%s: pattern type out of range\n", progname);
          exit(1);
        }
        break;
      case 'P':
        pattern2 = atoi(optarg);
        if (pattern2 <= 0 || pattern2 > 3) {
          fprintf(stderr, "%s: pattern type out of range\n", progname);
          exit(1);
        }
        break;
      case 'q':
        quality = atof(optarg);
        if (quality <= 0) {
          fprintf(stderr, "%s: quality factor %f out of range\n", progname, quality);
        }
        break;
      case 's':
        if ((sig = fopen(optarg, "r")) == NULL) {
          fprintf(stderr, "%s: unable to open signature file %s\n", progname, optarg);
          exit(1);
        }
        strcpy(signature_name, optarg);
        break;
      case 't':
        threshold_noise = atof(optarg);
        if (threshold_noise <= 0) {
          fprintf(stderr, "%s: noise threshold %f out of range\n", progname, threshold_noise);
        }
        break;
      case 'T':
        threshold_slope = atof(optarg);
        if (threshold_slope <= 0) {
          fprintf(stderr, "%s: slope threshold %f out of range\n", progname, threshold_slope);
        }
        break;
      case 'v':
        verbose = atoi(optarg);
        if (verbose < 0) {
          fprintf(stderr, "%s: verbosity level %d out of range\n",progname, verbose);
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

  // open input image file or read from stdin
  if (argc == 1 && *argv[0] != '-') {
    if ((in = fopen(argv[0], "rb")) == NULL) {
      fprintf(stderr, "%s: unable to open input file %s\n", progname, argv[0]);
      exit(1);
    }
    else
      strcpy(input_name, argv[0]);
  }

  // read signature file and set options
  // command line options override signature file options
  if (sig) {
    char line[1024];
    fgets(line, sizeof(line), sig);
    if (strspn(line, "BRSG") >= 4) {
      if (nbit_signature == 0)
        fscanf(sig, "%d\n", &nbit_signature);
      else
         fscanf(sig, "%*d\n");
      if (skipping == 0)
        fscanf(sig, "%d\n", &skipping);
      else
        fscanf(sig, "%*d\n");
      if (pattern1 == 0)
        fscanf(sig, "%d\n", &pattern1);
      else
        fscanf(sig, "%*d\n");
      if (pattern2 == 0)
        fscanf(sig, "%d\n", &pattern2);
      else
        fscanf(sig, "%*d\n");
      if (quality == 0.0)
        fscanf(sig, "%lf\n", &quality);
      else
        fscanf(sig, "%*f\n");
      if (threshold_noise == 0.0)
        fscanf(sig, "%lf\n", &threshold_noise);
      else
        fscanf(sig, "%*f\n");
      if (threshold_slope == 0.0)
        fscanf(sig, "%lf\n", &threshold_slope);
      else
        fscanf(sig, "%*f\n");
      if (blocksize == 0)
        fscanf(sig, "%d\n", &blocksize);
      else
        fscanf(sig, "%*d\n");
      fscanf(sig, "%d\n", &seed);
      srandom(seed);
      n_signature = NBITSTOBYTES(nbit_signature);
      fread(signature, sizeof(char), n_signature, sig);
      init_signature_bits();
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

  if (pattern1 <= 0 || pattern2 <= 0 || pattern1 > NPATTERN || pattern2 > NPATTERN) {
    fprintf(stderr, "%s: invalid pattern type specified\n", progname);
    exit(1);
  }

  // read dimensions of input image file
  pgm_readpgminit(in, &cols, &rows, &maxval, &format);

  // see if we can extract all signature bits
  // we want at least half of the blocks untouched
  if (((rows / blocksize) * (cols / blocksize)) < nbit_signature / 2) {
    fprintf(stderr, "%s: image not large enough to contain %d bits of signature\n", progname, nbit_signature);
    exit(1);
  }
  n_block = blocksize * blocksize;

  // allocate structure to remember which blocks we already touched, 
  // allow plenty of room to skip over blocks
  if ((coords = alloc_coords(nbit_signature * 16)) == NULL) {
    fprintf(stderr, "%s: unable to allocate memory\n", progname);
    exit(1);
  }

  // read in input image file
  image = pgm_allocarray(cols, rows);
  for (row = 0; row < rows; row++)
    pgm_readpgmrow(in, image[row], cols, maxval, format);

  fclose(in);

  row = 0;
  col = 0;

  // allocate memory for one block
  block = alloc_grays(blocksize, blocksize);

  // allocate memory for zone classification
  zone = alloc_grays(blocksize, blocksize);

  // allocate memory for category classification
  category1 = alloc_grays(blocksize, blocksize);
  category2 = alloc_grays(blocksize, blocksize);

  // set up category classification array according to 
  // pattern type parameter
  for (i = 0; i < blocksize; i++)
    for (j = 0; j < blocksize; j++) {
      category1[j][i] = lookup_pattern(pattern1, i, j);
      category2[j][i] = lookup_pattern(pattern2, i, j);
    }

  // allocate memory for slope calculation
  slope = malloc(sizeof(double) * n_block);

  fprintf(out, "BRWM\n");
  fprintf(out, "%d\n", nbit_signature);

  // extract all the signature bits, one by one
  n = 0;
  while (n < nbit_signature) {
    int xb;
    int yb;
    int blocktype;
    double smax;
    int alpha, beta_minus, beta_plus;
    double mean_1A, mean_1B, mean_2A, mean_2B, mean_1, mean_2;
    int n_1A, n_1B, n_2A, n_2B, n_1, n_2;
    double sigma, sigma_1, sigma_2;
    int zone1_ok, zone2_ok;

    // find an unused block randomly, depending on seed
    do {
      xb = random() % (cols / blocksize);
      yb = random() % (rows / blocksize);
    } while (add_coord(coords, xb, yb) < 0);

    // copy image block
    fprintf(stderr, "XXX1 %d %d %d\n", xb*blocksize, yb*blocksize, blocksize);
    copy_grays_to_block(block, image, xb * blocksize, yb * blocksize, blocksize, blocksize);    
    fprintf(stderr, "XXX2\n");
    
    if (verbose > 0)
      fprintf(stderr, "detecting bit #%d in block at (%d/%d)\n", n, xb * blocksize, yb * blocksize);

    // sort luminance values in block to represent increasing function F
    sort_grays(block[0], n_block);

    // calculate slopes of F and determine smax, the max. slope of F
    // the index where smax occures is called alpha
    alpha = 0;
    smax = 0.0;
    for (i = 0; i < n_block - 1; i++) {
      slope[i] = block[0][i + 1] - block[0][i];
      if (slope[i] > smax) {
        smax = slope[i];
        alpha = i;
      }
    }
    slope[n_block - 1] = 0;

    // block type classification
    blocktype = BLOCKTYPE_UNKNOWN;

    if (smax < threshold_noise) {
      // block has noise contrast
      beta_minus = beta_plus = alpha;
      blocktype = BLOCKTYPE_NOISE;
    }
    else {
      // block has progressive or hard contrast, let's find out...

      beta_minus = alpha - 1;
      while (beta_minus >= 0 && smax - slope[beta_minus] <= threshold_slope)
        beta_minus--;

      beta_plus = alpha + 1;
      while (beta_plus < n_block && smax - slope[beta_plus] <= threshold_slope)
        beta_plus++;

      if (beta_minus + 1 == alpha && beta_plus - 1 == alpha)
        blocktype = BLOCKTYPE_HARD;
      else 
        blocktype = BLOCKTYPE_PROGRESSIVE;
    }

    if (verbose > 1) {
      fprintf(stderr, "blocktype: %d\n", blocktype);
      fprintf(stderr, "Smax = %lf, alpha = %d, beta- = %d, beta+ = %d\n", smax, alpha, beta_minus, beta_plus);
    }

    // block pixel classification
    for (i = 0; i < blocksize; i++)
      for (j = 0; j < blocksize; j++) {
        gray pixel = image[yb * blocksize + j][xb * blocksize + i];
        zone[j][i] = ZONE_VOID;
        switch (blocktype) {
          case BLOCKTYPE_PROGRESSIVE:
          case BLOCKTYPE_HARD:
            if (pixel < block[0][beta_minus])
              zone[j][i] = ZONE_1;  
            else if (pixel > block[0][beta_plus])
              zone[j][i] = ZONE_2;  
            break;
          case BLOCKTYPE_NOISE:
            if (pixel < block[0][n_block / 2])
              zone[j][i] = ZONE_1;
            else if (pixel > block[0][n_block / 2])
              zone[j][i] = ZONE_2;
            break;
          default:
            fprintf(stderr, "%s: invalid block type\n", progname);
            break;
        }
      }

    // calculate mean values for zone/categories
    mean_1A = mean_1B = mean_2A = mean_2B = mean_1 = mean_2 = 0.0;
    n_1A = n_1B = n_2A = n_2B = n_1 = n_2 = 0;
    for (i = 0; i < blocksize; i++)
      for (j = 0; j < blocksize; j++) {
        gray pixel = image[yb * blocksize + j][xb * blocksize + i];
        int pixel_zone = zone[j][i];
        int pixel_category = CATEGORY_VOID;
        if (pixel_zone == ZONE_1)
          pixel_category = category1[j][i];
        else if (pixel_zone == ZONE_2)
          pixel_category = category2[j][i];

        switch (pixel_zone | pixel_category) {
          case CLASSIFICATION_1A:
            n_1++;
            n_1A++;
            mean_1A += pixel;
            mean_1 += pixel;
            break;
          case CLASSIFICATION_1B:
            n_1++;
            n_1B++;
            mean_1B += pixel;
            mean_1 += pixel;
            break;
          case CLASSIFICATION_2A:
            n_2++;
            n_2A++;
            mean_2A += pixel;
            mean_2 += pixel;
            break;
          case CLASSIFICATION_2B:
            n_2++;
            n_2B++;
            mean_2B += pixel;
            mean_2 += pixel;
            break;
        }
      }

    if (n_1 && n_1A && n_1B) {   
      mean_1 /= (double) n_1;
      mean_1A /= (double) n_1A;
      mean_1B /= (double) n_1B;
      zone1_ok = 1;
    }
    else {
      mean_1 = mean_1A = mean_1B = 0.0;
      zone1_ok = 0;
      if (verbose > 0)
        fprintf(stderr, "zone 1 unusable\n");
    }
   
    if (n_2 && n_2A && n_2B) {
      mean_2 /= (double) n_2;
      mean_2A /= (double) n_2A;
      mean_2B /= (double) n_2B;
      zone2_ok = 1;
    }
    else {
      mean_2 = mean_2A = mean_2B = 0.0;
      zone2_ok = 0;
      if (verbose > 0)
        fprintf(stderr, "zone 2 unusable\n");
    }  

    // bit extraction
    if (zone1_ok && zone2_ok) {
      sigma_1 = mean_1A - mean_1B;
      sigma_2 = mean_2A - mean_2B;

      if (verbose > 2) {
        fprintf(stderr, "m_1A = %lf, m_1B = %lf\n", mean_1A, mean_1B);
        fprintf(stderr, "m_2A = %lf, m_2B = %lf\n", mean_2A, mean_2B);
        fprintf(stderr, "sigma1 = %lf, sigma2 = %lf\n", sigma_1, sigma_2);
      }

#define EPSILON 0.001
      if (fabs(sigma_1 * sigma_2) < EPSILON) {
        // case 3
        sigma = MAX(fabs(sigma_1), fabs(sigma_2));
        set_signature_bit(n, sigma > 0.0);
        if (verbose > 0)
          fprintf(stderr, "case 3, bit #%d = %d\n", n, sigma > 0.0);
      }
      else if (sigma_1 * sigma_2 > 0.0) {
        // case 1
        set_signature_bit(n, sigma_1 > 0.0); 
        if (verbose > 0)
          fprintf(stderr, "case 1, bit #%d = %d\n", n, sigma_1 > 0.0);
      }
      else if (sigma_1 * sigma_2 < 0.0) {
        // case 2
        sigma = (double) (n_1A + n_1B) * sigma_1 + (double) (n_2A + n_2B) * sigma_2;
        set_signature_bit(n, sigma > 0.0);
        if (verbose > 0)
          fprintf(stderr, "case 2, bit #%d = %d\n", n, sigma > 0.0);
      }
    }
    else if (zone1_ok) {
      set_signature_bit(n, mean_1A > mean_1B);
      if (verbose > 0)
        fprintf(stderr, "case 4, bit #%d = %d\n", n, mean_1A > mean_1B);
    }
    else if (zone2_ok) {
      set_signature_bit(n, mean_2A > mean_2B);
      if (verbose > 0)
        fprintf(stderr, "case 5, bit #%d = %d\n", n, mean_2A > mean_2B);
    }
    else {
      // pathological case - can it ever happen?
      if (verbose > 0)
        fprintf(stderr, "block skipped\n");
      if (!skipping) continue;
    }

    n++;      
  }

  free_grays(category2);
  free_grays(category1);
  free_grays(zone);
  free_grays(block);

  // write extracted signature

  fwrite(signature, sizeof(char), n_signature, out);
  fclose(out);

  pgm_freearray(image, rows);

  exit(0);
}
