#ifndef BRUYN_COMMON_H
#define BRUYN_COMMON_H

#include "pgm.h"

// for block type classification
#define BLOCKTYPE_UNKNOWN 0
#define BLOCKTYPE_HARD 1
#define BLOCKTYPE_PROGRESSIVE 2
#define BLOCKTYPE_NOISE 3

// thresholds
#define THRESHOLD_NOISE 10.0
#define THRESHOLD_SLOPE 5.0
#define THRESHOLD_NOISE_USAGE "10.0"
#define THRESHOLD_SLOPE_USAGE "5.0"

// zone classification
#define ZONE_VOID 0
#define ZONE_1 1
#define ZONE_2 2

// category classification
#define CATEGORY_VOID 0
#define CATEGORY_A 4
#define CATEGORY_B 8

// classifiction = zone | category
#define CLASSIFICATION_1A (ZONE_1 | CATEGORY_A)
#define CLASSIFICATION_1B (ZONE_1 | CATEGORY_B)
#define CLASSIFICATION_2A (ZONE_2 | CATEGORY_A)
#define CLASSIFICATION_2B (ZONE_2 | CATEGORY_B)
#define CLASSIFICATION_A CATEGORY_A
#define CLASSIFICATION_B CATEGORY_B

#define NPATTERN 3
#define NPATTERN_USAGE "3"

gray lookup_pattern(int pattern, int c, int r);

#endif /* BRUYN_COMMON_H */
