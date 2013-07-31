#include "bruyn_common.h"

gray lookup_pattern(int pattern, int c, int r) {
#define A CATEGORY_A
#define B CATEGORY_B

  gray pattern1[4][4] =
    {{A, A, B, B},
     {A, A, B, B},
     {B, B, A, A},
     {B, B, A, A}};

  gray pattern2[8][8] =
    {{B, B, B, B, A, A, A, A},
     {B, B, B, B, A, A, A, A},
     {B, B, B, B, A, A, A, A},
     {B, B, B, B, A, A, A, A},
     {A, A, A, A, B, B, B, B},
     {A, A, A, A, B, B, B, B},
     {A, A, A, A, B, B, B, B},
     {A, A, A, A, B, B, B, B}};

  gray pattern3[2][2] =
    {{A, B}, {B, A}};

#undef A
#undef B

  switch (pattern) {
    case 1:
      return pattern1[r % 4][c % 4];
      break;
    case 2:
      return pattern2[r % 8][c % 8];
      break;
    case 3:
      return pattern3[r % 2][c % 2];
      break;
  }

  return CATEGORY_VOID;
}


