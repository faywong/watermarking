#include "wm.h"
#include "dct.h"
#include "signature.h"
#include "coord.h"
#include "gray.h"
#include "pgm.h"

char *progname;

double sign(double x) {
    if (x >= 0.0) return 1.0;
    else return -1.0;
}

double try_modif(gray **image_block, double **dcts, int c1, int c2, double w1, double w2) {
    int i, j;
    gray **altered_block;
    double **altered_dcts;
    double sum;

    altered_block = alloc_grays_8x8();
    altered_dcts = alloc_coeffs_8x8();

    for (i = 0; i < 8; i++) {
        memcpy(altered_dcts[i], dcts[i], sizeof(double) * 8);
    }
        
    // put the changed coefficients back to black
    altered_dcts[c1 / NJPEG][c1 % NJPEG] = w1;
    altered_dcts[c2 / NJPEG][c2 % NJPEG] = w2;

    dequantize_8x8(altered_dcts);

    idct_block_8x8(altered_dcts, altered_block, 0, 0);
    
    // compute MSE
    sum = 0.0;
    for (i = 0; i < 8; i++) {
        for (j = 0; j < 8; j++) {  
            double ib = image_block[i][j];
            double ab = altered_block[i][j];
            sum += (ib - ab) * (ib - ab);
        }
    }
    sum /= 64.0;
    
    free(altered_block);
    free(altered_dcts);
    
    return sum;
}

void usage(void) {
  fprintf(stderr, "usage: %s [-h] [-l n] [-o file] [-q n] [-v n] -s file file\n", progname);
  fprintf(stderr, "\t-h\t\tprint usage\n");
  fprintf(stderr, "\t-l n\t\tsignature robustness factor\n");
  fprintf(stderr, "\t-o file\t\toutput (watermarked) file\n");
  fprintf(stderr, "\t-q n\t\tquantization (JPEG quality) factor\n");
  fprintf(stderr, "\t-s file\t\tsignature to embed in input image\n");
  fprintf(stderr, "\t-v n\t\tverbosity level\n");
  exit(0);
}

