#ifndef GRAY_H
#define GRAY_H

#include "wm.h"
#include "pgm.h"

gray **alloc_grays(int cols, int rows);
gray **alloc_grays_8x8();
void free_grays(gray **grays);
void copy_grays_to_block(gray ** block_grays, gray ** image_grays, int col, int row, int width, int height);
void copy_grays_from_block(gray ** image_grays, gray ** block_grays, int col, int row, int width, int height);
void print_grays(gray **grays, int col, int row, int width, int height);
void print_grays_8x8(gray **grays);
#endif
