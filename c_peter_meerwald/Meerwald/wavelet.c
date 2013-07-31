#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "wavelet.h"
#include <ctype.h>
#include <values.h>

static int read_char(FILE *fp);
static int read_int(FILE *fp);

IntImage new_intimage(int width, int height)
{
	IntImage image;

	image = (IntImage) calloc(1,sizeof(struct IntImage_struct));
	if (image==NULL) goto error;
	image->data = (IntPixel*) calloc(width*height,sizeof(IntPixel));
	if (image->data==NULL) goto error;
	image->width = width;
	image->height = height;
	image->size = width*height;
	image->bpp = 0;
	image->min_val = (IntPixel) 0;
	image->max_val = (IntPixel) 0;

	return image;

	error:
	err_SimpleMessage(err_GetErrorMessage(Error_NotEnoughMemory));	
	return NULL;
}

Image new_image(int width, int height)
{
	Image image;

	image = (Image) calloc(1,sizeof(struct Image_struct));
	if (image==NULL) goto error;
	image->data = (Pixel*) calloc(width*height,sizeof(Pixel));
	if (image->data==NULL) goto error;
	image->width = width;
	image->height = height;
	image->size = width*height;
	image->bpp = 0;
	image->min_val = (Pixel) 0;
	image->max_val = (Pixel) 0;

	return image;

	error:
	err_SimpleMessage(err_GetErrorMessage(Error_NotEnoughMemory));	
	return NULL;
}


void free_intimage(IntImage img)
{
	if (img) {
          if (img->data) free(img->data);
          free(img);
	}
}

void free_image(Image img)
{
	if (img) {
          if (img->data) free(img->data);
          free(img);
	}
}

/************************************************************************
 *	Functionname:		load_intimage
 * --------------------------------------------------------------------
 *	PARAMETER:
 *	file:		filename of image
 *	max_val:	scaling of grey values to [0..max_val]
 *
 *	RETURN:
 *	Image that shoud be loaded, if not possible return NULL
 * --------------------------------------------------------------------
 * 	DESCRIPTION:
 *	This function loads an IntImage (PGM, ASCII or binary
 *	encoded (P5 or P3 format) ) from a file.
 ************************************************************************/

IntImage load_intimage(char *file, int max_val)
{	
	IntImage	img;
	FILE		*fp;
	IntPixel	*data;
	int		width, height, i, max, ch1, ch2;
		
	fp=fopen(file,"rb");
	if (fp==NULL) goto error;
	
	ch1=getc(fp);
	ch2=getc(fp);
	if (ch1!='P' || (ch2!='5' && ch2!='2')) goto error1;
	
	width=read_int(fp);
	height=read_int(fp);
	if ((width==0) || (height==0) ) goto error1;
	max=read_int(fp);

	img=new_intimage(width,height);

	img->bpp=8;
			
	data=img->data;
	for (i=0; i<img->size; i++)
	{ if (ch2=='5')
	    *data=getc(fp);
	  else
	    *data=read_int(fp);
	  data++;
	}	
	fclose(fp);
	return img;

        error1:
          err_SimpleMessage(err_GetErrorMessage(Error_WrongFileFormat));
          return NULL;
        error:
          err_SimpleMessage(err_GetErrorMessage(Error_CantOpenFile));
          return NULL;
}

/************************************************************************
 *	Functionname:		load_image
 * --------------------------------------------------------------------
 *	PARAMETER:
 *	file:		filename of image
 *	max_val:	scaling of grey values to [0..max_val]
 *
 *	RETURN:
 *	Image that shoud be loaded, if not possible return NULL
 * --------------------------------------------------------------------
 * 	DESCRIPTION:
 *	This function loads an IntImage with load_intimage
 *	and then converts to Image.
 ************************************************************************/
Image load_image(char *file, int max_val)
{
	Image img;
	IntImage intimg;

	intimg = load_intimage(file, max_val);
	if (!intimg) return NULL;
	img = intimage_to_image(intimg);
	if (!intimg) return NULL;

	return img;
}

/************************************************************************/
/*	Functionname:		save_image_P5                           */
/* -------------------------------------------------------------------- */
/*	Parameter:							*/
/*            img: Image that shoud be saved                            */
/*            file: filename of image                                   */
/* -------------------------------------------------------------------- */
/* 	Description: save an image as PGM (P5 binary decoded) file      */
/*	                                              			*/
/************************************************************************/

int save_image_P5(char *file, Image img)
{       FILE *fp;
        Pixel *data;
        long i;
	int p;
	
        fp=fopen(file,"wb");
        if (fp==NULL)
          goto error;
        fprintf(fp,"P5\n");
        fprintf(fp,"%d %d\n%d ",img->width,img->height,255);
        data=img->data;
        for (i=0;i<img->size;i++) {
	  p=floor(*data+0.5);
	  if (p<0) p=0;
	  if (p>255) p=255;
/*          putc(*data,fp);	*/
	  putc(p,fp);
          data++;
        }
        fclose(fp);
        return 1;

        error:
          err_SimpleMessage(err_GetErrorMessage(Error_CantOpenFile));
          return 0;
}

void clear_image(Image img)
{
	int i;

	PreCondition(img!=NULL,"image==NULL");

	for (i=0;i<img->size;i++)
		(img->data)[i]=(Pixel) 0;
}

void copy_into_image(Image img1,Image img2,int x,int y)
/* copy img2 into img1 at position (x,y)*/
{
        int start,i,j,aim;
        Pixel *temp;

        temp=img2->data;
        start=img1->width*y+x;
        for (i=0;i<img2->height;i++) {
          for (j=0;j<img2->width;j++) {
	    aim=start+j+img1->width*i;
            img1->data[aim]=*temp;
            temp++;
          }
        }
}

void copy_into_intimage(IntImage img1,IntImage img2,int x,int y)
{/* copy img2 into img1 at position (x,y)*/

        int start,i,j,aim;
        IntPixel *temp;

        temp=img2->data;
        start=img1->width*y+x;
        for (i=0;i<img2->height;i++)
        {
          for (j=0;j<img2->width;j++)
          {
	    aim=start+j+img1->width*i;
            img1->data[aim]=*temp;
            temp++;
          }
        }
}

void copy_part_of_image_into_image(Image dest_img, int dest_x, int dest_y,
				   Image src_img, int src_x, int src_y,
				   int width, int height)
{
	Pixel *sp,*dp;
	int y,siz;

	sp=get_pixel_adr(src_img,src_x,src_y);
	dp=get_pixel_adr(dest_img,dest_x,dest_y);

	siz=width*sizeof(Pixel);

	for (y=0;y<height;y++)
	{
		memcpy(dp,sp,siz);
		sp+=src_img->width;
		dp+=dest_img->width;
	}
}

void copy_part_of_image(Image img1,Image img2,int x,int y)
/* copy part of img2 begining at position (x,y) into img1 */
{	int i,j,width,height,start,step;
	Pixel *data;

	width=img1->width;
	height=img1->height;
	start=img2->width*y+x;
	data=img1->data;
	for (i=0;i<height;i++) {
	  step=i*img2->width;
	  for (j=0;j<width;j++){
	    *data=img2->data[start+j+step];
	    data++;
	  }
	}
}


void scale_image(Image img, int maximum)
/* scale image to [0..maximum]*/
{ 	int i;
	Pixel max = MINDOUBLE, min = MAXDOUBLE, multi;

	for (i=0;i<img->size;i++) {
  	  if (img->data[i]<min) min=img->data[i];
  	  else if (img->data[i]>max) max=img->data[i];
	}

	multi=(Pixel)maximum/(max-min);
	for (i=0;i<img->size;i++) img->data[i]=multi*(img->data[i]-min);
	img->min_val=0;
	img->max_val=maximum;
}

int string_to_pixel(char *str, Pixel *p)
{
	float ppp;
	if (sscanf(str,"%f",&ppp)==1)
	{
		*p=(Pixel) ppp;
		return 1;
	}
	else
	{
		*p=0.0;
		return 0;
	}
}

Image intimage_to_image(IntImage i)
{	Image img;
	int j; 	

	img=new_image(i->width,i->height);
	if (img==NULL) goto error;
	for (j=0;j<i->size;j++)
	  img->data[j]=(Pixel)i->data[j];
	img->bpp=i->bpp;
        return img;

       	error:
          err_SimpleMessage(err_GetErrorMessage(Error_NotEnoughMemory));	
	  return NULL;
}
IntImage image_to_intimage(Image i)
{       IntImage img;
        int j,multi=1,max,min;

        img=(IntImage)calloc(1,sizeof(struct IntImage_struct));
        if (img==NULL) goto error;
        img->data=(IntPixel *)calloc(i->size,sizeof(IntPixel));
        if (img->data==NULL) goto error;
        img->width=i->width;
        img->height=i->height;
        img->size=i->size;
        img->bpp=i->bpp;

        max=i->max_val;
        min=i->min_val;
        if ((max-min)!=0)
	  multi=255.0/(max-min);
	
        for (j=0;j<img->size;j++)
          img->data[j]=(int)((i->data[j]-min)*multi+0.5);
        return img;

        error:
          err_SimpleMessage(err_GetErrorMessage(Error_NotEnoughMemory));	
	  return NULL;
	  
}

static int read_char(FILE *fp)
/*read a character from file, but skip any comments*/
{       int ch;

	ch=getc(fp);
	if (ch=='#'){
	   do {
	     ch=getc(fp);
	   } while (ch!='\n' && ch!=EOF);
	}
	return ch;
}


static int read_int(FILE *fp)
/*read an ascii integer from file, and skip leading tabstops,new lines ...*/
{  	int r,ch;

	do {
	  ch=read_char(fp);
	} while (ch==' ' || ch=='\n' || ch=='\t');
	
	if (ch<'0' || ch>'9')
	 goto error;
	
	r=ch-'0';
	while ( (ch=read_char(fp)) >='0' && (ch <= '9') ) {
	  r*=10;
	  r+=ch-'0';
	}
	return r;
	error:
	  return 0;
}

Image_tree new_image_tree()
{
	Image_tree t;
	t=(Image_tree) calloc(1,sizeof(struct Image_tree_struct));
	t->entropy=0.0;
	t->image=NULL;
	t->coarse=t->horizontal=t->vertical=t->diagonal=t->doubletree=NULL;
	t->level=0;
	t->codec_data=NULL;
	t->significance_map=NULL;
	return t;
}

void free_image_tree(Image_tree t)
{
	if (t->coarse) free_image_tree(t->coarse);
	if (t->horizontal) free_image_tree(t->horizontal);
	if (t->vertical) free_image_tree(t->vertical);
	if (t->diagonal) free_image_tree(t->diagonal);
	if (t->doubletree) free_image_tree(t->doubletree);	
	if (t->image) free_image(t->image);
	if (t->significance_map) free_intimage(t->significance_map);
	if (t->codec_data) free(t->codec_data);
	t->image=NULL;
	free(t);
}

