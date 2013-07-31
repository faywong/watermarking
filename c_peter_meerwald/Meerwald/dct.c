#include "wm.h"
#include "dct.h"

#define INVROOT2 0.7071067814
#define SWAP(A, B) {double t = A; A = B; B = t;}

int N;
int M;

double *dct_NxN_tmp = NULL;
double *dct_NxN_costable = NULL;
int dct_NxN_log2N = 0;

static const unsigned int JPEG_lumin_quant_table[NJPEG][NJPEG] = {
    {16,  11,  10,  16,  24,  40,  51,  61},
    {12,  12,  14,  19,  26,  58,  60,  55},
    {14,  13,  16,  24,  40,  57,  69,  56},
    {14,  17,  22,  29,  51,  87,  80,  62},
    {18,  22,  37,  56,  68, 109, 103,  77},
    {24,  35,  55,  64,  81, 104, 113,  92},
    {49,  64,  78,  87, 103, 121, 120, 101},
    {72,  92,  95,  98, 112, 100, 103,  99}};

static const unsigned int JPEG_chromin_quant_table[NJPEG][NJPEG] = {
      {17,  18,  24,  47,  99,  99,  99,  99},
      {18,  21,  26,  66,  99,  99,  99,  99},
      {24,  26,  56,  99,  99,  99,  99,  99},
      {47,  66,  99,  99,  99,  99,  99,  99},
      {99,  99,  99,  99,  99,  99,  99,  99},
      {99,  99,  99,  99,  99,  99,  99,  99},
      {99,  99,  99,  99,  99,  99,  99,  99},
      {99,  99,  99,  99,  99,  99,  99,  99}};

static void initcosarray()
{
  int i,group,base,item,nitems,halfN;
  double factor;

  dct_NxN_log2N = -1;
  do{
    dct_NxN_log2N++;
    if ((1<<dct_NxN_log2N)>N){
      fprintf(stderr, "dct_NxN: %d not a power of 2\n", N);
      exit(1);
    }
  }while((1<<dct_NxN_log2N)<N);
  if (dct_NxN_costable) free(dct_NxN_costable);
  dct_NxN_costable = malloc(N * sizeof(double));
#ifdef DEBUG
  if(!dct_NxN_costable){
    fprintf(stderr, "Unable to allocate C array\n");
    exit(1);
  }
#endif
  halfN=N/2;
  for(i=0;i<=halfN-1;i++) dct_NxN_costable[halfN+i]=4*i+1;
  for(group=1;group<=dct_NxN_log2N-1;group++){
    base= 1<<(group-1);
    nitems=base;
    factor = 1.0*(1<<(dct_NxN_log2N-group));
    for(item=1; item<=nitems;item++) dct_NxN_costable[base+item-1]=factor*dct_NxN_costable[halfN+item-1];
  }

  for(i=1;i<=N-1;i++) dct_NxN_costable[i] = 1.0/(2.0*cos(dct_NxN_costable[i]*M_PI/(2.0*N)));
}

void init_dct_NxN(int width, int height) {
#ifdef DEBUG
  if (width != height || width <= 0) {
    fprintf(stderr, "init_dct_NxN(): dimensions out of range\n");
    exit(1);
  }
#endif

  if (dct_NxN_tmp && M != height)
    free(dct_NxN_tmp);

  N = width;
  M = height;

  dct_NxN_tmp = malloc(height * sizeof(double));
#ifdef DEBUG
  if (!dct_NxN_tmp) {
    fprintf(stderr, "init_dct_NxN(): failed to allocate memory\n");
    exit(1);
  }
#endif

  initcosarray();
}

static void bitrev(double *f, int len)
{
  int i,j,m;

  if (len<=2) return; /* No action necessary if n=1 or n=2 */
  j=1;
  for(i=1; i<=len; i++){
    if(i<j)
      SWAP(f[j-1], f[i-1]);
    m = len>>1;
    while(j>m){
      j=j-m;
      m=(m+1)>>1;
    }
    j=j+m;
  }
}

