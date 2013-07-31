#include "wm.h"
#include "dwt.h"

char filter_file[MAXPATHLEN] = "";
AllFilters dwt_allfilters;
FilterGH *dwt_filters = NULL;
int dwt_method;
int dwt_cols;
int dwt_rows;
int dwt_levels;
int dwt_filter;

void init_dwt(int cols, int rows, const char *filter_name, int filter, int level, int method) {
  int i;

  if (strcmp(filter_file, filter_name)) {
    if (filter_name)
      strcpy(filter_file, filter_name);
    else
      strcpy(filter_file, "filter.dat");

    /* memory leak here - there is no function unload_filters() */
    dwt_allfilters = load_filters(filter_file);

    if (!dwt_allfilters) {
      fprintf(stderr, "init_dwt(): unable to open filter definition file %s\n", filter_file);
      return;
    }
  }

#ifdef DEBUG
  if (level <= 0 || level > rint(log(MIN(cols, rows))/log(2.0)) - 2) {
    fprintf(stderr, "init_dwt(): level parameter does not match image width/height\n");
    return;
  }
#endif

  if (dwt_filters && level != dwt_levels) {
    free(dwt_filters);
    dwt_filters = NULL;
  }

  dwt_levels = level;

  if (!dwt_filters)
    dwt_filters = calloc(level + 1, sizeof(FilterGH));

  for (i = 0; i < level + 1; i++)
    dwt_filters[i] = (dwt_allfilters->filter)[filter];

  dwt_filter = filter;
  dwt_method = method;
  dwt_cols = cols;
  dwt_rows = rows;
}

Image_tree fdwt(gray **pixels) {
  Image image;
  Image_tree tree;
  int i, j;

  image = new_image(dwt_cols, dwt_rows);

  for (i = 0; i < dwt_rows; i++)
    for (j = 0; j < dwt_cols; j++)
      set_pixel(image, j, i, pixels[i][j]);

  tree = wavelettransform(image, dwt_levels, dwt_filters, dwt_method);
  free_image(image);

  return tree;
}

Image_tree fdwt_wp(gray **pixels) {
  Image image;
  Image_tree tree;
  int i, j;

  image = new_image(dwt_cols, dwt_rows);

  for (i = 0; i < dwt_rows; i++)
    for (j = 0; j < dwt_cols; j++)
      set_pixel(image, j, i, pixels[i][j]);

  tree = wavelettransform_wp(image, dwt_levels, dwt_filters, dwt_method);
  free_image(image);

  return tree;
}

void idwt(Image_tree dwts, gray **pixels) {
  Image image;
  int i, j;

  image = inv_transform(dwts, dwt_filters, dwt_method + 1);

  for (i = 0; i < dwt_rows; i++)
    for (j = 0; j < dwt_cols; j++)
      pixels[i][j] = PIXELRANGE((int) (get_pixel(image, j, i) + 0.5));

  free_image(image);
}

void idwt_wp(Image_tree dwts, gray **pixels) {
  Image image;
  int i, j;

  image = inv_transform(dwts, dwt_filters, dwt_method + 1);

  for (i = 0; i < dwt_rows; i++)
    for (j = 0; j < dwt_cols; j++)
      pixels[i][j] = PIXELRANGE((int) (get_pixel(image, j, i) + 0.5));

  free_image(image);
}