/***********************************************************************
 *      Functionname: get_image_infos                                   
 * -------------------------------------------------------------------- 
 *      Parameter:                                                      
 *          Image image: input image                                    
 *          Pixel *min,*max,*avg,*var: return minimum, maximum,         
 *                average and variance of current image                 
 * -------------------------------------------------------------------- 
 *      Description:                                                    
 *          get statistical information of Image                        
 ************************************************************************/

void get_image_infos(Image image, Image_info info)
{
	int x,y;
	Pixel p,sp,sp2;

	sp=sp2=(Pixel)0.0;

	p=get_pixel(image,0,0);

	info->min=info->max=p;

	for (y=0;y<image->height;y++)
		for (x=0;x<image->width;x++)
		{
			p=get_pixel(image,x,y);
			info->max=MAX(info->max,p);
			info->min=MIN(info->min,p);
			sp+=p;
			sp2+=p*p;
		}
	sp=sp/image->width/image->height;
	sp2=sp2/image->width/image->height;

	info->mean=sp;
	info->var=sp2-sp*sp;
	info->rms=sqrt(sp2);
}

/***********************************************************************
 *      Functionname: get_difference_image
 * -------------------------------------------------------------------- 
 *      Parameter:
 *          Image image1, image 2: input images
 *
 *      Return:
 *          Image : difference of image1 and image 2,
 *                  NULL if error occurs
 ************************************************************************/

Image get_difference_image(Image image1, Image image2)
{
	Image diff;
	int i,max,w,h;
	Pixel *d,*i1,*i2;

	if ((!image1) || (!image2)) return NULL;

	w=image1->width;
	h=image1->height;

	if (image2->width != w || image2->height != h) return NULL;

	diff=new_image(w,h);
	max=w*h;

	d=diff->data;
	i1=image1->data;
	i2=image2->data;

	for (i=0;i<max;i++)
		d[i]=i2[i]-i1[i];

	return diff;
}


/************************************************************************/
/*      Functionname: get_intimage_infos                                */
/* -------------------------------------------------------------------- */
/*      Parameter:                                                      */
/*          IntImage image: input image                                 */
/*          IntPixel *min,*max, return minimum, maximum			*/
/*	       Pixel *avg,*var: average and variance of current image   */
/*                average and variance of current image                 */
/* -------------------------------------------------------------------- */
/*      Description:                                                    */
/*          get statistical information of Image                        */
/************************************************************************/

void get_intimage_infos(IntImage image, IntPixel *min, IntPixel *max, Pixel *avg, Pixel *var)
{
	int x,y;
	Pixel p,sp,sp2;

	sp=sp2=(Pixel)0.0;

	p= (Pixel) get_intpixel(image,0,0);
	*min=*max=p;

	for (y=0;y<image->height;y++)
		for (x=0;x<image->width;x++)
		{
			p= (Pixel) get_intpixel(image,x,y);
			*max=MAX(*max, (IntPixel) p);
			*min=MIN(*min, (IntPixel) p);
			sp+=p;
			sp2+=p*p;
		}
	sp=sp/image->width/image->height;
	sp2=sp2/image->width/image->height;

	*avg=sp;
	*var=sp2-sp*sp;
}

/************************************************************************/
/*      Functionname: init_zigzag                                       */
/* -------------------------------------------------------------------- */
/*      Parameter:                                                      */
/*          Zigzag_data_struct:                                         */
/*              output: will be initialized, x/y hold coordinates of    */
/*                      the first pixel                                 */
/*          int width,height:                                           */
/*              input: width/height of image:                           */
/* -------------------------------------------------------------------- */
/*      Description:                                                    */
/*          initializes Zigzag_data structure for use with next_zigzag  */
/************************************************************************/

void init_zigzag(Zigzag_data zz, int width, int height)
{
	zz->x=0;
	zz->y=0;
	zz->dir=zigzag_up;
	zz->w=width;
	zz->h=height;
}

/************************************************************************/
/*      Functionname: next_zigzag                                       */
/* -------------------------------------------------------------------- */
/*      Parameter:                                                      */
/*          Zigzag_data_struct:                                         */
/*              int x,y:                                                */
/*                  input: current position of zigzag-scan              */
/*                  output: next position of zigzag-scan                */
/*              int w,h: width and height of image                      */
/*              enum zigzag_direction *dir: i/o:                        */
/*                  direction moving thru the image                     */
/* -------------------------------------------------------------------- */
/*      Description:                                                    */
/*          calculates the next point (x',y') of the zigzag-scan        */
/*          through the image with size (w,h)                           */
/************************************************************************/


void next_zigzag(Zigzag_data zz)
{
	switch(zz->dir)
	{
	case zigzag_up:
		if (zz->y==0)
		{
			if (zz->x==zz->w-1)
			{
				(zz->y)++; zz->dir=zigzag_down;
			}
			else
			{
				(zz->x)++; zz->dir=zigzag_down;
			}
		}
		else
		{
			if (zz->x==zz->w-1)
			{
				(zz->y)++; zz->dir=zigzag_down;
			}
			else
			{
				(zz->x)++; (zz->y)--;
			}
		}
		break;

	case zigzag_down:

		if (zz->x==0)
		{
			if (zz->y==zz->h-1)
			{
				(zz->x)++; zz->dir=zigzag_up;
			}
			else
			{
				(zz->y)++; zz->dir=zigzag_up;
			}
		}
		else
		{
			if (zz->y==zz->h-1)
			{
				(zz->x)++; zz->dir=zigzag_up;
			}
			else
			{
				(zz->x)--;(zz->y)++;
			}
		}
		break;
	}
}

Image get_absolute_image_scaled(Image img)
{
	Image out;

	int x,y;

	struct Image_info_struct info;
	Pixel scale,p;

	out=new_image(img->width,img->height);

	get_image_infos(img, &info);

	scale=255/MAX(fabs(info.min),fabs(info.max));

	for(y=0;y<img->height;y++)
	for(x=0;x<img->width;x++)
	{
		p=get_pixel(img,x,y)*scale;
		set_pixel(out,x,y,p);
	}
	return out;
}
	
#define FLOOR_HALF(x) ((x)&1 ? ((x)-1)/2 : (x)/2)
#define CEILING_HALF(x) ((x)&1 ? ((x)+1)/2 : (x)/2)

#define MOD(a,b) ( (a)<0 ? ((b)-((-(a))%(b))) : (a)%(b) )

Filter new_filter(int size)
{
	Filter f;

	Entering;
	f=(Filter) calloc(1,sizeof(struct FilterStruct));
	f->data=(Pixel *)calloc(size,sizeof(Pixel));
	f->len=size;
	f->hipass=0;

	Leaving;
	return f;
}

Pixel get_filter_center(Filter f)
{
	int i;
	Pixel p, sum, norm;

	if (f==NULL) return 0;

	sum=norm=0;

	for (i=0;i<f->len;i++)
	{
		p=f->data[i];
		p=p*p;
		norm += p;
		sum += (i+f->start)*p;
	}
	p=sum/norm;

	return p;
}
int filter_cutoff(Image in, int in_start, int in_len, int in_step,
		Image out, int out_start, int out_len, int out_step,
		Filter f)
{
	int i,i2,j;
	Pixel *out_pix, *in_pix, *f_data;
	int fstart,fend; /* Source interval */

	Entering;

	PreCondition(out_len == in_len/2,"out_len != in_len/2 !!!");

/* convolution: out[i]=sum_{j=start}^{end} (in[2*i-j]*f[j])

   boundaries:	image in [in_start ... in_start + in_len-1]
		image out [out_start ... out_start + out_len-1]
		filter f [0..f->len-1] = [f->start .. f->end]
		cutoff at: 
*/

	for (i=0;i<out_len;i++)
	{
		i2=2*i;

		fstart=i2-(in_len-1);
		fstart=MAX(fstart,f->start);
		fend=MIN(i2,f->end);

#ifdef TRACE
		sprintf(dbgstr,"i=%d fstart=%d fend=%d\n",i,fstart,fend);
		Trace(dbgstr);
#endif

		out_pix=out->data+out_start+i*out_step;

		in_pix=in->data+in_start+(i2-fstart)*in_step;

		f_data=f->data-f->start+fstart;

		for (j=fstart;j<=fend;j++,in_pix-=in_step,f_data++)
		{
			*out_pix += (*f_data) * (*in_pix);
#ifdef TRACE

			sprintf(dbgstr,"     j=%2d in: %4.2f filter: %4.2f [%4d] [%4d]  opt : %4.2f %4.2f\n",
				j,
				in->data[in_start+in_step*(i2-j)],
				f->data[j-f->start],
				in_start+in_step*(i2-j),
				j-f->start,
				*in_pix, *f_data);
			Trace(dbgstr);
#endif
		}
	}

	Leaving;
	return 1;
}


int filter_inv_cutoff(Image in, int in_start, int in_len, int in_step,
		Image out, int out_start, int out_len, int out_step,
		Filter f)
{
	int i,j;
	Pixel *out_pix, *in_pix, *f_data;
	int fstart,fend; /* Source interval */
	Entering;
	PreCondition(out_len == in_len*2,"out_len != in_len*2 !!!");

/* convolution: out[i]=sum_{j=start}^{end} (f[2*j-i]*in[j])

   boundaries:	image in [in_start ... in_start + in_len-1]
		image out [out_start ... out_start + out_len-1]
		filter f [0..f->len-1] = [f->start .. f->end]
		cutoff at: 
*/

	for (i=0;i<out_len;i++)
	{
		fstart=CEILING_HALF(f->start+i);
		fend=FLOOR_HALF(f->end+i);
		fstart=MAX(fstart,0);
		fend=MIN(fend,in_len-1);

#ifdef TRACE
		sprintf(dbgstr,"i=%d fstart=%d fend=%d\n",i,fstart,fend);
		Trace(dbgstr);
#endif
		out_pix=out->data+out_start+i*out_step;

		in_pix=in->data+in_start+fstart*in_step;

		f_data=f->data-f->start+2*fstart-i;

		for (j=fstart;j<=fend;j++,in_pix+=in_step,f_data+=2)
		{
			*out_pix += (*f_data) * (*in_pix);
#ifdef TRACE
			sprintf(dbgstr,"     j=%2d in: %4.2f filter: %4.2f [%4d] [%4d]  opt : %4.2f %4.2f\n",
				j,
				in->data[in_start+j*in_step],
				f->data[2*j-i-f->start],
				in_start+j*in_step,
				2*j-i-f->start,
				*in_pix, *f_data);
			Trace(dbgstr);
#endif
		}
	}

	Leaving;
	return 1;
}