static void inv_sums(double *f)
{
  int stepsize,stage,curptr,nthreads,thread,step,nsteps;

  for(stage=1; stage <=dct_NxN_log2N-1; stage++){
    nthreads = 1<<(stage-1);
    stepsize = nthreads<<1;
    nsteps   = (1<<(dct_NxN_log2N-stage)) - 1;
    for(thread=1; thread<=nthreads; thread++){
      curptr=N-thread;
      for(step=1; step<=nsteps; step++){
        f[curptr] += f[curptr-stepsize];
        curptr -= stepsize;
      }
    }
  }
}

static void fwd_sums(double *f)
{
  int stepsize,stage,curptr,nthreads,thread,step,nsteps;

  for(stage=dct_NxN_log2N-1; stage >=1; stage--){
    nthreads = 1<<(stage-1);
    stepsize = nthreads<<1;
    nsteps   = (1<<(dct_NxN_log2N-stage)) - 1;
    for(thread=1; thread<=nthreads; thread++){
      curptr=nthreads +thread-1;
      for(step=1; step<=nsteps; step++){
        f[curptr] += f[curptr+stepsize];
        curptr += stepsize;
      }
    }
  }
}

static void scramble(double *f,int len){
  int i,ii1,ii2;

  bitrev(f,len);
  bitrev(&f[0], len>>1);
  bitrev(&f[len>>1], len>>1);
  ii1=len-1;
  ii2=len>>1;
  for(i=0; i<(len>>2); i++){
    SWAP(f[ii1], f[ii2]);
    ii1--;
    ii2++;
  }
}

static void unscramble(double *f,int len)
{
  int i,ii1,ii2;

  ii1 = len-1;
  ii2 = len>>1;
  for(i=0; i<(len>>2); i++){
    SWAP(f[ii1], f[ii2]);
    ii1--;
    ii2++;
  }
  bitrev(&f[0], len>>1);
  bitrev(&f[len>>1], len>>1);
  bitrev(f,len);
}

static void inv_butterflies(double *f)
{
  int stage,ii1,ii2,butterfly,ngroups,group,wingspan,increment,baseptr;
  double Cfac,T;

  for(stage=1; stage<=dct_NxN_log2N;stage++){
    ngroups=1<<(dct_NxN_log2N-stage);
    wingspan=1<<(stage-1);
    increment=wingspan<<1;
    for(butterfly=1; butterfly<=wingspan; butterfly++){
      Cfac = dct_NxN_costable[wingspan+butterfly-1];
      baseptr=0;
      for(group=1; group<=ngroups; group++){
        ii1=baseptr+butterfly-1;
        ii2=ii1+wingspan;
        T=Cfac * f[ii2];
        f[ii2]=f[ii1]-T;
        f[ii1]=f[ii1]+T;
        baseptr += increment;
      }
    }
  }
}

static void fwd_butterflies(double *f)
{
  int stage,ii1,ii2,butterfly,ngroups,group,wingspan,increment,baseptr;
  double Cfac,T;

  for(stage=dct_NxN_log2N; stage>=1;stage--){
    ngroups=1<<(dct_NxN_log2N-stage);
    wingspan=1<<(stage-1);
    increment=wingspan<<1;
    for(butterfly=1; butterfly<=wingspan; butterfly++){
      Cfac = dct_NxN_costable[wingspan+butterfly-1];
      baseptr=0;
      for(group=1; group<=ngroups; group++){
        ii1=baseptr+butterfly-1;
        ii2=ii1+wingspan;
        T= f[ii2];
        f[ii2]=Cfac *(f[ii1]-T);
        f[ii1]=f[ii1]+T;
        baseptr += increment;
      }
    }
  }
}

static void ifct_noscale(double *f)
{
  f[0] *= INVROOT2;
  inv_sums(f);
  bitrev(f,N);
  inv_butterflies(f);
  unscramble(f,N);
}

static void fct_noscale(double *f)
{
  scramble(f,N);
  fwd_butterflies(f);
  bitrev(f,N);
  fwd_sums(f);
  f[0] *= INVROOT2;
}

