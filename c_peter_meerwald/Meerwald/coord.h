#ifndef COORD_H
#define COORD_H

struct coord {
  int x;
  int y;
};

struct coords {
  int count;
  int max;
  struct coord *values;
};

struct coords *alloc_coords(int n);
void free_coords(struct coords *c);
int init_coords(struct coords *c, int n);
int add_coord(struct coords *c, int x, int y);

#endif
