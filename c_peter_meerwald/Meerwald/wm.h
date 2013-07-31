#ifndef WM_H
#define WM_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <getopt.h>
#include <math.h>
#include <float.h>
#include <fcntl.h>

#if defined(MINGW)
#define M_PI 3.1415926536
#define rint floor
#define MAXPATHLEN 255
void bzero(char *b, size_t length);
#elif defined(LINUX)
#include <values.h>
#include <sys/param.h>
#include <unistd.h>
#include <time.h>
#else
#error plattform not supported
#endif

/*
 * This macro is used to ensure correct rounding of integer values.
 */
#define ROUND(a) (((a) < 0) ? (int) ((a) - 0.5) : (int) ((a) + 0.5))

/*
 * Macros to converts number of bytes to number of bits and vice verse
 */
#define NBITSTOBYTES(N) ((N & 7) ? (N >> 3) + 1 : N >> 3)
#define NBYTESTOBITS(N) (N << 3)

#define GRAYRANGE(P) ((P > 255) ? 255 : (P < 0) ? 0 : P)
#define PIXELRANGE(P) ((P > 255) ? 255 : (P < 0) ? 0 : P)

#ifndef sqr
#define sqr(X) ((X) * (X))
#endif

#ifndef MAX
#define MAX(X, Y) (((X) > (Y)) ? (X) : (Y))
#endif

#ifndef MIN
#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))
#endif

#ifdef NEED_STRCASECMP
#define strcasecmp stricmp
#endif

#ifndef SIGN
#define SIGN(X) (((X) > 0) ? ((X) == 0 ? 0 : 1) : -1)
#endif

void wm_init();
void wm_init1();
void wm_init2();

#endif
