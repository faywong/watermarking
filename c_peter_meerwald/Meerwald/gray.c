#include "wm.h"
#include "gray.h"

gray **alloc_grays_8x8() {
  return alloc_grays(8, 8);
}

gray **alloc_grays(int cols, int rows) {
  gray **p;
  int i;

  p = (gray **)malloc(rows * sizeof(gray *));
  if (!p) {
#ifdef DEBUG
    fprintf(stderr, "alloc_grays(): malloc() failed\n");
    exit(1);
#else
    return NULL;
#endif
  }

  p[0] = (gray *)malloc(rows * cols * sizeof(gray));
  if (!p[0]) {
#ifdef DEBUG
    fprintf(stderr, "alloc_grays(): malloc() failed\n");
    exit(1);
#else
    free(p);
    return NULL;
#endif
  }

  for (i = 1; i < rows; i++) {
    p[i] = &(p[0][i * cols]);
  }

  return p;
}

void free_grays(gray **grays) {
  free(grays[0]);
  free(grays);
}

void copy_grays_to_block(gray ** block_grays, gray ** image_grays, int c, int r, int w, int h) {
  int i, j;

#ifdef DEBUG
  if (!image_grays) {
    fprintf(stderr, "copy_grays_to_block(): NULL image pixels\n");
  }
  if (!block_grays) {
    fprintf(stderr, "copy_grays_to_block(): NULL block pixels\n");
  }
  if (w <= 0 || h <= 0 || c < 0 || r < 0) {
    fprintf(stderr, "copy_grays_to_block(): block dimension out of range\n");
  }
#endif
  
  for (i = 0; i < w; i++) {
    for (j = 0; j < h; j++)
      block_grays[j][i] = image_grays[r + j][c + i];
  }
}

void copy_grays_from_block(gray ** image_grays, gray ** block_grays, int
c, int r, int w, int h) {
  int i, j;

#ifdef DEBUG
  if (!image_grays) {
    fprintf(stderr, "copy_grays_from_block(): NULL image pixels\n");
  }
  if (!block_grays) {
    fprintf(stderr, "copy_grays_from_block(): NULL block pixels\n");
  }
  if (w <= 0 || h <= 0 || c < 0 || r < 0) {
    fprintf(stderr, "copy_grays_from_block(): block dimension out of range\n");
  }
#endif
  
  for (i = 0; i < w; i++) {
    for (j = 0; j < h; j++)
      image_grays[r + j][c + i] = block_grays[j][i];
  }
}

void print_grays(gray **grays, int c, int r, int w, int h) {
  int i, j;
  gray *p;

#ifdef DEBUG
  if (!grays) {
    fprintf(stderr, "print_grays(): NULL pixels\n");
  }
  if (w <= 0 || h <= 0 || c < 0 || r < 0) {
    fprintf(stderr, "print_grays(): block dimension out of range\n");
  }
#endif

  for (j = r; j < r + h; j++) {
    p = &grays[j][c];
    for (i = 0; i < w; i++)
      fprintf(stderr, "%3d ", *(p++));
    fprintf(stderr, "\n");
  }
}

void print_grays_8x8(gray **grays) {
  int i, j;

  for (i = 0; i < 8; i++) {
    for (j = 0; j < 8; j++)
      fprintf(stderr, "%3d ", grays[i][j]);
    fprintf(stderr, "\n");
  }
}
