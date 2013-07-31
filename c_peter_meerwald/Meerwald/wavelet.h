#ifndef WAVELET_H

#include <stdio.h>

extern char dbgstr[1000];

/* this are internal functions - don't use 'em! */
void out_dbg_str(const char *str);
void start_trace(void);
void stop_trace(void);
void flush_trace_file(void);

/* public functions / macros */
#define StartTrace
#define StopTrace

#define Trace(str)
#define TraceVar(str,var)

#define Entering
#define Leaving
#define LeavingErr
#define FlushTrace

#define Warning(str)

#define PreCondition(exp,str)
#define PostCondition(exp,str)

/* Note that if an error is added, an errormessage for this specific
   error must also be added. Otherwise no appropriate message can
   be displayed in an error window. ( Then "Unknown error ocurred"
   will be displayed.)
   The errormessage must be added to the case-construct in the 
   procedure err_GetErrorMessage
*/   

typedef enum
{
        Error_NoError,          /* No Error has happened. */
        Error_NotImplemented,   /* A needed part has not (yet) been 
                                   implemented */
        Error_AssertionFailed,  /* An assertion, pre- or postcondition failed. 
                                   Occurs only in buggy programs. */
        Error_NotEnoughMemory,   /* We can't allocate the memory we need. */

        Error_Limitation,       /* Some limitation exceeded, e.g. a string
                                   variable is too short */


	Error_CantOpenFile,	/* The file cannot be opened */
	Error_CantCreateFile,
        Error_CantWriteIntoFile,
	Error_CantCloseFile,
	Error_WrongFileFormat,

	Error_WidthOrHeightZero,
	Error_CompressedZeroContent,
	Error_OriginalZeroContent,

	Error_InternalError

}Error;	
	

/************************************************************************/
/*      Functionname:           err_simple_message                      */
/* -------------------------------------------------------------------- */
/*      Parameter:                                                      */
/*          char *: string that contains information about an           */
/*                  error the user should know.                         */
/* -------------------------------------------------------------------- */
/*      Description:                                                    */
/*          Prints error messages for the user.                         */
/************************************************************************/
void err_SimpleMessage(char *message);

/************************************************************************/
/*      Functionname:           err_get_message                         */
/* -------------------------------------------------------------------- */
/*      Return value:   Errormessage for this specific error.           */
/*      Parameter:                                                      */
/*          Error err:  Error whose errormessage should be returned     */
/* -------------------------------------------------------------------- */
/*      Description:                                                    */
/************************************************************************/
char * err_GetErrorMessage(Error err);

#include <stddef.h>

typedef double Pixel;

typedef struct Image_struct {
	Pixel *data;
	int width,height;

	/* redundant, for our fun only :-) */
	Pixel min_val,max_val;   /* range of pixel-values in data */
                                 /* [min_val..max_val] */
	int size;   /* = width * height */
	int bpp;    /* bits per pixel of original image */
	} *Image;

typedef unsigned int IntPixel;

typedef struct IntImage_struct {
	IntPixel *data;
	int width, height;

	/* redundant, for our fun only :-) */
	IntPixel min_val,max_val;   /* range of values in data */
                                    /* [min_val..max_val] */
	int size;   /* = width * height */
	int bpp;    /* bits per pixel of original image */
	} *IntImage;

typedef struct Image_tree_struct {
	double entropy;
	struct Image_tree_struct *coarse,*horizontal,*vertical,*diagonal,*doubletree;
	Image image;
	int level;
	int flag;

	void *codec_data;
	IntImage significance_map;
	} *Image_tree;

typedef struct Image_info_struct {
	Pixel min,max,mean,var,rms;
	} *Image_info;

enum zigzag_direction {zigzag_up,zigzag_down,zigzag_right,zigzag_left};

typedef struct Zigzag_data_struct {
	int x,y,w,h;
	enum zigzag_direction dir;
	} *Zigzag_data;

#define get_intpixel(image,x,y) ( ((image)==NULL || \
	(x)<0 || (x)>=(image)->width || (y)<0 || (y)>=(image)->height) \
	? (IntPixel) 0 : (image)->data[(x)+(y)*(image)->width])

#define set_intpixel(image,x,y,val) if (!((image)==NULL || \
	(x)<0 || (x)>=(image)->width || (y)<0 || (y)>=(image)->height)) \
	(image)->data[(x)+(y)*(image)->width]=(IntPixel) (val)

#define get_pixel(image,x,y) ( ((image)==NULL || \
	(x)<0 || (x)>=(image)->width || (y)<0 || (y)>=(image)->height) \
	? (Pixel) 0 : (image)->data[(x)+(y)*(image)->width])

#define set_pixel(image,x,y,val) if (!((image)==NULL || \
	(x)<0 || (x)>=(image)->width || (y)<0 || (y)>=(image)->height)) \
	(image)->data[(x)+(y)*(image)->width]=(Pixel) (val)

#define get_pixel_adr(image,x,y) ( ((image)==NULL || \
	(x)<0 || (x)>=(image)->width || (y)<0 || (y)>=(image)->height) \
	? (Pixel*) NULL : (image)->data+((x)+(y)*(image)->width))