void fdct_NxN(gray **pixels, double **dcts) {
  int u,v;
  double two_over_sqrtncolsnrows = 2.0/sqrt((double) N*M);

  for (u=0; u < N; u++)
    for (v=0; v < M; v++)
      dcts[u][v] = ((int) pixels[u][v] - 128);

  for (u=0; u<=M-1; u++){
    fct_noscale(dcts[u]);
  }
  for (v=0; v<=N-1; v++){
    for (u=0; u<=M-1; u++){
       dct_NxN_tmp[u] = dcts[u][v];
    }
    fct_noscale(dct_NxN_tmp);
    for (u=0; u<=M-1; u++){
      dcts[u][v] = dct_NxN_tmp[u]*two_over_sqrtncolsnrows;
    }
  }
}

void idct_NxN(double **dcts, gray **pixels) {
 int u,v;
 double two_over_sqrtncolsnrows = 2.0/sqrt((double) N*M);

  double **tmp;

  tmp = alloc_coeffs(N, N);
  for (u=0;u<N;u++)
    for (v=0;v<M;v++)
      tmp[u][v] = dcts[u][v];

  for (u=0; u<=M-1; u++){
    ifct_noscale(tmp[u]);
  }
  for (v=0; v<=N-1; v++){
    for (u=0; u<=M-1; u++){
       dct_NxN_tmp[u] = tmp[u][v];
    }
    ifct_noscale(dct_NxN_tmp);
    for (u=0; u<=M-1; u++){
       tmp[u][v] = dct_NxN_tmp[u]*two_over_sqrtncolsnrows;
    }
  }

 for (u=0;u<N;u++)
    for (v=0;v<M;v++)
      pixels[u][v] = PIXELRANGE(tmp[u][v] + 128.5);
 free(tmp);
}

void fdct_inplace_NxN(double **coeffs) {
  int u,v;
  double two_over_sqrtncolsnrows = 2.0/sqrt((double) N*M);

  for (u=0; u<=M-1; u++)
    fct_noscale(coeffs[u]);

  for (v=0; v<=N-1; v++){
    for (u=0; u<=M-1; u++)
       dct_NxN_tmp[u] = coeffs[u][v];

    fct_noscale(dct_NxN_tmp);
    for (u=0; u<=M-1; u++)
      coeffs[u][v] = dct_NxN_tmp[u]*two_over_sqrtncolsnrows;
  }
}

void idct_inplace_NxN(double **coeffs) {
 int u,v;
 double two_over_sqrtncolsnrows = 2.0/sqrt((double) N*M);

  for (u=0; u<=M-1; u++)
    ifct_noscale(coeffs[u]);

  for (v=0; v<=N-1; v++) {
    for (u=0; u<=M-1; u++)
       dct_NxN_tmp[u] = coeffs[u][v];

    ifct_noscale(dct_NxN_tmp);
    for (u=0; u<=M-1; u++)
       coeffs[u][v] = dct_NxN_tmp[u]*two_over_sqrtncolsnrows;
  }

}

double **dct_NxM_costable_x = NULL;
double **dct_NxM_costable_y = NULL;

void init_dct_NxM(int cols, int rows) {
  int i, j;
  double cx = sqrt(2.0 / cols);
  double cy = sqrt(2.0 / rows);

#ifdef DEBUG
  if (cols <= 0 || rows <= 0) {
    fprintf(stderr, "init_dct_NxM(): dimensions out of range\n");
    exit(1);
  }
#endif

  if (dct_NxM_costable_x && N != cols) {
    free_coeffs(dct_NxM_costable_x);
    dct_NxM_costable_x = NULL;
  }

  if (dct_NxM_costable_y && M != rows) {
    free_coeffs(dct_NxM_costable_y);
    dct_NxM_costable_y = NULL;
  }

  if (!dct_NxM_costable_x)
    dct_NxM_costable_x = alloc_coeffs(cols, cols);
  if (!dct_NxM_costable_y)
    dct_NxM_costable_y = alloc_coeffs(rows, rows);

  N = cols;
  M = rows;

  for (i = 0; i < cols; i++) {
    for (j = 0; j < cols; j++) {
      dct_NxM_costable_x[i][j] = cx * cos((M_PI * ((2*i + 1) * j)) / (double) (2 * N));
    }
  }

  for (i = 0; i < rows; i++) {
    for (j = 0; j < rows; j++) {
      dct_NxM_costable_y[i][j] = cy * cos((M_PI * ((2*i + 1) * j)) / (double) (2 * M));
    }
  }
}

