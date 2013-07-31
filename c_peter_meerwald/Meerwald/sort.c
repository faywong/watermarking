#include "sort.h"

#define SWAP_GRAY(A, B) {gray t = A; A = B; B = t;}

/*
 * quicksort-alike, from the EMX library (emx/src/lib/misc/qsort.c)
 *   by Eberhard Mattes
 *
 *   sort() is not stable, the order of equal elements is not defined
 */

void _sort_grays(gray *l, gray *r) {
  gray *i;
  gray *j;
  gray *x;

redo:
  i = l;
  j = r;
  x = l + sizeof(gray) * (((r-l) / sizeof(gray)) / 2);

  do {
    while (i != x && *i < *x)
      i++;
    while (j != x && *j > *x)
      j--;
    if (i < j) {
      SWAP_GRAY(*i, *j);
      if (x == i)
        x = j;
      else if (x == j)
        x = i;
    }
    if (i <= j) {
      i++;
      if (j > l)
        j--;
    }
  } while (i <= j);

  if (j-l < r-i) {
    if (l < j)
      _sort_grays(l, j);
    if (i < r) {
      l = i;
      goto redo;
    }
  }
  else {
    if (i < r)
      _sort_grays(i, r);
    if (l < j) {
      r = j;
      goto redo;
    }
  }
}

void sort_grays(gray a[], int n) {
  if (n > 1)
    _sort_grays(&a[0], &a[n-1]);
}

/*
 * select_largest(), from the Numeric Recipes in C, Chapter 8, p. 344,
 *   see http://www.nr.com
 *
 *   returns in largest[0..m-1] the largest m elements of array[0..n-1]
 *   with largest[0] guaranteed to be the mth largest element; largest[] is
 *   not sorted; the array[] is not altered; this function should be used
 *   only when m << n
 */

void select_largest_grays(gray array[], int n, int m, gray largest[]) {
  int i, j, k;

  if (m <= 0 || m > n/2)
    return;

  for (i = 0; i < m; i++)
    largest[i] = array[i];
  sort_grays(largest, m);

  for (i = m; i < n; i++) {
    if (array[i] > largest[0]) {
      largest[0] = array[i];
      j = 0;
      k = 1;
      while (k < m) {
        if (k < m-1 && largest[k] > largest[k+1])
          k++;
        if (largest[j] <= largest[k])
          break;
        SWAP_GRAY(largest[k], largest[j]);
        j = k;
        k = k << 1;
      }
    }
  }
}

#define SWAP_DOUBLE(A, B) {double t = A; A = B; B = t;}

void _sort_coeffs(double *l, double *r) {
  double *i;
  double *j;
  double *x;

redo:
  i = l;
  j = r;
  x = l + sizeof(double) * (((r-l) / sizeof(double)) / 2);

  do {
    while (i != x && *i < *x)
      i++;
    while (j != x && *j > *x)
      j--;
    if (i < j) {
      SWAP_DOUBLE(*i, *j);
      if (x == i)
        x = j;
      else if (x == j)
        x = i;
    }
    if (i <= j) {
      i++;
      if (j > l)
        j--;
    }
  } while (i <= j);

  if (j-l < r-i) {
    if (l < j)
      _sort_coeffs(l, j);
    if (i < r) {
      l = i;
      goto redo;
    }
  }
  else {
    if (i < r)
      _sort_coeffs(i, r);
    if (l < j) {
      r = j;
      goto redo;
    }
  }
}

void sort_coeffs(double a[], int n) {
  if (n > 1)
    _sort_coeffs(&a[0], &a[n-1]);
}

void select_largest_coeffs(double array[], int n, int m, double largest[]) {
  int i, j, k;

  if (m <= 0 || m > n/2)
    return;

  for (i = 0; i < m; i++)
    largest[i] = array[i];
  sort_coeffs(largest, m);

  for (i = m; i < n; i++) {
    if (array[i] > largest[0]) {
      largest[0] = array[i];
      j = 0;
      k = 1;
      while (k < m) {
        if (k < m-1 && largest[k] > largest[k+1])
          k++;
        if (largest[j] <= largest[k])
          break;
        SWAP_DOUBLE(largest[k], largest[j]);
        j = k;
        k = k << 1;
      }
    }
  }
}