int gen_pollen_filter(double *filter, double alpha, double beta, int which) {
  int i, j, k, filterlength;
  double tf[6];

  /* parameter alpha, beta have to be in range -Pi .. Pi */
  if (alpha < -M_PI || alpha >= M_PI) {
    fprintf(stderr, "alpha %f out of range\n", alpha);
    return -1;
  }

  if (beta < -M_PI || beta >= M_PI) {
    fprintf(stderr, "beta %f out of range\n", beta);
    return -1;
  }

  /* generate Pollen filter coefficients, see http://www.dfw.net/~cody for details */
  tf[0] = ((1.0 + cos(alpha) + sin(alpha)) * (1.0 - cos(beta) - sin(beta)) + 2.0 * sin(beta) * cos(alpha)) / 4.0;
  tf[1] = ((1.0 - cos(alpha) + sin(alpha)) * (1.0 + cos(beta) - sin(beta)) - 2.0 * sin(beta) * cos(alpha)) / 4.0;
  tf[2] = (1.0 + cos(alpha - beta) + sin(alpha - beta)) / 2.0;
  tf[3] = (1.0 + cos(alpha - beta) - sin(alpha - beta)) / 2.0;
  tf[4] = 1.0 - tf[0] - tf[2];
  tf[5] = 1.0 - tf[1] - tf[3];

  /* set close-to-zero filter coefficients to zero */
  for (i = 0; i < 6; i++)
    if (fabs(tf[i]) <  1.0e-15) tf[i] = 0.0;

  /* find the first non-zero wavelet coefficient */
  i = 0;
  while (tf[i] == 0.0) i++;

  /* find the last non-zero wavelet coefficient */
  j = 5;
  while (tf[j] == 0.0) j--;

  filterlength = j - i + 1;
  for (k = 0; k < filterlength; k++)
    switch (which) {
        case FILTERH:
          filter[k] = tf[j--] / 2.0;
          break;
        case FILTERG:
          filter[k] = (double) (((i & 0x01) * 2) - 1) * tf[i] / 2.0;
          i++;
          break;
        case FILTERHi:
          filter[k] = tf[j--];
          break;
        case FILTERGi:
          filter[k] = (double) (((i & 0x01) * 2) - 1) * tf[i];
          i++;
          break;
        default:
          return -1;
    }

  while (k < 6)
    filter[k++] = 0.0;

  return filterlength;
}

void dwt_pollen_filter(double alpha, double beta) {
  FilterGH filter;  
  int i;

  filter = malloc(sizeof(struct FilterGHStruct));
#ifdef DEBUG
  if (!filter) {
    fprintf(stderr, "dwt_pollen_filter(): malloc failed()\n");
    return;
  }
#endif

  filter->type = FTOther;
  filter->name = "pollen";

  filter->g = new_filter(6);
  filter->g->type = FTSymm;
  filter->g->hipass = 1;
  filter->g->len = gen_pollen_filter(filter->g->data, alpha, beta, FILTERG);
  filter->g->start = -filter->g->len / 2;
  filter->g->end = filter->g->len / 2 - 1;

  filter->h = new_filter(6);
  filter->h->type = FTSymm;
  filter->h->hipass = 0;
  filter->h->len = gen_pollen_filter(filter->h->data, alpha, beta, FILTERH);
  filter->h->start = -filter->h->len / 2;
  filter->h->end = filter->h->len / 2 - 1;

  filter->gi = new_filter(6);
  filter->gi->type = FTSymm;
  filter->gi->hipass = 1;
  filter->gi->len = gen_pollen_filter(filter->gi->data, alpha, beta, FILTERGi);
  filter->gi->start = -filter->gi->len / 2;
  filter->gi->end = filter->gi->len / 2 - 1;
        
  filter->hi = new_filter(6);
  filter->hi->type = FTSymm;
  filter->hi->hipass = 0;
  filter->hi->len = gen_pollen_filter(filter->hi->data, alpha, beta, FILTERHi);
  filter->hi->start = -filter->hi->len / 2;
  filter->hi->end = filter->hi->len / 2 - 1;
  
#ifdef DEBUG
  if (dwt_levels <= 0) {
    fprintf(stderr, "dwt_pollen_filter(): level invalid - set to zero\n");
    return;
  }
#endif

#ifdef DEBUG
  if (!dwt_filters) {
    fprintf(stderr, "dwt_pollen_filter(): wm_dwt not initialized, call init_dwt() first\n");
    return;
  }
#endif

  for (i = 0; i < dwt_levels + 1; i++)
    dwt_filters[i] = filter;
}