/* functions: */

IntImage new_intimage(int width, int height);
IntImage load_intimage(char *file, int max_val);
void free_intimage(IntImage img);

void clear_intimage(IntImage img);
void copy_into_intimage(IntImage img1,IntImage img2,int x,int y);
void copy_part_of_intimage(IntImage img1,IntImage img2,int x,int y);

Image new_image(int width, int height);
void free_image(Image img);
void clear_image(Image img);
void copy_into_image(Image img1,Image img2,int x,int y);
void scale_image(Image img,int maximum);
void copy_part_of_image(Image img1,Image img2,int x,int y);

void copy_part_of_image_into_image(
				Image dest_img, int dest_x, int dest_y,
				Image src_img, int src_x, int src_y,
				int width, int height);


int string_to_pixel(char *str, Pixel *p);

Image load_image(char *file, int max_val);
int save_image_P5(char *file, Image img);

Image intimage_to_image(IntImage i);
IntImage image_to_intimage(Image i);

Image_tree new_image_tree();
void free_image_tree(Image_tree t);

Image get_difference_image(Image image1, Image image2);

void get_image_infos(Image image, Image_info info);

void get_intimage_infos(IntImage image, IntPixel *min, IntPixel *max, Pixel *avg, Pixel *var);

void init_zigzag(Zigzag_data zz, int width, int height);
void next_zigzag(Zigzag_data zz);
Image get_absolute_image_scaled(Image img);

/* common macros */

#ifndef MIN
#define MIN(a,b) ((a)<(b)?(a):(b))
#endif

#ifndef MAX
#define MAX(a,b) ((a)>(b)?(a):(b))
#endif

enum FilterType { FTNoSymm, FTSymm, FTAntiSymm};

typedef struct FilterStruct {
	enum FilterType type;
	int hipass;
	Pixel * data;
	int start,end;

	int len;
	} *Filter;

Filter new_filter(int size);

int filter_cutoff(Image in, int in_start, int in_len, int in_step,
		Image out, int out_start, int out_len, int out_step,
		Filter f);

int filter_inv_cutoff(Image in, int in_start, int in_len, int in_step,
		Image out, int out_start, int out_len, int out_step,
		Filter f);

int filter_periodical(Image in, int in_start, int in_len, int in_step,
		Image out, int out_start, int out_len, int out_step,
		Filter f);

int filter_inv_periodical(Image in, int in_start, int in_len, int in_step,
		Image out, int out_start, int out_len, int out_step,
		Filter f);

int filter_mirror(Image in, int in_start, int in_len, int in_step,
		Image out, int out_start, int out_len, int out_step,
		Filter f);

int filter_inv_mirror(Image in, int in_start, int in_len, int in_step,
		Image out, int out_start, int out_len, int out_step,
		Filter f);

Pixel get_filter_center(Filter f);

enum FilterGHType { FTOrtho, FTBiOrtho, FTOther};

typedef struct FilterGHStruct {
	enum FilterGHType type;
	Filter g, h, gi, hi;
	char *name;
	} *FilterGH;

typedef struct AllFilterStruct {
	FilterGH *filter;
	int count;
	} *AllFilters;


AllFilters load_filters(char *name);

typedef struct SegmentsStruct	{
		int width,height; /* segment width & height*/
		int *data;
	} *Segments;
	
enum FilterMethod{cutoff,inv_cutoff,periodical,inv_periodical,mirror,inv_mirror};

enum Information_Cost{threshold,log_energy,entropy,norml,norml2,gauss_markov,
		shanon,weak_l,weak_lq,compression_number,compression_numberq,
		compression_area,compression_areaq,sdiscrepancy,discrepancy,concentration};

Image_tree wavelettransform(Image original,int level,FilterGH *flt,enum FilterMethod method);
Image_tree wavelettransform_wp(Image original,int level,FilterGH *flt,enum FilterMethod method);
                            
Image_tree best_basis(Image original,int level,FilterGH *flt,
				enum FilterMethod method,enum Information_Cost cost,double epsilon);
				                             
Image_tree best_level(Image original,int maxlevel,int *bestlevel,FilterGH *flt,enum FilterMethod method,
				enum Information_Cost cost,double epsilon);

Image build_image(Image_tree quadtree,int width,int height);

Image inv_transform(Image_tree quadtree,FilterGH *flt,
                                 enum FilterMethod method);

Image inv_transform_wp(Image_tree quadtree,FilterGH *flt,
                                 enum FilterMethod method);

int rec_double(Image_tree dtree,int level,FilterGH *flt,enum FilterMethod method,enum Information_Cost cost,double epsilon);

Image_tree decompose_to_level(Image original,int level,FilterGH *flt,enum FilterMethod method);

int decompose_all(Image_tree tree,int maxlevel,FilterGH *flt,enum FilterMethod method,
				enum Information_Cost cost,double epsilon);

int find_deepest_level(int width,int height);


#define WAVELET_H
#endif
