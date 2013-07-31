#include <stdlib.h>
#include <stdio.h>
#include "coord.h"

struct coords *alloc_coords(int n) {
  struct coords *c;

  if ((c = malloc(sizeof(struct coords))) != NULL)
    init_coords(c, n);
#ifdef DEBUG
  else
    fprintf(stderr, "alloc_coords(): malloc failed\n");
#endif

  return c;
}

void free_coords(struct coords *c) {

#ifdef DEBUG
  if (!c)
    fprintf(stderr, "free_coords(): got NULL pointer\n");
#endif

  free(c->values);
  free(c);
}

int init_coords(struct coords *c, int n) {

#ifdef DEBUG
  if (!c)
    fprintf(stderr, "init_coords(): got NULL poiner\n");

  if (n <= 0)
    fprintf(stderr, "init_coords(): n out of range\n");
#endif

  c->count = 0;
  c->max = n;

  if ((c->values = malloc(n * sizeof(struct coord))) != NULL)
    return 0;
  else
    return -1;
}

int add_coord(struct coords *c, int x, int y) {
  struct coord *v;
  int n;

#ifdef DEBUG
  if (!c)
    fprintf(stderr, "add_coord(): got NULL pointer\n");

  if (c->count >= c->max)
    fprintf(stderr, "add_coord(): maximum reached\n");
#endif

  v = c->values;

  for (n = 0; n < c->count; v++, n++)
    if (v->x == x && v->y == y) break;

  if (n == c->count) {
    v->x = x;
    v->y = y;
    c->count++;
    return 0;
  }
  else
    return -1;
}