int filter_periodical(Image in, int in_start, int in_len, int in_step,
		Image out, int out_start, int out_len, int out_step,
		Filter f)
{
	int i,i2,j;
	Pixel *out_pix, *in_pix, *f_data;
	int fstart,fend;
	int istart;
	int ipix_len;

	Entering;
	PreCondition(out_len == in_len/2,"out_len != in_len/2 !!!");

/* convolution: out[i]=sum_{j=start}^{end} (in[2*i-j]*f[j])

   boundaries:	image in [in_start ... in_start + in_len-1]
		image out [out_start ... out_start + out_len-1]
		filter f [0..f->len-1] = [f->start .. f->end]
*/

	ipix_len=in_len*in_step;

	for (i=0;i<out_len;i++)
	{
		i2=2*i;

		fstart=f->start;
		fend=f->end;

		istart=(i2-fstart);
		istart=MOD(istart,in_len);

#ifdef TRACE
		sprintf(dbgstr,"i=%d istart=%d\n",i,istart);
		Trace(dbgstr);
#endif

		out_pix=out->data+out_start+i*out_step;

		in_pix=in->data+in_start+istart*in_step;

		f_data=f->data;

		for (j=fstart;j<=fend;j++,f_data++)
		{
			*out_pix += (*f_data) * (*in_pix);
#ifdef TRACE

			sprintf(dbgstr,"     j=%2d in: %4.2f filter: %4.2f [%4d] [%4d]  opt : %4.2f %4.2f\n",
				j,
				in->data[in_start+in_step*((i2-j+in_len)%in_len)],
				f->data[j-f->start],
				in_start+in_step*((i2-j+in_len)%in_len),
				j-f->start,
				*in_pix, *f_data);
			Trace(dbgstr);
#endif
			in_pix-=in_step;
			istart--;
			if (istart<0)
			{
				istart+=in_len;
				in_pix+=ipix_len;
			}
		}
	}

	Leaving;
	return 1;
}

int filter_inv_periodical(Image in, int in_start, int in_len, int in_step,
		Image out, int out_start, int out_len, int out_step,
		Filter f)
{
	int i,j;
	Pixel *out_pix, *in_pix, *f_data;
	int fstart,fend; /* Source interval */
	int istart;
	int ipix_len;
	Entering;
	PreCondition(out_len == in_len*2,"out_len != in_len*2 !!!");

/* convolution: out[i]=sum_{j=start}^{end} (f[2*j-i]*in[j])

   boundaries:	image in [in_start ... in_start + in_len-1]
		image out [out_start ... out_start + out_len-1]
		filter f [0..f->len-1] = [f->start .. f->end]
*/

	ipix_len=in_len*in_step;

	for (i=0;i<out_len;i++)
	{
		fstart=CEILING_HALF(f->start+i);
		fend=FLOOR_HALF(f->end+i);

		istart=MOD(fstart,in_len);
#ifdef TRACE
		sprintf(dbgstr,"i=%d fstart=%d fend=%d istart=%d\n",i,fstart,fend,istart);
		Trace(dbgstr);
#endif
		out_pix=out->data+out_start+i*out_step;

		in_pix=in->data+in_start+istart*in_step;

		f_data=f->data-f->start+2*fstart-i;

		for (j=fstart;j<=fend;j++,f_data+=2)
		{
			*out_pix += (*f_data) * (*in_pix); 
#ifdef TRACE
			sprintf(dbgstr,"     j=%2d in: %4.2f filter: %4.2f [%4d] [%4d]  opt : %4.2f %4.2f\n",
				j,
				in->data[in_start+(j % in_len)*in_step],
				f->data[2*j-i-f->start],
				in_start+(j%in_len)*in_step,
				2*j-i-f->start,
				*in_pix, *f_data);
			Trace(dbgstr);
#endif
			in_pix+=in_step;
			istart++;
			if (istart>=in_len)
			{
				istart-=in_len;
				in_pix-=ipix_len;
			}
		}
	}

	Leaving;
	return 1;
}

int filter_mirror(Image in, int in_start, int in_len, int in_step,
		Image out, int out_start, int out_len, int out_step,
		Filter f)
{
	int i,i2,j;
	Pixel *out_pix, *in_pix, *f_data;
	int fstart,fend;
	int in_pos;

	Entering;
	PreCondition(out_len == in_len/2,"out_len != in_len/2 !!!");

/* convolution: out[i]=sum_{j=start}^{end} (in[2*i-j]*f[j])

   boundaries:	image in [in_start ... in_start + in_len-1]
		image out [out_start ... out_start + out_len-1]
		filter f [0..f->len-1] = [f->start .. f->end]
*/

	in_pix=in->data+in_start;

	for (i=0;i<out_len;i++)
	{
		i2=2*i;

		fstart=f->start;
		fend=f->end;

		out_pix=out->data+out_start+i*out_step;

		f_data=f->data;

		for (j=fstart;j<=fend;j++)
		{
			in_pos=(i2-j);
			if (in_pos<0)
			{
				in_pos=-in_pos;
				if (in_pos>=in_len) continue;
			}
			if (in_pos>=in_len)
			{
				in_pos=2*in_len-2-in_pos;
				if (in_pos<0) continue;
			}
			*out_pix += (f_data[j-fstart]) * (in_pix[in_pos*in_step]);
		}
	}

	Leaving;
	return 1;
}

int filter_inv_mirror(Image in, int in_start, int in_len, int in_step,
		Image out, int out_start, int out_len, int out_step,
		Filter f)
{
	int i,j;
	Pixel *out_pix, *in_pix;
	int fstart,fend; /* Source interval */
	int in_pos;

	Entering;
	PreCondition(out_len == in_len*2,"out_len != in_len*2 !!!");

/* convolution: out[i]=sum_{j=start}^{end} (f[2*j-i]*in[j])

   boundaries:	image in [in_start ... in_start + in_len-1]
		image out [out_start ... out_start + out_len-1]
		filter f [0..f->len-1] = [f->start .. f->end]
*/

	/*fprintf(stderr,"inv started\n");*/
	for (i=0;i<out_len;i++)
	{
		fstart=CEILING_HALF(f->start+i);
		fend=FLOOR_HALF(f->end+i);

		out_pix=out->data+out_start+i*out_step;

		in_pix=in->data+in_start;
		
/*
		printf("in: %4d - %4d  flt: %4d - %4d   [%s]\n",fstart,fend,2*fstart-i,2*fend-i,
		(2*fstart-i<f->start || 2*fend-i>f->end) ? "error":"ok");
*/
		/*fprintf(stderr,"inv[%d]\n",i);*/
		for (j=fstart;j<=fend;j++)
		{
			in_pos=j;
			if (in_pos<0)
			{
				if (f->hipass)
					in_pos=-in_pos-1;
				else
					in_pos=-in_pos;
				if (in_pos>=in_len) continue;
			}
			if (in_pos>=in_len)
			{
				if (f->hipass)
					in_pos=2*in_len-2-in_pos;
				else
					in_pos=2*in_len-1-in_pos;
				if (in_pos<0) continue;
			}
			/*fprintf(stderr,"out+= %7.2f * %7.2f  = %7.2f\n",f->data[2*j-i-f->start],in_pix[in_pos*in_step],f->data[2*j-i-f->start]*in_pix[in_pos*in_step]);*/
			*out_pix += f->data[2*j-i-f->start] * (in_pix[in_pos*in_step]);
		}
	}

	Leaving;
	return 1;
}

#define MAX_LINE 256

#define skip_blank(str) { while(isspace(*(str))) (str)++; }

static int get_next_line(FILE *f, char *c)
{
        char *str,string[200];
        int len;
        do
        {
                str=string;
                if (!fgets(str,200,f))
		{
			Trace("get_next_line: eof\n");
			goto error;
		}
                len=strlen(str);
                while (len>=1 && isspace(str[len-1])) str[--len]=0;
                while (isspace(*str)) str++;
        }
        while (strlen(str)==0 || *str=='#');
        strcpy(c,str);
	return 1;
error:
	return 0;
}

static int next_line_str(FILE *f, char *tag, char *out)
{
	char str[MAX_LINE],*t_str;

	if (!get_next_line(f,str)) goto error;
	t_str=strtok(str," ");
	if (!t_str || strcmp(t_str,tag)) goto error;
	t_str=strtok(NULL,"\n");
	if (!t_str) goto error;
	skip_blank(t_str);

	strcpy(out,t_str);
	return 1;
error:
	return 0;
}

static int next_line_str_alloc(FILE *f, char *tag, char **out)
{
	char str[MAX_LINE];
	if (!next_line_str(f,tag,str)) goto error;

	*out=malloc(strlen(str)+1);
	strcpy(*out,str);

	return 1;
error:
	return 0;
}

static int next_line_int(FILE *f, char *tag, int *out)
{
	char str[MAX_LINE];
	if (next_line_str(f,tag,str) && sscanf(str,"%d",out)==1)
		return 1;
	else
		return 0;
}


static Filter read_filter(FILE *f)
{
	char str[MAX_LINE];
	Filter filter;
	int i;

	Entering;

	filter=calloc(1,sizeof(struct FilterStruct));

	if (!next_line_str(f,"Type",str)) goto error1;

	if (!strcmp(str,"nosymm"))
	{
		filter->type=FTNoSymm;
	}
	else if (!strcmp(str,"symm"))
	{
		filter->type=FTSymm;
	}
	else if (!strcmp(str,"antisymm"))
	{
		filter->type=FTAntiSymm;
	}
	else 
 		goto error1;

	if (!next_line_int(f,"Length",&(filter->len))) goto error1;
	if (!next_line_int(f,"Start",&(filter->start))) goto error1;
	if (!next_line_int(f,"End",&(filter->end))) goto error1;	

	if ((filter->end-filter->start+1!=filter->len))
	{
		Trace("error: len != end-start+1\n");
		goto error1;
	}

	filter->data=calloc(filter->len,sizeof(Pixel));

	for (i=0;i<filter->len;i++)
	{
		if (!get_next_line(f,str)) goto error2;
		if (!string_to_pixel(str,filter->data+i))
		{
			Trace("error: invalid filter-value\n");
			goto error2;
		}
	}
	if (!get_next_line(f,str)) goto error2;
	if (*str!='}')
	{
		Trace("error: '}' not found\n");
		goto error2;
	}

	Leaving;
	return filter;

error2:
	free(filter->data);

error1:
	free(filter);

	LeavingErr;
	return NULL;

}

