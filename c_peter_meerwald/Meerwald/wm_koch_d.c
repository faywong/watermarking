#include "wm.h"
#include "dct.h"
#include "signature.h"
#include "coord.h"
#include "pgm.h"

char *progname;

void usage(void) {
  fprintf(stderr, "usage: %s [-h] [-l n] [-o file] [-q n] [-v n] -s file file\n\n", progname);
  fprintf(stderr, "\t-h\t\tprint usage\n");
  fprintf(stderr, "\t-l n\t\tsignature robustness factor\n");
  fprintf(stderr, "\t-o file\t\textracted signature file\n");
  fprintf(stderr, "\t-q n\t\tquantization (JPEG quality) factor\n");
  fprintf(stderr, "\t-s file\t\toriginal signature file\n");
  fprintf(stderr, "\t-v n\t\tverbosity level\n");
  exit(0);
}

int main(int argc, char *argv[]) {

  FILE *in = stdin;
  FILE *out = stdout;
  FILE *sig = NULL;

  gray **image;
  struct coords *coords;

  char signature_name[MAXPATHLEN];
  char input_name[MAXPATHLEN] = "(stdin)";
  char output_name[MAXPATHLEN] = "(stdout)";

  int c;
  int n;

  int rows, cols, format;
  gray maxval;
  int row;

  int seed;
  int verbose = 0;

  double quality = 0.0;
  int quantization = 0;

  double **dcts;

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
      n_signature = NBITSTOBYTES(nbit_signature);
      if (quality == 0.0)
        fscanf(sig, "%lf\n", &quality);
      else
        fscanf(sig, "%*f\n");
      if (quantization == 0)
        fscanf(sig, "%d\n", &quantization);
      else
        fscanf(sig, "%*d\n");
      fscanf(sig, "%d\n", &seed);
      srandom(seed);
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

  pgm_readpgminit(in, &cols, &rows, &maxval, &format);

  if (cols % NJPEG) {
    fprintf(stderr, "%s: image width %d not a multiple of %d\n", progname, cols, NJPEG);
    exit(1);
  }

  if (rows % NJPEG) {
    fprintf(stderr, "%s: image height %d not a multiple of %d\n", progname, rows, NJPEG);
    exit(1);
  }

  if ((rows * cols) / (NJPEG * NJPEG) < nbit_signature) {
    fprintf(stderr, "%s: image too small to extract %d bits of signature\n", progname, nbit_signature);
    exit(1);
  }

  init_dct_8x8();
  init_quantum_JPEG_lumin(quantization);

  dcts = alloc_coeffs_8x8();

  if ((coords = alloc_coords(nbit_signature)) == NULL) {
    fprintf(stderr, "%s: unable to allocate memory\n", progname);
    exit(1);
  }

  image = pgm_allocarray(cols, rows);

  for (row = 0; row < rows; row++)
    pgm_readpgmrow(in, image[row], cols, maxval, format);

  fclose(in);

  n = 0;
  while (n < nbit_signature) {
    int xb;
    int yb;
    int c1, c2;
    double v1, v2;

    do {
      xb = random() % (cols / NJPEG);
      yb = random() % (rows / NJPEG);
    } while (add_coord(coords, xb, yb) < 0);

    fdct_block_8x8(image, xb * NJPEG, yb * NJPEG, dcts);

    do {
      c1 = (random() % (NJPEG * NJPEG - 2)) + 1;
      c2 = (random() % (NJPEG * NJPEG - 2)) + 1;
    } while (c1 == c2 || !is_middle_frequency_coeff_8x8(c1) || !is_middle_frequency_coeff_8x8(c2));

    quantize_8x8(dcts);

    if (verbose >= 1)
      fprintf(stderr, "%d: quantized DCT block (x %d/y %d), extracting (x %d/y %d), (x %d/y %d) ", n, xb * NJPEG, yb * NJPEG, c1 % NJPEG, c1 / NJPEG, c2 % NJPEG, c2 / NJPEG);

    v1 = dcts[c1 / NJPEG][c1 % NJPEG];
    v2 = dcts[c2 / NJPEG][c2 % NJPEG];

    if (fabs(v1) > fabs(v2)) {
      set_signature_bit(n, 1);
      if (verbose >= 1)
        fprintf(stderr, "HIGH\n");
    }
    else {
      set_signature_bit(n, 0);
      if (verbose >= 1)
        fprintf(stderr, "LOW\n");
    }

    if (verbose >= 2)
      print_coeffs_8x8(dcts);

    n++;
  }

  fprintf(out, "KCWM\n");
  fprintf(out, "%d\n", nbit_signature);
  fwrite(signature, sizeof(char), n_signature, out);
  fprintf(out, "\n");

  fclose(out);

  free_coeffs(dcts);

  pbm_freearray(image, rows);

  exit(0);
}