void fdct_NxM(gray **pixels, double **dcts) {
  int x, y;
  int i, j;
  double t;
  double cx0 = sqrt(1.0 / N);
  double cy0 = sqrt(1.0 / M);

  t = 0.0;
  for (x = 0; x < N; x++)
    for (y = 0; y < M; y++)
      t += ((int) pixels[y][x] - 128);
  dcts[0][0] = cx0 * cy0 * t;

  for (i = 1; i < N; i++) {
    t = 0.0;
    for (x = 0; x < N; x++) {
      double s = 0.0;
      for (y = 0; y < M; y++) {
        s += ((int) pixels[y][x] - 128);
      }
      t += s * dct_NxM_costable_x[x][i];
    }
    dcts[0][i] = cy0 * t;
  }

  for (j = 1; j < M; j++) {
    t = 0.0;
    for (y = 0; y < M; y++) {
      double s = 0.0;
      for (x = 0; x < N; x++) {
        s += ((int) pixels[y][x] - 128);
      }
      t += s * dct_NxM_costable_y[y][j];
    }
    dcts[j][0] = cx0 * t;
  }

  for (i = 1; i < N; i++) {
     for (j = 1; j < M; j++) {
      t = 0.0;
      for (x = 0; x < N; x++) {
        double s = 0;
        for (y = 0; y < M; y++) {
          s += ((int) pixels[y][x] - 128) * dct_NxM_costable_y[y][j];
        }
        t += s * dct_NxM_costable_x[x][i];
      }
      dcts[j][i] = t;
    }
  }
}

void idct_NxM(double **dcts, gray **pixels) {
  int x, y;
  int i, j;
  double cx0 = sqrt(1.0 / N);
  double cy0 = sqrt(1.0 / M);
  double t;

  for (x = 0; x < N; x++) {
    for (y = 0; y < M; y++) {

      t = cx0 * cy0 * dcts[0][0];

      for (i = 1; i < N; i++)
        t += cy0 * dcts[0][i] * dct_NxM_costable_x[x][i];

      for (j = 1; j < M; j++)
        t += cx0 * dcts[j][0] * dct_NxM_costable_y[y][j];

      for (i = 1; i < N; i++) {
        double s = 0.0;
        for (j = 1; j < M; j++) {
          s += dcts[j][i] * dct_NxM_costable_y[y][j];
        }
        t += s * dct_NxM_costable_x[x][i];
      }

      pixels[y][x] = PIXELRANGE((int) (t + 128.5));
    }
  }
}

double C[NJPEG][NJPEG];
double Ct[NJPEG][NJPEG];
int Quantum[NJPEG][NJPEG];

void init_quantum_8x8(int quality) {
  int i;
  int j;

  for (i = 0; i < NJPEG; i++)
    for ( j = 0 ; j < NJPEG ; j++ )
      Quantum[ i ][ j ] = 1 + ( ( 1 + i + j )  * quality );
}

void init_quantum_JPEG_lumin(int quality) {
  int i;
  int j;

  if (quality < 50)
    quality = 5000 / quality;
  else
    quality = 200 - quality * 2;

  for (i = 0; i < NJPEG; i++)
    for (j = 0 ; j < NJPEG ; j++)
      if (quality)
        Quantum[i][j] = (JPEG_lumin_quant_table[i][j] * quality + 50) / 100;
      else
        Quantum[i][j] = JPEG_lumin_quant_table[i][j];
}

void init_quantum_JPEG_chromin(int quality) {
  int i;
  int j;

  if (quality < 50)
    quality = 5000 / quality;
  else
    quality = 200 - quality * 2;

  for (i = 0; i < NJPEG; i++)
    for (j = 0 ; j < NJPEG ; j++)
      if (quality)
        Quantum[i][j] = (JPEG_lumin_quant_table[i][j] * quality + 50) / 100;
      else
        Quantum[i][j] = JPEG_lumin_quant_table[i][j];
}

void quantize_8x8(double **transform) {
  int i;
  int j;

  for (i = 0; i < NJPEG; i++)
    for (j = 0; j < NJPEG; j++)
      transform[i][j] = ROUND(transform[i][j] / Quantum[i][j]);
}