static FilterGH read_filter_gh(FILE *f)
{
	char str[MAX_LINE];
	FilterGH fgh;
	Filter filter;
	int i,max;

	Entering;

	fgh=calloc(1,sizeof(struct FilterGHStruct));

	if (!next_line_str_alloc(f,"Name",&(fgh->name)))
	{
		Trace("error: 'Name' tag not found\n");
		goto error1;
	}

	if (!next_line_str(f,"Type",str))
	{
		Trace("error: 'Type' tag not found\n");
		goto error1;
	}

	if (!strcmp(str,"orthogonal"))
	{
		fgh->type=FTOrtho;
		max=2;
	}
	else if (!strcmp(str,"biorthogonal"))
	{
		fgh->type=FTBiOrtho;
		max=4;
	}
	else if (!strcmp(str,"other"))
	{
		fgh->type=FTOther;
		max=4;
	}
	else
	{
		Trace("error: expecting 'orthogonal', 'biorthogonal' or 'other' type-tag\n");
		goto error1;
	}

	for (i=0;i<max;i++)
	{
		if (!get_next_line(f,str)) goto error2;
		if (*str!='{')
		{
			Trace("error: '{' not found\n");
			goto error2;
		}
		if (!(filter=read_filter(f)))
		{
			Trace("error: read_filter failed\n");
			goto error2;
		}
		filter->hipass = !(i&1);
		switch(i)
		{
		case 0: fgh->g=filter; break;
		case 1: fgh->h=filter; break;
		case 2: fgh->gi=filter; break;
		case 3: fgh->hi=filter; break;
		}
	}
	if (!get_next_line(f,str)) goto error2;
	if (*str!='}')
	{
		Trace("error: '}' not found\n");
		goto error2;
	}

	Leaving;
	return fgh;

error2:
	if (fgh->g) free(fgh->g);
	if (fgh->h) free(fgh->h);
	if (fgh->gi) free(fgh->gi);
	if (fgh->hi) free(fgh->hi);

error1:
	free(fgh);

	LeavingErr;
	return NULL;
}


AllFilters load_filters(char *name)
{
	FILE *f;
	char str[MAX_LINE];
	AllFilters a;
	FilterGH fgh;

	Entering;

	PreCondition(name!=NULL,"name=NULL!");

	f=fopen(name,"rt");
	if (!f)
	{
		Trace("error: fopen failed\n");
		goto error1;
	}

	a=calloc(1,sizeof(struct AllFilterStruct));
	a->count=0;

	while (get_next_line(f,str))
	{
		if (*str=='{')
		{
			fgh=read_filter_gh(f);
			if (!fgh)
			{
				Trace("error: read_filter returned NULL\n");
				goto error2;
			}
			if (a->count)
				a->filter=realloc(a->filter,(a->count+1)*sizeof(FilterGH));
			else
				a->filter=malloc(sizeof(FilterGH));
			(a->filter)[a->count]=fgh;
			a->count++;
		}
		else
		{
			Trace("error: '{' not found\n");
			goto error2;
		}
	}

	fclose(f);

	Leaving;
	return a;

error2:
	fclose(f);
error1:

	LeavingErr;
	return 0;
}

#define doubletree_min 32
#define best_basis_min 8

static int convolute_lines(Image output,Image input,Filter flt,enum FilterMethod method);
static int convolute_rows(Image output,Image input,Filter flt,enum FilterMethod method);
static int decomposition(Image t_img,Image coarse,Image horizontal,Image vertical,
                               Image diagonal,Filter g,Filter h,enum FilterMethod method);
static int compute_best(Image_tree tree,int level,int max_level,FilterGH *flt,
                        enum FilterMethod method,enum Information_Cost cost,double epsilon);                           
static double compute_entropy(Image img,enum Information_Cost cost,double epsilon);
static void compute_levels(Image_tree tree,double *entropies,enum Information_Cost cost,double epsilon);
static void free_levels(Image_tree tree,int best);

static Pixel sumationq(Image img);
static Pixel normq(Image_tree tree);
static Pixel sumation_down(Image_tree tree, Pixel normq);
static Pixel compute_non_additive(Image_tree tree,int size,enum Information_Cost cost,double
epsilon,int down);

/************************************************************************/
/*	Functionname: wavelettransform					*/
/* -------------------------------------------------------------------- */
/*	Parameter: 							*/
/*		original: Image that should be transformed  		*/
/*	        level: transform down to level                 		*/
/*		flt: transform with filters flt[0..level]		*/
/*		method: method of filtering				*/
/* -------------------------------------------------------------------- */
/* 	Description: Carries out the wavelettransform 			*/
/*	                           		           		*/
/************************************************************************/
Image_tree wavelettransform(Image original,int level,FilterGH *flt,enum FilterMethod method)
{	int i,width,height,min,max_level,e;
        Image coarsei,horizontali,verticali,diagonali,tempi;
	Image_tree ret_tree,temp_tree;
			
	width=original->width;
	height=original->height;
	
	tempi=new_image(width,height);
	if(!tempi) goto error;
	
        copy_into_image(tempi,original,0,0);
        	
	ret_tree=new_image_tree();
	if(!ret_tree) goto error;
	
	temp_tree=ret_tree;
	ret_tree->level=0;

	min=original->width;
	if (original->height<min) min=original->height;
	max_level=log(min)/log(2)-2;
	if (max_level<level) level=max_level;

	if (level<1)  /* do not transform */
	{
		ret_tree->image=tempi;
		return ret_tree;
	}

	/* decomposition */
	
	for (i=0;i<level;i++)
	{

	        width=(width+1)/2;
		height=(height+1)/2;
	
		coarsei=new_image(width,height);
		horizontali=new_image(width,height);
		verticali=new_image(width,height);
		diagonali=new_image(width,height);
		if(!coarsei||!horizontali||!verticali||!diagonali) goto error;		

		e=decomposition(tempi,coarsei,horizontali,verticali,diagonali,
	                       flt[i]->g,flt[i]->h,method);
		if (!e) return NULL;	                       
	                       
		temp_tree->coarse=new_image_tree();
		temp_tree->horizontal=new_image_tree();
		temp_tree->vertical=new_image_tree();
		temp_tree->diagonal=new_image_tree();
		
		temp_tree->coarse->level=i+1;
		temp_tree->horizontal->level=i+1;		
		temp_tree->vertical->level=i+1;		
		temp_tree->diagonal->level=i+1;		
		
		temp_tree->horizontal->image=horizontali;
		temp_tree->vertical->image=verticali;
		temp_tree->diagonal->image=diagonali;
	  	free_image(tempi);
	    	  
		if (i!=(level-1))
		{
	    		tempi=new_image(width,height);
	    		copy_into_image(tempi,coarsei,0,0);
	    		free_image(coarsei);
	    		/*if i=level coarsei is inserted into the image tree
	    		  so we should not free coarsei on level-1*/
	 	}
	 	
		temp_tree=temp_tree->coarse;
		
  	}

 	temp_tree->image=coarsei;
	return ret_tree;

	error:
        	err_SimpleMessage(err_GetErrorMessage(Error_NotEnoughMemory));	
	  	return NULL;		
}

static Image_tree wavelettransform__wp(Image original, int current_level, int level, FilterGH *flt, enum FilterMethod method)
{	
  int i, width, height, min, max_level, e;
  Image coarse_image,horizontal_image,vertical_image,diagonal_image,temp_image;
  Image_tree return_tree, temp_tree;
			
  width = original->width;
  height = original->height;
	
  temp_image = new_image(width, height);
  if (!temp_image) goto error;
	
  copy_into_image(temp_image, original, 0, 0);
        	
  temp_tree = return_tree = new_image_tree();
  if (!return_tree) goto error;

  temp_tree->level = current_level;
	
  min = original->width;
  if (original->height < min) min = original->height;
  max_level = log(min) / log(2) - 2;
  if (max_level < level) level = max_level;

  if (current_level >= level) {
    return_tree->image = temp_image;
    return return_tree;
  }

  for (i = current_level; i < level; i++) {
    width = (width + 1) / 2;
    height = (height + 1) / 2;
	
    coarse_image = new_image(width, height);
    horizontal_image = new_image(width, height);
    vertical_image = new_image(width, height);
    diagonal_image = new_image(width, height);

    if (!coarse_image || !horizontal_image || 
        !vertical_image || !diagonal_image) goto error;

    e = decomposition(temp_image, coarse_image, horizontal_image, 
                                  vertical_image, diagonal_image,
	                          flt[i]->g, flt[i]->h, method);
    if (!e) return NULL;	                       
		                       
    temp_tree->coarse = new_image_tree();
    temp_tree->coarse->level = i+1;
    temp_tree->horizontal = wavelettransform__wp(horizontal_image, i+1, level, flt, method);
    temp_tree->vertical = wavelettransform__wp(vertical_image, i+1, level, flt, method);
    temp_tree->diagonal = wavelettransform__wp(diagonal_image, i+1, level, flt, method);
		
    free_image(horizontal_image);
    free_image(vertical_image);
    free_image(diagonal_image);
    free_image(temp_image);
	    	  
    if (i != (level - 1)) {
      temp_image = new_image(width, height);
      copy_into_image(temp_image, coarse_image, 0, 0);
      free_image(coarse_image);
    }
	 	
    temp_tree = temp_tree->coarse;
  }

 temp_tree->image = coarse_image;
 return return_tree;

  error:
    err_SimpleMessage(err_GetErrorMessage(Error_NotEnoughMemory));	
    return NULL;		
}

Image_tree wavelettransform_wp(Image original, int level, FilterGH *flt, enum FilterMethod method) {
  return wavelettransform__wp(original, 0, level, flt, method);
}


/************************************************************************/
/*	Functionname: best_basis					*/
/* -------------------------------------------------------------------- */
/*	Parameter:							*/
/*	    	original: Image to be transformed			*/
/*		level: best basis selection down to this level		*/
/*		flt: transform with filters flt[0..level]		*/
/*		method: transform with filter method			*/
/*		cost: carry best basis selection out with this costfunc */
/*		epsilon: limit for threshold method			*/
/* -------------------------------------------------------------------- */
/* 	Description: carries best basis and near best basis selection	*/
/*			out						*/
/************************************************************************/
Image_tree best_basis(Image original,int level,FilterGH *flt,
				enum FilterMethod method,enum Information_Cost cost,double epsilon)

{       Image_tree tree;
	Image img;
	int min,max_level,e;

	tree=new_image_tree();
	if(!tree) goto error;
	
	img=new_image(original->width,original->height);
	if(!img) goto error;
	
        copy_into_image(img,original,0,0);
        
        tree->image=img;	

	min=original->width;
	if (original->height<min) min=original->height;
	max_level=log10((float) min/best_basis_min)/log10(2);
	if (max_level>level) max_level=level;
	
	e=compute_best(tree,0,max_level,flt,method,cost,epsilon);

	if (!tree->image) free(img);

        return tree;

