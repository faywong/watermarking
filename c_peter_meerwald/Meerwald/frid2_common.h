#ifndef FRID2_COMMON_H
#define FRID2_COMMON_H

#define FORWARD_STEP(A) ((1.0 + A) / (1.0 - A))
#define BACKWARD_STEP(A) ((1.0 - A) / (1.0 + A))

void embed_low_freq(double **dcts, int cols, int rows, double alpha, int verbose);
void embed_med_freq(double **dcts, int cols, int rows, double gamma, int seed, int verbose);
double detect_low_freq(double **dcts, int cols, int rows, double alpha, double beta, int verbose);
double detect_med_freq(double **dcts, int cols, int rows, int seed, int verbose);

#endif
