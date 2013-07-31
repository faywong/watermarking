#define PI      3.1415926535897932
#define INVROOT2 0.7071067814

void open_image(FILE *in, int *width, int *height);
void load_image(int **im, FILE *in, int width, int height);
void save_image(int **im, FILE *out, int width, int height);
int ** imatrix(int nrows, int ncols);
void freematrix(int **I, int rows);
float ran0(long int *idum);
float gasdev(long int *idum);
void put_image_from_int_2_double(int **i, double *f, int N);
void put_image_from_double_2_int(double *f, int **i, int N);
void fct2d(double f[], int nrows, int ncols);
void ifct2d(double f[], int nrows, int ncols);
void matmul(double **a, double **b, double **r, int N);
void hartley(double **in, double **out, int N);
double ** dmatrix(int nrows, int ncols);
void freematrix_d(double **I, int rows);
void hartley(double **in, double **out, int N);
void matrix_i2d(int **i, double **d, int N);
void matrix_d2i(double **d, int **i, int N);
void put_matrix_2_vector(double **i, double *f, int N);
void put_vector_2_matrix(double *f, double **i, int N);
double * dvector(long int N);

void wm_init();   
void wm_init1();   
void wm_init2(); 