void dequantize_8x8(double **transform) {
  int i;
  int j;

  for (i = 0; i < NJPEG; i++)
    for (j = 0; j < NJPEG; j++)
      transform[i][j] = ROUND(transform[i][j] * Quantum[i][j]);
}

void init_dct_8x8() {
  int i;
  int j;
  double pi = atan( 1.0 ) * 4.0;

  for ( j = 0 ; j < NJPEG ; j++ ) {
    C[ 0 ][ j ] = 1.0 / sqrt( (double) NJPEG );
    Ct[ j ][ 0 ] = C[ 0 ][ j ];
  }

  for ( i = 1 ; i < NJPEG ; i++ )
    for ( j = 0 ; j < NJPEG ; j++ ) {
      C[ i ][ j ] = sqrt( 2.0 / NJPEG ) * cos( pi * ( 2 * j + 1 ) * i / ( 2.0 * NJPEG ) );
      Ct[ j ][ i ] = C[ i ][ j ];
    }
}

/*
 * The Forward DCT routine implements the matrix function:
 *
 *                     DCT = C * pixels * Ct
 */

void fdct_8x8(gray **input, double **output) {
    double temp[NJPEG][NJPEG];
    double temp1;
    int i;
    int j;
    int k;

/*  MatrixMultiply( temp, input, Ct ); */
    for ( i = 0 ; i < NJPEG ; i++ ) {
        for ( j = 0 ; j < NJPEG ; j++ ) {
            temp[ i ][ j ] = 0.0;
            for ( k = 0 ; k < NJPEG ; k++ )
                 temp[ i ][ j ] += ( (int) input[ i ][ k ] - 128 ) *
                                   Ct[ k ][ j ];
        }
    }

/*  MatrixMultiply( output, C, temp ); */
    for ( i = 0 ; i < NJPEG ; i++ ) {
        for ( j = 0 ; j < NJPEG ; j++ ) {
            temp1 = 0.0;
            for ( k = 0 ; k < NJPEG ; k++ )
                temp1 += C[ i ][ k ] * temp[ k ][ j ];
            output[ i ][ j ] = temp1;
        }
    }
}

void fdct_block_8x8(gray **input, int col, int row, double **output) {
  int i;
  gray *input_array[NJPEG];

  for (i = 0; i < NJPEG; i++)
    input_array[i] = &input[row + i][col];

  fdct_8x8(input_array, output);
}

/*
 * The Inverse DCT routine implements the matrix function:
 *
 *                     pixels = C * DCT * Ct
 */

void idct_8x8(double **input, gray **output) {
    double temp[ NJPEG ][ NJPEG ];
    double temp1;
    int i;
    int j;
    int k;

/*  MatrixMultiply( temp, input, C ); */
    for ( i = 0 ; i < NJPEG ; i++ ) {
        for ( j = 0 ; j < NJPEG ; j++ ) {
            temp[ i ][ j ] = 0.0;
            for ( k = 0 ; k < NJPEG ; k++ )
                temp[ i ][ j ] += input[ i ][ k ] * C[ k ][ j ];
        }
    }

/*  MatrixMultiply( output, Ct, temp ); */
    for ( i = 0 ; i < NJPEG ; i++ ) {
        for ( j = 0 ; j < NJPEG ; j++ ) {
            temp1 = 0.0;
            for ( k = 0 ; k < NJPEG ; k++ )
                temp1 += Ct[ i ][ k ] * temp[ k ][ j ];
            temp1 += 128.0;
            output[i][j] = PIXELRANGE(ROUND(temp1));
        }
    }
}

void idct_block_8x8(double **input, gray **output, int col, int row) {
  int i;
  gray *output_array[NJPEG];

  for (i = 0; i < NJPEG; i++)
    output_array[i] = &output[row + i][col];

  idct_8x8(input, output_array);
}

int is_middle_frequency_coeff_8x8(int coeff) {
  switch (coeff) {
    case 3:
    case 10:
    case 17:
    case 24:
      return 1;
    case 4:
    case 11:
    case 18:
    case 25:
    case 32:
      return 2;
    case 5:
    case 12:
    case 19:
    case 26:
    case 33:
    case 40:
      return 3;
    case 13:
    case 20:
    case 27:
    case 34:
    case 41:
      return 4;
    case 28:
    case 35:
      return 5;
    default:
      return 0;
  }
}