	error:
        	err_SimpleMessage(err_GetErrorMessage(Error_NotEnoughMemory));	
	  	return NULL;	        

}
/************************************************************************/
/*	Functionname: best_level_selection				*/
/* -------------------------------------------------------------------- */
/*	Parameter: 							*/
/*		original: Image that should be transformed  		*/
/*	        maxlevel: transform down to level                 	*/
/*		flt: transform with filters flt[0..level]		*/
/*		method: transform with filter method			*/
/*		cost: carry best basis selection out with this costfunc */
/*		epsilon: limit for threshold method			*/
/* -------------------------------------------------------------------- */
/* 	Description: Carries out the best level selection		*/
/*	                           		           		*/
/************************************************************************/
Image_tree best_level(Image original,int maxlevel,int *bestlevel,FilterGH *flt,enum FilterMethod method,
				enum Information_Cost cost,double epsilon)
{	Image_tree tree;
	Image img;
	double *entropies,min;
	int best=0,i,e;

	img=new_image(original->width,original->height);
       	copy_into_image(img,original,0,0);
	
	tree=new_image_tree();
	tree->image=img;
	
	entropies=(double *)calloc(maxlevel+1,sizeof(double));
	if(!entropies) goto error;

	/* decompose down to maxlevel */
	e=decompose_all(tree,maxlevel,flt,method,cost,epsilon);
	if (!e) return NULL;

	/* compute costs of each level and store it in entropies array*/
	compute_levels(tree,entropies,cost,epsilon);

	min=entropies[0];
	for (i=1;i<=maxlevel;i++)
	{
		if (entropies[i]<min)
		{
			min=entropies[i];
			best=i;
		}
	}

	/* free all other levels */
	free_levels(tree,best);

	*bestlevel=best;
	
	return tree;

	error:
        	err_SimpleMessage(err_GetErrorMessage(Error_NotEnoughMemory));	
	  	return NULL;	
}

/************************************************************************/
/*	Functionname: compute_best					*/
/* -------------------------------------------------------------------- */
/*	Parameter: 							*/
/*		img: Image that should be transformed	 		*/
/*	        level: transform level	                		*/
/*		max_level: transform to maximum level			*/
/*		flt: transform with filters flt[0..level]		*/
/*		method: transform with filter method			*/
/*		cost: carry best basis selection out with this costfunc */
/*		epsilon: limit for threshold method			*/
/* -------------------------------------------------------------------- */
/* 	Description: Carries out the best basis selection (recursivly)  */
/*	             (is needed by the waveletpacket procedure)		*/
/************************************************************************/
static int compute_best(Image_tree tree,int level,int max_level,FilterGH *flt,
                        enum FilterMethod method,enum Information_Cost cost,double epsilon)

{ 	Image coarse,horizontal,vertical,diagonal;
        int e,width,height;
	double sum;
	
	tree->level=level;
		
	/* non additive cost function*/
	if (cost>=shanon) 
	{
		tree->entropy=compute_non_additive(tree,tree->image->size,cost,epsilon,0);
	}
	/*additive cost function*/
	else 	tree->entropy=compute_entropy(tree->image,cost,epsilon);
	
	if (level<max_level) {
        	width=(tree->image->width+1)/2;
        	height=(tree->image->height+1)/2;
          
	  	tree->coarse=new_image_tree();
	  	tree->horizontal=new_image_tree();
		tree->vertical=new_image_tree();
	  	tree->diagonal=new_image_tree();

          	coarse=new_image(width,height);	
          	horizontal=new_image(width,height);	
          	vertical=new_image(width,height);
          	diagonal=new_image(width,height);
		if(!coarse||!vertical||!horizontal||!diagonal) goto error;          	

	        e=decomposition(tree->image,coarse,horizontal,vertical,diagonal,flt[0]->g,flt[0]->h,method);
	        if (!e) return 0;

	        tree->coarse->image=coarse;
	        tree->horizontal->image=horizontal;
	        tree->vertical->image=vertical;
	        tree->diagonal->image=diagonal;
	        
          	e=compute_best(tree->coarse,level+1,max_level,flt+1,method,cost,epsilon);
          	e=compute_best(tree->horizontal,level+1,max_level,flt+1,method,cost,epsilon);
          	e=compute_best(tree->vertical,level+1,max_level,flt+1,method,cost,epsilon);
          	e=compute_best(tree->diagonal,level+1,max_level,flt+1,method,cost,epsilon);
          	if (!e) return 0;
	  
		/*going back in recursion*/

		if (cost>=shanon) {
			 sum=compute_non_additive(tree,tree->image->size,cost,epsilon,1);
		}
		else sum=(tree->coarse->entropy)+(tree->horizontal->entropy)
			+(tree->vertical->entropy)+(tree->diagonal->entropy);
			
	  	if (tree->entropy>sum)
	  	{
	  		tree->entropy=sum;	     		
	      		free_image(tree->image);	/* take down tree */
	      		tree->image=NULL;
	      		
	  	}
	  	else
	  	{   				/* delete the tree downwards */
         		free_image_tree(tree->coarse);
              		free_image_tree(tree->horizontal);
              		free_image_tree(tree->vertical);
              		free_image_tree(tree->diagonal);                			
              			
              		tree->coarse=tree->vertical=tree->horizontal=tree->diagonal=NULL;	    		
	  	}
	}

	return 1;

	error:
        	err_SimpleMessage(err_GetErrorMessage(Error_NotEnoughMemory));
        	return 0;
	  		
}

/************************************************************************/
/*	Functionname:		decompose_all                         */
/* -------------------------------------------------------------------- */
/*	Parameter:							*/
/*            	tree: Image tree to be decomposed			*/
/*           	maxlevel: decompose down to level	                */
/*		flt: transform with filters flt[0..maxlevel]		*/
/*		method: transform with filter method			*/
/*		cost: cost function for entropy computing		*/
/*		epsilon: limit for threshold method			*/
/* -------------------------------------------------------------------- */
/* 	Description: whole decompositing down to maxlevel          	*/
/*		The original image must be in tree->image		*/
/************************************************************************/
int decompose_all(Image_tree tree,int maxlevel,FilterGH *flt,enum FilterMethod method,
				enum Information_Cost cost,double epsilon)
{	
	Image original,coarse,horizontal,vertical,diagonal;
	int e,width,height,level;
	
	if (tree->level<maxlevel)
	{
		tree->coarse=new_image_tree();
		tree->horizontal=new_image_tree();
		tree->vertical=new_image_tree();
		tree->diagonal=new_image_tree();

		original=tree->image;
		width=(original->width+1)/2;
		height=(original->height+1)/2;
		level=tree->level;
		
		coarse=new_image(width,height);
		horizontal=new_image(width,height);
		vertical=new_image(width,height);
		diagonal=new_image(width,height);
		if(!coarse||!vertical||!horizontal||!diagonal) goto error;          	
		
		
		e=decomposition(tree->image,coarse,horizontal,vertical,diagonal,flt[0]->g,flt[0]->h,method);
		if (!e) return 0;

		tree->coarse->image=coarse;
		tree->horizontal->image=horizontal;
		tree->vertical->image=vertical;
		tree->diagonal->image=diagonal;

		tree->coarse->entropy=compute_entropy(coarse,cost,epsilon);
		tree->horizontal->entropy=compute_entropy(horizontal,cost,epsilon);
		tree->vertical->entropy=compute_entropy(vertical,cost,epsilon);
		tree->diagonal->entropy=compute_entropy(diagonal,cost,epsilon);
		
		tree->coarse->level=tree->horizontal->level=
			tree->vertical->level=tree->diagonal->level=level+1;

		e=decompose_all(tree->coarse,maxlevel,flt+1,method,cost,epsilon);
		e=decompose_all(tree->horizontal,maxlevel,flt+1,method,cost,epsilon);
		e=decompose_all(tree->vertical,maxlevel,flt+1,method,cost,epsilon);
		e=decompose_all(tree->diagonal,maxlevel,flt+1,method,cost,epsilon);
		if (!e) return 0;
		
	}

	return 1;
	
	error:
        	err_SimpleMessage(err_GetErrorMessage(Error_NotEnoughMemory));	
		return 0;	  		
}

/************************************************************************/
/*	Functionname:		compute_levels                          */
/* -------------------------------------------------------------------- */
/*	Parameter:							*/
/*            	tree: Image tree where the entropy should be computed	*/
/*           	entropies : array for entropy          	                */
/*		cost: carry best basis selection out with this costfunc */
/*		epsilon: limit for threshold method			*/
/* -------------------------------------------------------------------- */
/* 	Description: Compute the entropies of all decomposition	levels	*/
/************************************************************************/
static void compute_levels(Image_tree tree,double *entropies,enum Information_Cost cost,double epsilon)
{	
	if (tree->image){
		entropies[tree->level]+=compute_entropy(tree->image,cost,epsilon);
	}
	if (tree->coarse) compute_levels(tree->coarse,entropies,cost,epsilon);
	if (tree->horizontal) compute_levels(tree->horizontal,entropies,cost,epsilon);
	if (tree->vertical) compute_levels(tree->vertical,entropies,cost,epsilon);
	if (tree->diagonal) compute_levels(tree->diagonal,entropies,cost,epsilon);

}

/************************************************************************/
/*	Functionname:		free_levels	                        */
/* -------------------------------------------------------------------- */
/*	Parameter:							*/
/*            	tree: Image tree which should be cleaned		*/
/*           	best: best level	         	                */
/* -------------------------------------------------------------------- */
/* 	Description: clean the image tree except the best level      	*/
/************************************************************************/
static void free_levels(Image_tree tree,int best)
{
	if (tree->level<best)
	{
		free_image(tree->image);
		tree->image=NULL;
		free_levels(tree->coarse,best);
		free_levels(tree->horizontal,best);
		free_levels(tree->vertical,best);
		free_levels(tree->diagonal,best);
	}
	else
	{
		if (tree->coarse)
		{
			free_image_tree(tree->coarse);
			free_image_tree(tree->horizontal);
			free_image_tree(tree->vertical);
			free_image_tree(tree->diagonal);
			tree->coarse=tree->horizontal=tree->vertical=tree->diagonal=NULL;
		}
	}
}
		
/************************************************************************/
/*	Functionname:		decompose_to_level                       */
/* -------------------------------------------------------------------- */
/*	Parameter:							*/
/*            	original: original image		 		*/
/*           	level: decompose to level	 	                */
/*		flt: decompos with filters[0..level]                   	*/
/*		method: transform with filter method			*/
/* -------------------------------------------------------------------- */
/* 	Description: Decomposes an image to an certain level and stores	*/
/*	     only this level in the returned quadtree      		*/
/************************************************************************/
Image_tree decompose_to_level(Image original,int level,FilterGH *flt,enum FilterMethod method)
{	Image_tree tree;
	int e;
	
	tree=new_image_tree();
	tree->image=original;
	
	e=decompose_all(tree,level,flt,method,entropy,1);
	if (!e) return NULL;
	
	free_levels(tree,level);
	
	return tree;

}
	