int gen_param_filter(double *filter, int n, double alpha[], int which) {
  int i, j, k, filterlength;
  double *tf, *t;

  tf = malloc(2 * (n + 1) * sizeof(double));
  t = malloc(2 * (n + 1) * sizeof(double));
  if (!tf) {
    fprintf(stderr, "gen_param_filter(): malloc() failed\n");
    return -1;
  }

  tf[0] = 1.0 / sqrt(2.0);
  tf[1] = 1.0 / sqrt(2.0);

  for (k = 0; k < n; k++) {
    for (i = 0; i < 2 * (k + 2); i++) {

#define H(X) (((X) < 0 || (X) >= 2 * (k + 1)) ? 0.0 : tf[X])

       t[i] = 0.5 * (H(i - 2) + H(i) +
                     cos(alpha[k]) * (H(i - 2) - H(i)) +
                     (i & 1 ? -1.0 : 1.0) * sin(alpha[k]) * (H(2 * (k + 2) - i - 1) - H(2 * (k + 2) - i - 3)));
    }
    for (i = 0; i < 2 * (k + 2); i++) tf[i] = t[i];
  }

  /* set close-to-zero filter coefficients to zero */
  for (i = 0; i < 2 * (n + 1) ; i++)
    if (fabs(tf[i]) <  1.0e-15) tf[i] = 0.0;

  /* find the first non-zero wavelet coefficient */
  i = 0;
  while (tf[i] == 0.0) i++;
        
  /* find the last non-zero wavelet coefficient */
  j = 2 * (n + 1) - 1;
  while (tf[j] == 0.0) j--;

  filterlength = j - i + 1;
  for (k = 0; k < filterlength; k++)
    switch (which) {
        case FILTERG:
        case FILTERGi:
          filter[k] = (double) ((((i+1) & 0x01) * 2) - 1) * tf[i];
          i++;
          break;
        case FILTERH:
        case FILTERHi:
          filter[k] = tf[j--];
          break;
        default: 
          return -1;
    }

  while (k < 2 * (n + 1)) 
    filter[k++] = 0.0;

  return filterlength;
}

void dwt_param_filter(double alpha[], int param_len[]) {
  FilterGH filter;  
  int i;
  int param_len_sum = 0;

#ifdef DEBUG
  if (dwt_levels <= 0) {
    fprintf(stderr, "dwt_param_filter(): level invalid - set to zero\n");
    return;
  }
#endif

#ifdef DEBUG
  if (!dwt_filters) {
    fprintf(stderr, "dwt_param_filter(): wm_dwt not initialized, call init_dwt() first\n");
    return;
  }
#endif


  for (i = 0; i < dwt_levels + 1; i++) {

    filter = malloc(sizeof(struct FilterGHStruct));
#ifdef DEBUG
    if (!filter) {
      fprintf(stderr, "dwt_param_filter(): malloc failed()\n");
      return;
    }
#endif

    filter->type = FTOrtho;
    filter->name = "param";

    filter->g = new_filter(2 * (param_len[i] + 1));
    filter->g->type = FTSymm;
    filter->g->hipass = 1;
    filter->g->len = gen_param_filter(filter->g->data,
				      param_len[i], &alpha[param_len_sum],
				      FILTERG);
    filter->g->start = -filter->g->len / 2;
    filter->g->end = filter->g->len / 2 - 1;

    filter->h = new_filter(2 * (param_len[i] + 1));
    filter->h->type = FTSymm;
    filter->h->hipass = 0;
    filter->h->len = gen_param_filter(filter->h->data,
				      param_len[i], &alpha[param_len_sum],
				      FILTERH);
    filter->h->start = -filter->h->len / 2;
    filter->h->end = filter->h->len / 2 - 1;

    filter->gi = 0;
    filter->hi = 0;

    dwt_filters[i] = filter;

    param_len_sum += param_len[i];
  }
}

void done_dwt() {
}