int main(int argc, char *argv[]) {

  FILE *in = stdin;
  FILE *out = stdout;
  FILE *sig = NULL;

  char signature_name[MAXPATHLEN];
  char input_name[MAXPATHLEN] = "(stdin)";
  char output_name[MAXPATHLEN] = "(stdout)";

  int c;
  int n;

  int seed;
  int verbose = 0;

  int rows, cols, format;
  gray maxval;
  int row;

  int quantization = 0;
  double quality = 0.0;

  struct coords *coords;

  gray **image;
  double **dcts;
  gray **image_block;

  progname = argv[0]; 

  pgm_init(&argc, argv); wm_init();

  while ((c = getopt(argc, argv, "h?i:l:o:q:s:v:")) != EOF) {
    switch (c) {
      case 'h':
      case '?':
        usage();
        break;
      case 'l':
        quality = atof(optarg);
        if (quality <= 0.0) {
          fprintf(stderr, "%s: signature strength factor %f out of range\n", progname, quality);
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
        quantization = atoi(optarg);
        if (quantization <= 0 || quantization > 100) {
          fprintf(stderr, "%s: quantization factor %d out of range\n", progname, quantization);
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
    char line[128];
    fgets(line, sizeof(line), sig);
    if (strspn(line, "KCSG") >= 4) {
      fscanf(sig, "%d\n", &nbit_signature);
      if (quality == 0.0)
        fscanf(sig, "%lf\n", &quality);
      else
        fscanf(sig, "%*f\n");
      if (quantization == 0)
        fscanf(sig, "%d\n", &quantization);
      else
        fscanf(sig, "%*d\n");
      fscanf(sig, "%d\n", &seed);
      n_signature = NBITSTOBYTES(nbit_signature);
      fread(signature, sizeof(char), n_signature, sig);
      fscanf(sig, "\n");
      srandom(seed);
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

  if (cols % NJPEG) {
    fprintf(stderr, "%s: image width %d not a multiple of %d\n", progname, cols, NJPEG);
    exit(1);
  }

  if (rows % NJPEG) {
    fprintf(stderr, "%s: image height %d not a multiple of %d\n", progname, rows, NJPEG);
    exit(1);
  }

  if ((cols * rows) / (NJPEG * NJPEG) < nbit_signature) {
    fprintf(stderr, "%s: image not large enough to embed %d bits of signature\n", progname, nbit_signature);
    exit(1);
  }

  init_dct_8x8();
  init_quantum_JPEG_lumin(quantization);

  dcts = alloc_coeffs_8x8();
  image_block = alloc_grays_8x8();

  if ((coords = alloc_coords(nbit_signature)) == NULL) {
    fprintf(stderr, "%s: unable to allocate memory\n", progname);
    exit(1);
  }

  image = pgm_allocarray(cols, rows);

  for (row = 0; row < rows; row++)
    pgm_readpgmrow(in, image[row], cols, maxval, format);

  fclose(in);

  // embedding signature bits by modifying two coefficient relationship,
  // one bit for each block
  n = 0;
  while (n < nbit_signature) {
    int xb;
    int yb;
    int c1, c2;
    double v1, v2;
    double w1, w2;
    double best_w1, best_w2;
    double diff;
    double mod;
    double try;
    double best_mse;
    int no_mse_opt = 0;

    // randomly select a block, check to get distinct blocks
    // (don't watermark a block twice)
    do {
      xb = random() % (cols / NJPEG);
      yb = random() % (rows / NJPEG);
    } while (add_coord(coords, xb, yb) < 0);

    // do the forward 8x8 DCT of that block
    fdct_block_8x8(image, xb * NJPEG, yb * NJPEG, dcts);

    copy_grays_to_block(image_block, image, xb*NJPEG, yb*NJPEG, NJPEG, NJPEG);

    // randomly select two distinct coefficients from block
    // only accept coefficients in the middle frequency range
    do {
      c1 = (random() % (NJPEG * NJPEG - 2)) + 1;
      c2 = (random() % (NJPEG * NJPEG - 2)) + 1;
    } while (c1 == c2 || !is_middle_frequency_coeff_8x8(c1) || !is_middle_frequency_coeff_8x8(c2));

    // quantize block according to quantization quality parameter
    quantize_8x8(dcts);

    if (verbose > 0)
      fprintf(stderr, "%d: quantized DCT block (x %d/y %d), modifying (x %d/y %d), (x %d/y %d) for %s\n", n, xb * NJPEG, yb * NJPEG, c1 % NJPEG, c1 / NJPEG, c2 % NJPEG, c2 / NJPEG, get_signature_bit(n) ? "HIGH" : "LOW");
    if (verbose > 5)
      print_coeffs_8x8(dcts);

    v1 = dcts[c1 / NJPEG][c1 % NJPEG];
    v2 = dcts[c2 / NJPEG][c2 % NJPEG];

    best_w1 = DBL_MAX, best_w2 = DBL_MAX;
    try = 0.0;
    best_mse = DBL_MAX;

    diff = fabs(v1) - fabs(v2);

    if (get_signature_bit(n)) 
      mod = fabs(quality - ( fabs(v1) - fabs(v2) ));
    else
      mod = fabs(quality -  (fabs(v2) - fabs(v1)));
    
    if (verbose > 2)
        fprintf(stderr, "%d / %d: %.2f %.2f %.2f | %d\n", xb, yb, diff, v1, v2, get_signature_bit(n));    
    
    while (try <= mod) {
        w1 = v1;
        w2 = v2;

        // modify coefficient's relationship to embed signature bit
        // using mean square error to minimize error
        if (get_signature_bit(n)) {
          if (diff < quality) {
            // we have to impose the relationship, does not occur naturally
            w1 = sign(v1)*(fabs(v1) + mod - try);
            w2 = sign(v2)*(fabs(v2) - try);
          }
        }
        else {
          if (diff > -quality) {
            // force the relationship
            w2 = sign(v2)*(fabs(v2) + mod - try);
            w1 = sign(v1)*(fabs(v1) - try);
          }
        }

        double mse = try_modif(image_block, dcts, c1, c2, w1, w2);
        if (mse < best_mse) {
            best_w1 = w1;
            best_w2 = w2;
            best_mse = mse;
        }
        
        if (verbose > 2)
            fprintf(stderr, "%d / %d: MSE %.2f %.2f; %.2f: %.2f %.2f\n", xb, yb, mse, best_mse, try, w1, w2);
        
        if (fabs(mse) == 1e-3)
            break;
        
        if (fabs(fabs(w1) - fabs(w2) + quality) > 1e-3)
            break;

        if (no_mse_opt)
            break;

        try += 0.05;
    }

    if (verbose > 1)
      fprintf(stderr, "  %f -> %f, %f -> %f\n", v1, best_w1, v2, best_w2);

    // put the changed coefficients back to black
    dcts[c1 / NJPEG][c1 % NJPEG] = best_w1;
    dcts[c2 / NJPEG][c2 % NJPEG] = best_w2;

    // the obvious :-)
    dequantize_8x8(dcts);

    // do the inverse DCT on the modified 8x8 block
    idct_block_8x8(dcts, image, xb * NJPEG, yb * NJPEG);

    n++;
  }

  free_grays(image_block);
  free_coeffs(dcts);

  pgm_writepgminit(out, cols, rows, maxval, 0);
  for (row = 0; row < rows; row++)
    pgm_writepgmrow(out, image[row], cols, maxval, 0);

  fclose(out);

  pgm_freearray(image, rows);

  exit(0);
}