/************************************************************************/
/*	Functionname:		decomposition                           */
/* -------------------------------------------------------------------- */
/*	Parameter:							*/
/*            	t_img: Image which should be decomposed  		*/
/*           	coarse,horizontal,vertical,diagonal: 	                */
/*			decomposed images                             	*/
/*		method: transform with filter method			*/
/*            	g,h: the transformation is carried out with these filters*/
/* -------------------------------------------------------------------- */
/* 	Description: This carries out one wavelettransformation		*/
/*	     using waveletfilters.            				*/
/************************************************************************/

static int decomposition(Image t_img,Image coarse,Image horizontal,Image vertical,
                               Image diagonal,Filter g,Filter h,enum FilterMethod method)
{ 	Image temp1;
	
 	/*coarse*/	
	temp1=new_image(coarse->width,t_img->height);
	if(!temp1) goto error;          	
        convolute_lines(temp1,t_img,h,method);
	convolute_rows(coarse,temp1,h,method);

	/*horizontal*/
	convolute_rows(horizontal,temp1,g,method);	
	free_image(temp1);
	
        /*vertical*/
	temp1=new_image(vertical->width,t_img->height);
	if(!temp1) goto error;          	
        convolute_lines(temp1,t_img,g,method);
	convolute_rows(vertical,temp1,h,method);

        /*diagonal*/
	convolute_rows(diagonal,temp1,g,method);
	free_image(temp1);

	return 1;

	error:
        	err_SimpleMessage(err_GetErrorMessage(Error_NotEnoughMemory));	
	  	return 0;		

}

/************************************************************************/
/*	Functionname: inv_decomposition					*/
/* -------------------------------------------------------------------- */
/*	Parameter: 							*/
/*		sum: reconstructed image		  		*/
/*		coarse,horizontal,vertical,diagonal: images to carry out*/
/*			the inverse transformation			*/
/*		flt_gh: transform with filters g and h			*/
/* 		method: transform with filter method			*/
/* -------------------------------------------------------------------- */
/* 	Description: Carries out the wavelettransform 			*/
/*	                           		           		*/
/************************************************************************/
static int inv_decomposition(Image sum,Image coarse,Image horizontal,Image vertical,
                               Image diagonal,FilterGH flt_gh,enum FilterMethod method)
{       Image temp1;
	Filter g,h;

	if (flt_gh->type==FTOrtho) {
	  g=flt_gh->g;
	  h=flt_gh->h;
	}
	else {
	  g=flt_gh->gi;
	  h=flt_gh->hi;
	}

	/*coarse*/
	temp1=new_image(coarse->width,sum->height);
	if(!temp1) goto error;          	
	convolute_rows(temp1,coarse,h,method);

        /*horizontal*/
	convolute_rows(temp1,horizontal,g,method);
        convolute_lines(sum,temp1,h,method);
        free_image(temp1);
        
        /*vertical*/
        temp1=new_image(vertical->width,sum->height);
	if(!temp1) goto error;          	
        convolute_rows(temp1,vertical,h,method);

        /*diagonal*/
        convolute_rows(temp1,diagonal,g,method);
        convolute_lines(sum,temp1,g,method);
	
        free_image(temp1);
        
	return 1;

	error:
        	err_SimpleMessage(err_GetErrorMessage(Error_NotEnoughMemory));	
	  	return 0;	
}

/************************************************************************/
/*	Functionname: build_image					*/
/* -------------------------------------------------------------------- */
/*	Parameter: 							*/
/*		quadtree: quadtree with decomposition information	*/
/*	        width,height: image width and height           		*/
/*		RETURN: returns the build up image			*/
/* -------------------------------------------------------------------- */
/* 	Description: builds up an image out of an Image_tree		*/
/*	                           		           		*/
/************************************************************************/
Image build_image(Image_tree quadtree,int width,int height)
{ 	Image ret_img,coarse,horizontal,vertical,diagonal;

	
	ret_img=new_image(width,height);
	if(!ret_img) goto error;          	
	
	width=(width+1)/2;
	height=(height+1)/2;

	if (!(quadtree->image)) {
        	coarse=build_image(quadtree->coarse,width,height);
          	horizontal=build_image(quadtree->horizontal,width,height);
          	vertical=build_image(quadtree->vertical,width,height);
          	diagonal=build_image(quadtree->diagonal,width,height);
          	if (!coarse||!horizontal||!vertical||!diagonal) return NULL;
     
          	copy_into_image(ret_img,coarse,0,0);
	  	copy_into_image(ret_img,horizontal,width,0);
	  	copy_into_image(ret_img,vertical,0,height);
	  	copy_into_image(ret_img,diagonal,width,height);
	  
	  	if (!quadtree->coarse->image) free_image(coarse);
	  	if (!quadtree->horizontal->image) free_image(horizontal);
	  	if (!quadtree->vertical->image) free_image(vertical);
	  	if (!quadtree->diagonal->image) free_image(diagonal);
	  
          	return ret_img;
	}
	else return quadtree->image;

	error:
        	err_SimpleMessage(err_GetErrorMessage(Error_NotEnoughMemory));	
	  	return NULL;	
}

/************************************************************************/
/*	Functionname: inv_transform					*/
/* -------------------------------------------------------------------- */
/*	Parameter: 							*/
/*		tree: tree with decomposition information		*/
/*		flt_gh: transform with filters g and h			*/
/* 		method: transform with filter method			*/
/* -------------------------------------------------------------------- */
/* 	Description: Inverts the wavelettransform,best_basis,best_level */
/*	                           		           		*/
/************************************************************************/
Image inv_transform(Image_tree tree,FilterGH *flt,
                                enum FilterMethod method)

{	int er,width,height;
	Image ret_img,coarse,vertical,horizontal,diagonal;

	if (!tree->image) {
		
		coarse=inv_transform(tree->coarse,flt,method);
        	horizontal=inv_transform(tree->horizontal,flt,method);
	        vertical=inv_transform(tree->vertical,flt,method);
	        diagonal=inv_transform(tree->diagonal,flt,method);
         	if (!coarse||!horizontal||!vertical||!diagonal) return NULL;
 
		width=coarse->width+horizontal->width;
		height=coarse->height+vertical->height;
		
		ret_img=new_image(width,height);
		if(!ret_img) goto error;          	


	        if (tree->flag==0)		/*if flag is set it is a doubletree tiling*/
	        {
//			er=inv_decomposition(ret_img,coarse,horizontal,vertical,diagonal,flt[1],method);
			er=inv_decomposition(ret_img,coarse,horizontal,vertical,diagonal,flt[tree->level],method);
			if (!er) return NULL;
	        }
	  	else
	  	{
	  		copy_into_image(ret_img,coarse,0,0);
	  		copy_into_image(ret_img,horizontal,coarse->width,0);
	  		copy_into_image(ret_img,vertical,0,coarse->height);
	  		copy_into_image(ret_img,diagonal,coarse->width,coarse->height);  		
	  	}

		if (!tree->coarse->image) free_image(coarse);
		if (!tree->horizontal->image) free_image(horizontal);
		if (!tree->vertical->image) free_image(vertical);
		if (!tree->diagonal->image) free_image(diagonal);
	  	
		return ret_img;
	}

	else return tree->image;

	error:
        	err_SimpleMessage(err_GetErrorMessage(Error_NotEnoughMemory));	
	  	return NULL;		
}

/************************************************************************/
/*	Functionname: find_deepest_level				*/
/* -------------------------------------------------------------------- */
/*	Parameter: 							*/
/* -------------------------------------------------------------------- */
/* 	Description: Finds the deepest possible level where width and 	*/
/*			height can divided by two exactly.		*/
/************************************************************************/
int find_deepest_level(int width,int height)
{
	int level=0,w=width,h=height;
	
	while ( !((w%2)||(h%2)))
	{
		w=w/2;
		h=h/2;
		level++;
	}
	
	return level-1;

}

/************************************************************************/
/*	Functionname: convolute_lines					*/
/* -------------------------------------------------------------------- */
/*	Parameter: 							*/
/*		output: output image of wavelettransformation		*/
/*	        input: input image for decomposition            	*/
/*		flt: transform with filter flt				*/
/*		method: transform with filter method			*/
/* -------------------------------------------------------------------- */
/* 	Description: Carries out the wavelettransform for all lines of  */
/*	             the input image					*/
/************************************************************************/
static int convolute_lines(Image output,Image input,Filter flt,enum FilterMethod method)
/*Convolute the lines with filter*/
{       int i;

        for (i=0;i<input->height;i++) {
	  switch(method) {
	  case cutoff:
            filter_cutoff(input,input->width*i,input->width,1,
			output,output->width*i,output->width,1,flt);
	    break;
	  case inv_cutoff:
            filter_inv_cutoff(input,input->width*i,input->width,1,
			output,output->width*i,output->width,1,flt);
	    break;
	  case periodical:
            filter_periodical(input,input->width*i,input->width,1,
			output,output->width*i,output->width,1,flt);
	    break;
	  case inv_periodical:
            filter_inv_periodical(input,input->width*i,input->width,1,
			output,output->width*i,output->width,1,flt);
	    break;
	  case mirror:
            filter_mirror(input,input->width*i,input->width,1,
			output,output->width*i,output->width,1,flt);
	    break;
	  case inv_mirror:
            filter_inv_mirror(input,input->width*i,input->width,1,
			output,output->width*i,output->width,1,flt);
	    break;

	    
	  }
        }
        
        return 1;
}

/************************************************************************/
/*	Functionname: convolute_rows					*/
/* -------------------------------------------------------------------- */
/*	Parameter: 							*/
/*		output: output image of wavelettransformation		*/
/*	        input: input image for decomposition            	*/
/*		flt: transform with filter flt				*/
/*		method: transform with filter method			*/
/* -------------------------------------------------------------------- */
/* 	Description: Carries out the wavelettransform for all rows of   */
/*	             the input image					*/
/************************************************************************/
static int convolute_rows(Image output,Image input,Filter flt,enum FilterMethod method)
/*Convolute the rows with filter*/
{       int i;

        for (i=0;i<input->width;i++)
        {
         	switch (method)
         	{
		case cutoff:
	    		filter_cutoff(input,i,input->height,input->width,
	           		output,i,output->height,output->width,flt);
	    		break;
	  	case inv_cutoff:
	    		filter_inv_cutoff(input,i,input->height,input->width,
	           		output,i,output->height,output->width,flt);
	    		break;
	  	case periodical:
	    		filter_periodical(input,i,input->height,input->width,
	           		output,i,output->height,output->width,flt);
	    		break;
		case inv_periodical:
			filter_inv_periodical(input,i,input->height,input->width,
	           		output,i,output->height,output->width,flt);
	    		break;
	  	case mirror:
	    		filter_mirror(input,i,input->height,input->width,
	           		output,i,output->height,output->width,flt);
	    		break;
		case inv_mirror:
			filter_inv_mirror(input,i,input->height,input->width,
	           		output,i,output->height,output->width,flt);
	    		break;
	    		
	  	}
        }
        return 1;
}

/************************************************************************/
/*	Functionname: sumationq						*/
/* -------------------------------------------------------------------- */
/*	Parameter: 							*/
/*		img: image to compute					*/
/* -------------------------------------------------------------------- */
/* 	Description: compute the sum of quadrats of all elements of  	*/
/*	             the input image					*/
/************************************************************************/
static Pixel sumationq(Image img)
{	Pixel sum=0;
	int i;
	
	for (i=0;i<img->size;i++) {
		sum+=(*img->data+i)*(*img->data+i);
	}
	return sum;
}

/************************************************************************/
/*	Functionname: normq						*/
/* -------------------------------------------------------------------- */
/*	Parameter: 							*/
/*		tree: tree to compute					*/
/* -------------------------------------------------------------------- */
/* 	Description: computes the quadratic norm over all images in 	*/
/*	             the input tree					*/
/************************************************************************/
static Pixel normq(Image_tree tree)
{	Pixel sum=0;

	if (tree->image)
	{
		sum=sumationq(tree->image);
	}
	else 
	{
		if (tree->coarse) sum+=normq(tree->coarse);
		if (tree->horizontal) sum+=normq(tree->horizontal);
		if (tree->vertical) sum+=normq(tree->vertical);
		if (tree->diagonal) sum+=normq(tree->diagonal);
	}
	
	return sum;
}

/************************************************************************/
/*	Functionname: sumation_down					*/
/* -------------------------------------------------------------------- */
/*	Parameter: 							*/
/*		tree: tree to compute					*/
/* 		normq: norm of the images in the tree			*/
/* -------------------------------------------------------------------- */
/* 	Description: computes the Entropy over all (string aded) images */
/*	             in the input tree					*/
/************************************************************************/
static Pixel sumation_down(Image_tree tree, Pixel normq)
{	Pixel sum=0,p;
	int i;
	Image img;
	Pixel *data;

	if (tree->image)
	{
		img=tree->image;
		data=img->data;
		for (i=0;i<img->size;i++,data++)
		{
			if (*data!=0)
			{
				p=(*data)*(*data)/normq;
				sum+=p*log(1/p);
			}
		}
	}
	else
	{
		if (tree->coarse) sum+=sumation_down(tree->coarse,normq);
		if (tree->horizontal) sum+=sumation_down(tree->horizontal,normq);
		if (tree->vertical) sum+=sumation_down(tree->vertical,normq);
		if (tree->diagonal) sum+=sumation_down(tree->diagonal,normq);
	}
	
	return sum;
}

/************************************************************************/
/*	Functionname: comp						*/
/* -------------------------------------------------------------------- */
/* 	Description: used for quicksort for decreasing order		*/
/************************************************************************/
int comp(const Pixel *x,const Pixel *y)
{
	if (*x<*y) return 1;
	else if (*x==*y) return 0;
	else return -1;
}

/************************************************************************/
/*	Functionname: recarea						*/
/*		tree: Image tree to compute				*/
/*		list: target list					*/
/*		list_size: actual size of the list			*/
/* -------------------------------------------------------------------- */
/* 	Description: copies all elements within the tree into an list	*/
/************************************************************************/
static void recarea(Image_tree tree,Pixel *list,int *list_size)
{	Image img;
	
	if (tree->image)
	{
		img=tree->image;
		memcpy(list+(*list_size),img->data,img->size*sizeof(Pixel));
		*list_size+=img->size;
	}
	else 
	{
		if (tree->coarse) recarea(tree->coarse,list,list_size);
		if (tree->horizontal) recarea(tree->horizontal,list,list_size);
		if (tree->vertical) recarea(tree->vertical,list,list_size);
		if (tree->diagonal) recarea(tree->diagonal,list,list_size);
	}
		
}

static void abs_list(Pixel *list,int list_size)
{
	int i;

	for (i=0;i<list_size;i++) list[i]=fabs(list[i]);
}

/************************************************************************/
/*	Functionname: sum_list						*/
/* -------------------------------------------------------------------- */
/* 	Description: computes the sum of all poweres list elements	*/
/************************************************************************/
static Pixel sum_list(Pixel *list,int p,int size)
{	Pixel sum=0;
	int i;
	
	for (i=0;i<size;i++)
	{
		if (p!=1) sum+=pow(list[i],p);
		else sum+=list[i];
	}
	return sum;
}

static Pixel weak_lp(Image_tree tree,int size,int p,double epsilon)
{	Pixel wlp,*list,max=0;
	int *list_size,k;

	list_size=(int *)malloc(sizeof(int));
	if (!list_size) goto error;
	
	*list_size=0;
	
	list=(Pixel *)calloc(size,sizeof(Pixel));
	if (!list) goto error;
	
	recarea(tree,list,list_size);
	abs_list(list,*list_size);

	qsort(list,*list_size, sizeof(Pixel), (int (*)(const void*, const void*)) comp);

	for (k=0;k<size;k++)
	{
		if (k!=0) wlp=pow(k,1/p)*list[k];
		else wlp=0;
		if (wlp>max) max=wlp;
	}

	free(list);
	
	return max;
	
	error:
        	err_SimpleMessage(err_GetErrorMessage(Error_NotEnoughMemory));	
	  	return 0;		
}

static Pixel comp_number(Image_tree tree,int size,int p,double f)
{	Pixel sum=0,*list,min=MAXDOUBLE,norm,npf,normf;
	int *list_size=0,k;

	list_size=(int *)malloc(sizeof(int));
	if (!list_size) goto error;
	*list_size=0;

	list=(Pixel *)calloc(size,sizeof(Pixel));
	if (!list) goto error;
	recarea(tree,list,list_size);
	abs_list(list,*list_size);

	qsort(list,*list_size, sizeof(Pixel), (int (*)(const void*, const void*)) comp);

	norm=sum_list(list,p,size);
	normf=norm*f;

	for (k=0;k<size;k++)
	{
		if (list[k]!=0)
		{
			sum+=pow(list[k],p);
			npf=fabs(sum-normf);
			if (npf<min) min=npf;
		}
	}
	min=min/norm;

	free(list);
	
	return min;

	error:
        	err_SimpleMessage(err_GetErrorMessage(Error_NotEnoughMemory));	
	  	return 0;	
}

static Pixel comp_area(Image_tree tree,int size,int p,double f)
{	Pixel sum=0,*list,norm,vk=0;
	int *list_size=0,k;

	list_size=(int *)malloc(sizeof(int));
	if (!list_size) goto error;
	*list_size=0;

	list=(Pixel *)calloc(size,sizeof(Pixel));
	if (!list) goto error;
	
	recarea(tree,list,list_size);
	abs_list(list,*list_size);
	
	qsort(list,*list_size, sizeof(Pixel), (int (*)(const void*, const void*)) comp);

	norm=sum_list(list,p,size);

	for (k=0;k<size;k++)
	{
		if (list[k]!=0)
		{
			vk+=pow(list[k],p);
			sum+=vk;
			
		}
	}

	free(list);
	
	return (size-sum/norm);

	error:
        	err_SimpleMessage(err_GetErrorMessage(Error_NotEnoughMemory));	
	  	return 0;	
}

static Pixel compute_sdiscrepancy(Image_tree tree,int size)
{	Pixel *list,min,max,factor,maximum=0,x;
	int *list_size=0,k;

	list_size=(int *)malloc(sizeof(int));
	if (!list_size) goto error;
	*list_size=0;

	list=(Pixel *)calloc(size,sizeof(Pixel));
	if (!list) goto error;

	recarea(tree,list,list_size);
	
	qsort(list,*list_size, sizeof(Pixel), (int (*)(const void*, const void*)) comp);

	min=list[0];
	max=list[size-1];
	factor=1/(max-min);

				/*scaling to [0,1]*/
	for (k=0;k<size;k++)
	{
		list[k]=factor*(list[k]-min);
	}

	for (k=0;k<size;k++)
	{
		x=fabs(list[k]-(2*k-1)/(2*size));
		if (x>maximum) maximum=x;
	}
	
	free(list);
	
	return (1/(2*size)+maximum);

	error:
        	err_SimpleMessage(err_GetErrorMessage(Error_NotEnoughMemory));	
	  	return 0;	
}

static Pixel compute_discrepancy(Image_tree tree,int size)
{	Pixel *list,min,max,factor,maximum=0,minimum=0,x;
	int *list_size=0,k;

	list_size=(int *)malloc(sizeof(int));
	if (!list_size) goto error;
	*list_size=0;

	list=(Pixel *)calloc(size,sizeof(Pixel));
	if (!list) goto error;

	recarea(tree,list,list_size);
	
	qsort(list,*list_size, sizeof(Pixel), (int (*)(const void*, const void*)) comp);

	min=list[0];
	max=list[size-1];
	factor=1/(max-min);

				/*scaling to [0,1]*/
	for (k=0;k<size;k++)
	{
		list[k]=factor*(list[k]-min);
	}

	for (k=0;k<size;k++)
	{
		x=((Pixel)k/size-list[k]);
		if (x>maximum) maximum=x;
		else if (x<minimum) minimum=x;
		
	}
	
	free(list);
	
	return (1/size+maximum-minimum);

	error:
        	err_SimpleMessage(err_GetErrorMessage(Error_NotEnoughMemory));	
	  	return 0;	
}

static Pixel compute_concentration(Image_tree tree,int size)
{	Pixel *list,min,max,factor,lkm=0,length,sum=0,value,norm;
	int *list_size=0,k,next=0,last=0,i=0;

	list_size=(int *)malloc(sizeof(int));
	if (!list_size) goto error;
	*list_size=0;

	list=(Pixel *)calloc(size+1,sizeof(Pixel));
	if (!list) goto error;

	recarea(tree,list,list_size);
	
	qsort(list,*list_size, sizeof(Pixel), (int (*)(const void*, const void*)) comp);

	min=list[0];
	max=list[size-1];
	length=(max-min)/100;
	
	factor=1/(max-min);
	for (k=0;k<size;k++)
	{
		list[k]=factor*(list[k]-min);
	}
	
	norm=size*sum_list(list,1,size);
	length=0.01;
	value=length;
	
	list[size]=max+value;
	
	for (k=0;k<100;k++)
	{
		while ((list[i]<value)&(i<size))
		{
			sum+=list[i];
			next++;
			i++;
		}
		lkm+=(next-last)*sum/norm;
		value+=length;
		last=next;
		sum=0;
	}
	
	return -lkm;
	
	error:
        	err_SimpleMessage(err_GetErrorMessage(Error_NotEnoughMemory));	
	  	return 0;	
}
/************************************************************************/
/*	Functionname: compute_entropy					*/
/* -------------------------------------------------------------------- */
/*	Parameter: 							*/
/*		img: Image from which the entropy should be computed	*/
/*		cost: choosen costfunction				*/
/*		epsilon: limit for threshold method			*/
/* -------------------------------------------------------------------- */
/* 	Description: computes entropy of an image			*/
/************************************************************************/
static double compute_entropy(Image img,enum Information_Cost cost,double epsilon)
{       double sum=0,x=0;
	int i;
	Pixel *data;
	
	data=img->data;
	
	switch(cost) {

	case threshold:
  		for(i=0;i<img->size;i++) 
	    		if (fabs(img->data[i])>epsilon) sum++;  
  	break;

	case log_energy:  
        	for(i=0;i<img->size;i++,data++) {
        		x=(*data) * (*data);
	  		if (x!=0) sum+=(x*log(1/x));
	  	}
	break;

	case norml:
        	for(i=0;i<img->size;i++,data++) {
        		x=fabs(*data);
	  		sum+=x;
	  	}
	break;	

	case norml2:
        	for(i=0;i<img->size;i++,data++) {
        		x=(*data) * (*data);
	  		sum+=x;
	  	}
	  	sum=pow(sum,0.5);
	break;	

	case entropy:
		for(i=0;i<img->size;i++,data++) {
	    		x=(*data)*(*data);
	    		if (x!=0) sum-=(x*log(x));
	  	}
	break;
	 
	case gauss_markov:
		for(i=0;i<img->size;i++) {
	    		x=(img->data[i])*(img->data[i]);
		if (x!=0) sum+=log(x*x);
		}
	break;

        }

        return sum;
}

/************************************************************************/
/*	Functionname: compute_non_additive				*/
/* -------------------------------------------------------------------- */
/*	Parameter: 							*/
/*		tree: Image tree from which the entropy should be 	*/
/*			computed					*/
/*		size :	size of the image				*/
/*		cost: choosen costfunction				*/
/*		epsilon: limit for threshold method			*/
/*		down: decides if only the first image should be computed*/ 
/* -------------------------------------------------------------------- */
/* 	Description: computes entropy of an image			*/
/************************************************************************/
static Pixel compute_non_additive(Image_tree tree,int size,enum Information_Cost cost,double epsilon,int down)
{	Pixel sum=0,normx;
	Image img=NULL;

	if (down)
	{
		img=tree->image;
		tree->image=NULL;
	}
	switch (cost)
	{
		case shanon:
			normx=normq(tree);
			sum=-sumation_down(tree,normx);
			
			break;
		case weak_l:
			sum=weak_lp(tree,size,1,epsilon);
			break;
		case weak_lq:
			sum=weak_lp(tree,size,2,epsilon);
			break;
		case compression_number:
			sum=comp_number(tree,size,1,epsilon);
			break;
		case compression_numberq:
			sum=comp_number(tree,size,2,epsilon);
			break;
		case compression_area:
			sum=comp_area(tree,size,1,epsilon);
			break;
		case compression_areaq:
			sum=comp_area(tree,size,2,epsilon);
			break;
		case discrepancy:
			sum=compute_discrepancy(tree,size);
			break;	
		case sdiscrepancy:
			sum=compute_sdiscrepancy(tree,size);
			break;
		case concentration:
			sum=compute_concentration(tree,size);
			break;
			
					
	}

	if (down) tree->image=img;
	
	return sum;
}

int rec_double(Image_tree dtree,int level,FilterGH *flt,enum FilterMethod method,enum Information_Cost cost,double epsilon)

{	int min,width,height;
	double sum=0;
	Image c,h,v,d;

	dtree->level=0;
	
	if (cost>=shanon)
	{
		dtree->entropy=compute_non_additive(dtree,dtree->image->size,cost,epsilon,0);
	}
	else 	dtree->entropy=compute_entropy(dtree->image,cost,epsilon);
	
	dtree->doubletree=best_basis(dtree->image,level,flt,method,cost,epsilon);
	
	min=dtree->image->width;
	if (dtree->image->height<min) min=dtree->image->height;

	if (doubletree_min<min)
	{
		width=(dtree->image->width+1)/2;
		height=(dtree->image->height+1)/2;
		
		dtree->coarse=new_image_tree();
		dtree->horizontal=new_image_tree();
		dtree->vertical=new_image_tree();	
		dtree->diagonal=new_image_tree();
		
		c=new_image(width,height);
		h=new_image(width,height);
		v=new_image(width,height);
		d=new_image(width,height);
		if(!c||!h||!v||!d) goto error;          	
		

		copy_part_of_image(c,dtree->image,0,0);
		copy_part_of_image(h,dtree->image,width,0);
		copy_part_of_image(v,dtree->image,0,height);
		copy_part_of_image(d,dtree->image,width,height);

		dtree->coarse->image=c;
		dtree->horizontal->image=h;
		dtree->vertical->image=v;	
		dtree->diagonal->image=d;

		rec_double(dtree->coarse,level,flt,method,cost,epsilon);
 		rec_double(dtree->horizontal,level,flt,method,cost,epsilon);
 		rec_double(dtree->vertical,level,flt,method,cost,epsilon);
 		rec_double(dtree->diagonal,level,flt,method,cost,epsilon);

 		/* going back in recursion*/

 		sum=dtree->coarse->entropy+dtree->horizontal->entropy+
 			dtree->vertical->entropy+dtree->diagonal->entropy;

 		if (sum>dtree->entropy)
 		{
 			/*take image*/
 			
 			free_image_tree(dtree->coarse);
  			free_image_tree(dtree->horizontal);
  			free_image_tree(dtree->vertical);
  			free_image_tree(dtree->diagonal);
  			dtree->coarse=dtree->horizontal=dtree->vertical=dtree->diagonal=NULL;
 		}
 		else
 		{	/*take tiling*/
 			dtree->entropy=sum;
 			free_image(dtree->image);
 			dtree->image=NULL;
 		}
  			
 		if (dtree->entropy>dtree->doubletree->entropy)
 		{	
 			/*take best basis tree*/

 			dtree->entropy=dtree->doubletree->entropy;
 			
 			if(dtree->coarse) free_image_tree(dtree->coarse);
  			if(dtree->horizontal) free_image_tree(dtree->horizontal);
  			if(dtree->vertical) free_image_tree(dtree->vertical);
  			if(dtree->diagonal) free_image_tree(dtree->diagonal);
 					
  			dtree->coarse=dtree->doubletree->coarse;
  			dtree->horizontal=dtree->doubletree->horizontal;
  			dtree->vertical=dtree->doubletree->vertical;
  			dtree->diagonal=dtree->doubletree->diagonal;
  			
 			free_image(dtree->image);
 			dtree->image=NULL;
 			free(dtree->doubletree);
 			dtree->doubletree=NULL;
   			
  		}
 		else
 		{
 			dtree->flag=1;
 			if(dtree->doubletree) free_image_tree(dtree->doubletree);
 			dtree->doubletree=NULL;
 		}
	}
		
	return 1;

	error:
        	err_SimpleMessage(err_GetErrorMessage(Error_NotEnoughMemory));	
	  	return 0;		
}		

static void save_structur(Image_tree tree,FILE *fp,int pos)
{
	int shift,next_pos,max;

	if (tree->flag)
	{
		fprintf(fp,"%d ",pos);
	
		shift=pos-(pow(4,tree->level-1)-1)*4/3-1;
		max=(int) ((pow(4,tree->level)-1)*4/3);
		next_pos=max+4*shift+1;
		if (tree->coarse) save_structur(tree->coarse,fp,next_pos);
		if (tree->horizontal) save_structur(tree->horizontal,fp,next_pos+1);
		if (tree->vertical) save_structur(tree->vertical,fp,next_pos+2);
		if (tree->diagonal) save_structur(tree->diagonal,fp,next_pos+3);	
	}
}

static int is_in_list(int *list,int len, int x)
{
	int i,found=0;

	for (i=0;i<len;i++)
	{
		if (list[i]==x)
		{
			found=1;
			i=len;
		}
	}

	return found;
}

static void write_flags(Image_tree tree,int *list,int len,int pos)
{
	int shift,next_pos,max;

	if (is_in_list(list,len,pos))
	{
		tree->flag=1;
	
		shift=pos-(pow(4,tree->level-1)-1)*4/3-1;
		max=(int) ((pow(4,tree->level)-1)*4/3);
		next_pos=max+4*shift+1;
		
		write_flags(tree->coarse,list,len,next_pos);
		write_flags(tree->horizontal,list,len,next_pos+1);
		write_flags(tree->vertical,list,len,next_pos+2);
		write_flags(tree->diagonal,list,len,next_pos+3);		
	}
}

/************************************************************************/
/*	Functionname:		err_simple_message			*/
/* -------------------------------------------------------------------- */
/*	Parameter:							*/
/*	    char *: string that contains information about an		*/
/*		    error the user should know.				*/
/* -------------------------------------------------------------------- */
/* 	Description:							*/
/*	    Prints error messages for the user.				*/
/************************************************************************/

void err_SimpleMessage(char *message)
{
	fprintf(stderr,"%s\n",message);
}

/************************************************************************/
/*	Functionname: 		err_get_message				*/
/* -------------------------------------------------------------------- */
/* 	Return value:   Errormessage for this specific error.		*/
/*	Parameter:							*/
/*	    Error err:	Error whose errormessage should be returned	*/
/* -------------------------------------------------------------------- */
/*	Description:							*/
/************************************************************************/
char * err_GetErrorMessage(Error err)
{

        switch (err)
        {
            case Error_NotImplemented:
                return "Sorry, this is not implemented yet. ";
                break;

            case Error_AssertionFailed:
                return "Sorry, an internal assertion was violated.\n"
                       "This action can not be completed. :-(";
                break;

            case Error_NotEnoughMemory:
                return "Sorry, there is not enough memory";
                break;

            case Error_Limitation:
                return "Some limitation of the program exceeded";
                break;

	/* - FILES - */

	    case Error_CantOpenFile:
		return "Could not open file";
		break;

	    case Error_CantCreateFile:
		return "Could not create file";
		break;

	    case Error_CantCloseFile:
		return "Could not close file";
		break;
            
	    case Error_InternalError:
                return "Sorry, an internal error occured.\n"
                       "This action can not be completed. :-(";
                break;

            default:
                return "Sorry, but an unknown error ocurred.\n"
                       "This action can not be completed. :-(";
                break;


	}
}
