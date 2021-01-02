/*------------------------------------------------------------------------
This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License version 2 as
published by the Free Software Foundation.

			---  IMPORTANT NOTE ---

  1. Visional illusions will mar the appearance of your GUI design.
  2. Always Select correct colours to cover visual defects for your GUI!


TODO: Using pointer params is better than just adding inline qualifier ?

EGI Color Functions

Midas ZHou
-----------------------------------------------------------------------*/

#include "egi_color.h"
#include "egi_debug.h"
#include "egi_utils.h"
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
//#include <time.h>
#include <stdio.h>
#include <sys/time.h> /*gettimeofday*/


/*------------------------------------------------------------------------------------
Get a 16bit color value/alpha between two given colors/alphas by interpolation.

@color1, color2:	The first and second input color value.
@alpha1, alpha2:	The first and second input alpha value.
@f15_ratio:		Ratio for interpolation, in fixed point type f15.
			that is [0-1]*(2^15);
			!!! A bigger ratio makes alpha more closer to color2/alpha2 !!!
@colro, alpha		Pointer to pass output color and alpha.
			Ignore if NULL.

-------------------------------------------------------------------------------------*/
inline void egi_16bitColor_interplt( EGI_16BIT_COLOR color1, EGI_16BIT_COLOR color2,
				     unsigned char alpha1,  unsigned char alpha2,
				     int f15_ratio, EGI_16BIT_COLOR* color, unsigned char *alpha)
{
	unsigned int R,G,B,A;

	/* Interpolate color value */
	if(color) {
	 	R  =((color1&0xF800)>>8)*((1U<<15)-f15_ratio);	/* R */
		R +=((color2&0xF800)>>8)*f15_ratio;
		R >>= 15;

	 	G  =((color1&0x7E0)>>3)*((1U<<15)-f15_ratio);	/* G */
		G +=((color2&0x7E0)>>3)*f15_ratio;
		G >>= 15;

 		B  =((color1&0x1F)<<3)*((1U<<15)-f15_ratio);	/* B */
		B +=((color2&0x1F)<<3)*f15_ratio;
		B >>= 15;

        	*color=COLOR_RGB_TO16BITS(R, G, B);
	}
	/* Interpolate alpha value */
	if(alpha) {
	        A  = alpha1*((1U<<15)-f15_ratio);  /* R */
	        A += alpha2*f15_ratio;
		A >>= 15;
		*alpha=A;
	}
}

/*------------------------------------------------------------------
Get average color value

@colors:	array of color values
@n		number of input colors

--------------------------------------------------------------------*/
inline EGI_16BIT_COLOR egi_16bitColor_avg(EGI_16BIT_COLOR *colors, int n)
{
	int i;
	unsigned int avgR,avgG,avgB;

	/* to enusre array colors is not NULL */
	avgR=avgG=avgB=0;
	for(i=0; i<n; i++) {
        	avgR += ((colors[i]&0xF800)>>8);
	        avgG += ((colors[i]&0x7E0)>>3);
        	avgB += ((colors[i]&0x1F)<<3);
        }

        return COLOR_RGB_TO16BITS(avgR/n, avgG/n, avgB/n);
}


/*------------------------------------------------------------------
                16bit color blend function
Note: Ignore back alpha value.
--------------------------------------------------------------------*/
inline EGI_16BIT_COLOR egi_16bitColor_blend(int front, int back, int alpha)
{
	float	fa;
	float	gamma=2.2; //0.45; //2.2;

#if 0 	/* Gamma correction, SLOW!!! */
	fa=(alpha+0.5)/256;
	fa= pow(fa,1.0/gamma);
	alpha=(unsigned char)(fa*256-0.5);

#elif 1 /* WARNING!!!! Must keep 0 alplha value unchanged, or BLACK bk color will appear !!!
           a simple way to improve sharpness */
	alpha = alpha*3/2;
        if(alpha>255)
                alpha=255;
	//if(alpha<10)
	//	alpha=0;
#endif

        return COLOR_16BITS_BLEND(front, back, alpha);
}


/*------------------------------------------------------------------------------
                16bit color blend function
Note: Back alpha value also applied.
-------------------------------------------------------------------------------*/
inline EGI_16BIT_COLOR egi_16bitColor_blend2(EGI_16BIT_COLOR front, unsigned char falpha,
					     EGI_16BIT_COLOR back,  unsigned char balpha )
{
	/* applay front color if both 0 */
	if(falpha==0 && balpha==0) {
		return front;
	}
	/* blend them */
	else {
          return  COLOR_RGB_TO16BITS (
         	  	  ( ((front&0xF800)>>8)*falpha + ((back&0xF800)>>8)*balpha) /(falpha+balpha),
		          ( ((front&0x7E0)>>3)*falpha + ((back&0x7E0)>>3)*balpha) /(falpha+balpha),
        		  ( ((front&0x1F)<<3)*falpha + ((back&0x1F)<<3)*balpha) /(falpha+balpha)
                  );
	}
}


/*-------------------------------------------------------------------------
Get a random 16bit color from Douglas.R.Jacobs' RGB Hex Triplet Color Chart

@rang:  Expected color range:
		0--all range
		1--light color
		2--medium range
		3--deep color

with reference to  http://www.jakesan.com

R: 0x00-0x33-0x66-0x99-0xcc-0xff
G: 0x00-0x33-0x66-0x99-0xcc-0xff
B: 0x00-0x33-0x66-0x99-0xcc-0xff

------------------------------------------------------------------------*/
inline EGI_16BIT_COLOR egi_color_random(enum egi_color_range range)
{
        int i,j;
        uint8_t color[3]; /*RGB*/
        struct timeval tmval;
        EGI_16BIT_COLOR retcolor;

        gettimeofday(&tmval,NULL);
        srand(tmval.tv_usec);

        /* random number 0-5 */
        for(i=0;i<3;i++)
        {
	        #if 0   /* Step 0x33 */
                j=(int)(6.0*rand()/(RAND_MAX+1.0));
                color[i]= 0x33*j;  /*i=0,1,2 -> R,G,B, j=0-5*/
		#else   /* Step 0x11 */
                j=(int)(15.0*rand()/(RAND_MAX+1.0));
                color[i]= 0x11*j;  /*i=0,1,2 -> R,G,B, j=0-5*/
		#endif

                if( range > 0 && i==1) /* if not all color range */
		{
			/* to select G range, so as to select color range */
                        if( color[i] >= 0x33*(2*range-2) && color[i] <= 0x33*(2*range-1) )
                        {
		                EGI_PDEBUG(DBG_COLOR," ----------- color G =0X%02X\n",color[i]);
				continue;
			}
			else /* retry */
			{
                                i--;
                                continue;
                        }
		}
        }

        retcolor=COLOR_RGB_TO16BITS(color[0],color[1],color[2]);

        EGI_PDEBUG(DBG_COLOR,"egi random color GRAY: 0x%04X(16bits) / 0x%02X%02X%02X(24bits) \n",
									retcolor,color[0],color[1],color[2]);
        return retcolor;
}


/*-----------------------------------------------------------
Get a random color value with Min. Luma/brightness Y.

!!! TOo big Luma will cost much more time !!!

@rang:  Expected color range:
		0--all range
		1--light color
		2--medium range
		3--deep color

@luman: Min. luminance of the color
-------------------------------------------------------------*/
EGI_16BIT_COLOR egi_color_random2(enum egi_color_range range, unsigned char luma)
{
	EGI_16BIT_COLOR color;

	/* Set LIMIT of input Luma
	 * Rmax=0xF8; Gmax=0xFC; Bmax=0xF8
         * Max Luma=( 307*(31<<3)+604*(63<<2)+113*(31<<3) )>>10 = 250;
	 */
	if( luma > 220 ) {
		printf("%s: Input luma is >220!\n",__func__);
	}
	if( luma > 250 )
		luma=250;

	do {
		color=egi_color_random(range);
	} while( egi_color_getY(color) < luma );

	return color;
}


/*---------------------------------------------------
Get a random 16bit gray_color from Douglas.R.Jacobs'
RGB Hex Triplet Color Chart

rang: color range:
	0--all range
	1--light color
	2--mid range
	3--deep color

with reference to  http://www.jakesan.com
R: 0x00-0x33-0x66-0x99-0xcc-0xff
G: 0x00-0x33-0x66-0x99-0xcc-0xff
B: 0x00-0x33-0x66-0x99-0xcc-0xff
---------------------------------------------------*/
EGI_16BIT_COLOR egi_colorGray_random(enum egi_color_range range)
{
        int i;
        uint8_t color; /*R=G=B*/
        struct timeval tmval;
        uint16_t ret;

        gettimeofday(&tmval,NULL);
        srand(tmval.tv_usec);

        /* random number 0-5 */
        for(;;)
        {
                i=(int)(15.0*rand()/(RAND_MAX+1.0));
                color= 0x11*i;

                if( range > 0 ) /* if not all color range */
		{
			/* to select R/G/B SAME range, so as to select color-GRAY range */
                        if( color>=0x11*5*(range-1) && color <= 0x11*(5*range-1) )
				break;
			else /* retry */
                                continue;
		}

		break;
        }

        ret=(EGI_16BIT_COLOR)COLOR_RGB_TO16BITS(color,color,color);

        EGI_PDEBUG(DBG_COLOR,"egi random color GRAY: 0x%04X(16bits) / 0x%02X%02X%02X(24bits) \n",
											ret,color,color,color);
        return ret;
}

/*---------------------------------------------------------------------
Color Luminance/Brightness control, brightness to be increase/decreased
according to k.

@color	 Input color in 16bit
@k	 Deviation value

Note:
1. Permit Y<0, otherwise R,G,or B MAY never get  to 0, when you need
   to obtain a totally BLACK RGB(000) color in conversion, as of a check
   point, say.
2. R,G,B each contributes differently to the luminance, with G the most and B the least !

	--- RGB to YUV ---
Y=0.30R+0.59G+0.11B=(307R+604G+113G)>>10   [0-255]
U=0.493(B-Y)+128=( (505(B-Y))>>10 )+128    [0-255]
V=0.877(R-Y)+128=( (898(R-Y))>>10 )+128	   [0-255]

	--- YUV to RGB ---
R=Y+1.4075*(V-128)=Y+1.4075*V-1.4075*128
	=(Y*4096 + 5765*V -737935)>>12
G=Y-0.3455*(U-128)-0.7169*(V-128)=Y+136-0.3455U-0.7169V
	=(4096*Y+(136<<12)-1415*U-2936*V)>>12
B=Y+1.779*(U-128)=Y+1.779*U-1.779*128
	=(Y*4096+7287*U-932708)>>12
---------------------------------------------------------------------*/
EGI_16BIT_COLOR egi_colorLuma_adjust(EGI_16BIT_COLOR color, int k)
{
	int32_t R,G,B; /* !!! to be signed int32, same as YUV */
	int32_t Y,U,V;

	/* get 3*8bit R/G/B */
	R = (color>>11)<<3;
	G = (((color>>5)&(0b111111)))<<2;
	B = (color&0b11111)<<3;
//	printf(" color: 0x%02x, ---  R:0x%02x -- G:0x%02x -- B:0x%02x  \n",color,R,G,B);
//	return (EGI_16BIT_COLOR)COLOR_RGB_TO16BITS(R,G,B);

	/* convert RBG to YUV */
	Y=(307*R+604*G+113*B)>>10;
	U=((505*(B-Y))>>10)+128;
	V=((898*(R-Y))>>10)+128;
	//printf("R=%d, G=%d, B=%d  |||  Y=%d, U=%d, V=%d \n",R,G,B,Y,U,V);
	/* adjust Y, k>0 or k<0 */
	Y += k; /* (k<<12); */
	if(Y<0) {
		Y=0;
//		printf("------ Y=%d <0 -------\n",Y);
		/* !! Let Y<0,  otherwise R,G,or B MAY never get back to 0 when you need a totally BLACK */
		// Y=0; /* DO NOT set to 0 when you need totally BLACK RBG */
	}
	else if(Y>255)
		Y=255;

	/* convert YUV back to RBG */
	R=(Y*4096 + 5765*V -737935)>>12;
	//printf("R'=0x%03x\n",R);
	if(R<0)R=0;
	else if(R>255)R=255;

	G=((4096*Y-1415*U-2936*V)>>12)+136;
	//printf("G'=0x%03x\n",G);
	if(G<0)G=0;
	else if(G>255)G=255;

	B=(Y*4096+7287*U-932708)>>12;
	//printf("B'=0x%03x\n",B);
	if(B<0)B=0;
	else if(B>255)B=255;
	//printf(" Input color: 0x%02x, aft YUV adjust: R':0x%03x -- G':0x%03x -- B':0x%03x \n",color,R,G,B);

	return (EGI_16BIT_COLOR)COLOR_RGB_TO16BITS(R,G,B);
}


/*----------------------------------------------
	Convert YUYV to 24BIT RGB color
Y,U,V [0 255]

@src:	Source of YUYV data.
@dest:	Dest of RGB888 data
@w,h:	Size of image blocks.
	W,H MUST be multiples of 2.

@reverse:	To reverse data.

Note:
1. The caller MUST ensure memory space for dest!

Return:
	0	OK
	<0	Fails
-----------------------------------------------*/
int egi_color_YUYV2RGB888(const unsigned char *src, unsigned char *dest, int w, int h, bool reverse)
{
	int i,j;
	unsigned char y1,y2,u,v;
	int r1,g1,b1, r2,g2,b2;

	if(src==NULL || dest==NULL)
		return -1;

	if(w<1 || h<1)
		return -2;

	for(i=0; i<h; i++) {
	    for(j=0; j<w; j+=2) { /* 2 pixels each time */

		if(reverse) {
			y1 =*(src + (((h-1-i)*w+j)<<1));
			u  =*(src + (((h-1-i)*w+j)<<1) +1);
			y2 =*(src + (((h-1-i)*w+j)<<1) +2);
			v  =*(src + (((h-1-i)*w+j)<<1) +3);
		}
		else {
			y1 =*(src + ((i*w+j)<<1));
			u  =*(src + ((i*w+j)<<1) +1);
			y2 =*(src + ((i*w+j)<<1) +2);
			v  =*(src + ((i*w+j)<<1) +3);
		}

		r1 =(y1*4096 + 5765*v -737935)>>12;
		g1 =((4096*y1-1415*u-2936*v)>>12)+136;
		b1 =(y1*4096+7287*u-932708)>>12;
		if(r1>255) r1=255; else if(r1<0) r1=0;
		if(g1>255) g1=255; else if(g1<0) g1=0;
		if(b1>255) b1=255; else if(b1<0) b1=0;

		r2 =(y2*4096 + 5765*v -737935)>>12;
		g2 =((4096*y2-1415*u-2936*v)>>12)+136;
		b2 =(y2*4096+7287*u-932708)>>12;
		if(r2>255) r2=255; else if(r2<0) r2=0;
		if(g2>255) g2=255; else if(g2<0) g2=0;
		if(b2>255) b2=255; else if(b2<0) b2=0;

		*(dest + (i*w+j)*3)	=(unsigned char)r1;
		*(dest + (i*w+j)*3 +1)	=(unsigned char)g1;
		*(dest + (i*w+j)*3 +2)	=(unsigned char)b1;
		*(dest + (i*w+j)*3 +3)	=(unsigned char)r2;
		*(dest + (i*w+j)*3 +4)	=(unsigned char)g2;
		*(dest + (i*w+j)*3 +5)	=(unsigned char)b2;
	    }
	}

	return 0;
}


/*----------------------------------------------
	Convert YUYV to YUV
Y,U,V [0 255]

@src:	Source of YUYV data.
@dest:	Dest of YUV data
@w,h:	Size of image blocks.
	W,H MUST be multiples of 2.

@reverse:	To reverse data.

Note:
1. The caller MUST ensure memory space for dest!

Return:
	0	OK
	<0	Fails
-----------------------------------------------*/
int egi_color_YUYV2YUV(const unsigned char *src, unsigned char *dest, int w, int h, bool reverse)
{
	int i,j;
	unsigned char y1,y2,u,v;

	if(src==NULL || dest==NULL)
		return -1;

	if(w<1 || h<1)
		return -2;

	for(i=0; i<h; i++) {
	    for(j=0; j<w; j+=2) { /* 2 pixels each time */

		if(reverse) {
			y1 =*(src + (((h-1-i)*w+j)<<1));
			u  =*(src + (((h-1-i)*w+j)<<1) +1);
			y2 =*(src + (((h-1-i)*w+j)<<1) +2);
			v  =*(src + (((h-1-i)*w+j)<<1) +3);
		}
		else {
			y1 =*(src + ((i*w+j)<<1));
			u  =*(src + ((i*w+j)<<1) +1);
			y2 =*(src + ((i*w+j)<<1) +2);
			v  =*(src + ((i*w+j)<<1) +3);
		}

		*(dest + (i*w+j)*3)	=y1;
		*(dest + (i*w+j)*3 +1)	=u;
		*(dest + (i*w+j)*3 +2)	=v;
		*(dest + (i*w+j)*3 +3)	=y2;
		*(dest + (i*w+j)*3 +4)	=u;
		*(dest + (i*w+j)*3 +5)	=v;
	    }
	}

	return 0;
}


/*--------------------------------------------------
 Get Y(Luma/Brightness) value from a 16BIT RGB color
 as of YUV

 Y=0.30R+0.59G+0.11B=(307R+604G+113G)>>10 [0 255]
---------------------------------------------------*/
unsigned char egi_color_getY(EGI_16BIT_COLOR color)
{
        uint16_t R,G,B;

        /* get 3*8bit R/G/B */
        R = (color>>11)<<3;
        G = (((color>>5)&(0b111111)))<<2;
        B = (color&0b11111)<<3;
        //printf(" color: 0x%02x, ---  R:0x%02x -- G:0x%02x -- B:0x%02x  \n",color,R,G,B);

        /* convert to Y */
        return (307*R+604*G+113*B)>>10;
}


/*--------------------------------------------------------------
Convert HSV to RGB
H--Hue 		[0 360]  or X%360
S--Saturation	[0 100%]*10000
V--Value 	[0 255]

Note: All vars to be float type will be more accurate!
---------------------------------------------------------------*/
EGI_16BIT_COLOR egi_color_HSV2RGB(const EGI_HSV_COLOR *hsv)
{
	unsigned int h, hi,p,q,t,v;  /* float type will be more accurate! */
	float f;
	EGI_16BIT_COLOR color;

	if(hsv==NULL)
		return WEGI_COLOR_BLACK;

	/* Convert hsv->h to [0 360] */
	if( hsv->h < 0)
		h=360+(hsv->h%360);
	else
		h=hsv->h%360;

	v=hsv->v;
	hi=(h/60)%6;
	f=h/60.0-hi;
	p=v*(10000-hsv->s)/10000;
	q=round(v*(10000-f*hsv->s)/10000);
	t=round(v*(10000-(1-f)*hsv->s)/10000);

	switch(hi) {
		case 0:
			color=COLOR_RGB_TO16BITS(v,t,p); break;
		case 1:
			color=COLOR_RGB_TO16BITS(q,v,p); break;
		case 2:
			color=COLOR_RGB_TO16BITS(p,v,t); break;
		case 3:
			color=COLOR_RGB_TO16BITS(p,q,v); break;
		case 4:
			color=COLOR_RGB_TO16BITS(t,p,v); break;
		case 5:
			color=COLOR_RGB_TO16BITS(v,p,q); break;
	}

	return color;
}

/*--------------------------------------------------------------
Convert RGB to HSV
H--Hue 		[0 360]  or X%360
S--Saturation	[0 100%]*10000
V--Value 	[0 255]

Note: All vars to be float type will be more accurate!

Return:
	0	OK
	<0	Fails
---------------------------------------------------------------*/
int egi_color_RGB2HSV(EGI_16BIT_COLOR color, EGI_HSV_COLOR *hsv)
{
	uint8_t R,G,B;
	uint8_t max,min;

	if(hsv==NULL) return -1;

	/* Get 3*8bit R/G/B */
	R = (color>>11)<<3;
	G = (((color>>5)&(0b111111)))<<2;
	B = (color&0b11111)<<3;

	/* Get max and min */
	max=R;
	if( max < G ) max=G;
	if( max < B ) max=B;
	min=R;
	if( min > G ) min=G;
	if( min > B ) min=B;

	/* Cal. hue */
	if( max==min )
		hsv->h=0;
	else if(max==R && G>=B)
		hsv->h=60*(G-B)/(max-min)+0;
	else if(max==R && G<B)
		hsv->h=60*(G-B)/(max-min)+360;
	else if(max==G)
		hsv->h=60*(B-R)/(max-min)+120;
	else if(max==B)
		hsv->h=60*(R-G)/(max-min)+240;

	/* Cal. satuarion */
	if(max==0)
		hsv->s=0;
	else
		hsv->s=10000*(max-min)/max;

	/* Cal. value */
	hsv->v=max;

	return 0;
}


/*-------------------------------------------------------
Create an EGI_COLOR_BANDMAP and set default color.

@color:	Initial color for the map.
@len:	Initial total length of the map.
Return:
	An pointer	OK
	NULL		Fails
-------------------------------------------------------*/
EGI_COLOR_BANDMAP *egi_colorBandMap_create(EGI_16BIT_COLOR color, unsigned int len)
{
	EGI_COLOR_BANDMAP *map;

	/* Limit len */
	if(len<=0)len=1;

	/* Calloc map */
	map=calloc(1,sizeof(EGI_COLOR_BANDMAP));
	if(map==NULL)
		return NULL;
	map->bands=calloc(COLORMAP_BANDS_GROW_SIZE, sizeof(EGI_COLOR_BAND));
	if(map->bands==NULL) {
		free(map);
		return NULL;
	}

	/* Set default color to the first band */
	map->bands[0].pos=0;
	map->bands[0].len=len;
	map->bands[0].color=color;

	/* Set other params */
	map->size=1;  		/* Init. one band */
	map->capacity=COLORMAP_BANDS_GROW_SIZE;

	return map;
}

/*------------------------------
Free an EGI_COLOR_BANDMAP.
-------------------------------*/
void egi_colorBandMap_free(EGI_COLOR_BANDMAP **map)
{
	if(map==NULL || *map==NULL)
		return;

	free( (*map)->bands );
	free( *map );

	*map=NULL;
}


/*-----------------------------------------------------------------------
Pick color from the band map, according to its position in the band map.
Note:
1. If map unavailbale, return 0 as black;
2. If position is out of range, then it returns the color of last band.
TODO: A fast way

@map:	An EGI_COLOR_BANDMAP pointer
@pos:   Position of the band map.
----------------------------------------------------------------------*/
EGI_16BIT_COLOR  egi_colorBandMap_pickColor(const EGI_COLOR_BANDMAP *map, unsigned int pos)
{
	unsigned int i;

	if(map==NULL || map->bands==NULL)
		return 0;

	for(i=0; i< map->size-1; i++) {
		if( pos >= map->bands[i].pos && pos < map->bands[i+1].pos )
			return map->bands[i].color;
	}

	/* Wahtever, same color as the last band */
	//if( map->bands[ map->size-1 ].pos  )
	return map->bands[ map->size-1 ].color;
}


/*-----------------------------------------------------------------------
Get index of map->bands[] accroding to pos.

Note:
1. If map is unavailable, it returns 0!
2. If pos out of range, it returns the last band index(map->size-1).

TODO: A fast way.


@map:	An EGI_COLOR_BANDMAP pointer
@pos:   Position of the band map.

----------------------------------------------------------------------*/
unsigned int egi_colorBandMap_get_bandIndex(const EGI_COLOR_BANDMAP *map, unsigned int pos)
{
	unsigned int i;

	if(map==NULL || map->bands==NULL)
		return 0;

	for(i=0; i< map->size-1; i++) {
		if( pos >= map->bands[i].pos && pos < map->bands[i+1].pos )
			return map->bands[i].pos;
	}

	/* Same color as the last band! */
	return map->bands[ map->size-1 ].pos;
}

/*-------------------------------------------------------------------
Split a color band in map into two, pos is the start position of the
new band. The new band holds the same color.

Note:
1. If pos is out of map range, then need NOT to split the last band.
2. If pos is already start position of a band in map, then need NOT
   to split the band.

@pos:   Position to the band map, as split position.
@map:	An EGI_COLOR_BANDMAP pointer

Return:
	0	OK
	<0	Fails
--------------------------------------------------------------------*/
int  egi_colorBandMap_splitBand(EGI_COLOR_BANDMAP *map, unsigned int pos)
{
	unsigned int  index; /* pos located in map->bands[index] */

	if(map==NULL || map->bands==NULL)
		return -1;

	/* If pos is out of range, need NOT to split */
	if( pos >= map->bands[map->size-1].pos+map->bands[map->size-1].len ) /* == is the bottom */
		return 0;

	/* Get index of band, as pos refers */
	index=egi_colorBandMap_get_bandIndex(map, pos);

	/* If pos is start position of a band, need NOT to split */
	if( pos == map->bands[index].pos )
		return 0;

	/* Here: Need to split the band */

	/* Check capacity and growsize */
	if( map->size >= map->capacity ) {
		if( egi_mem_grow( (void **)&map->bands, map->capacity*sizeof(EGI_COLOR_BAND), COLORMAP_BANDS_GROW_SIZE*sizeof(EGI_COLOR_BAND)) !=0 ) {
			printf("%s: Fail to mem grow map->bands!\n", __func__);
			return -2;
		}
		else
			map->capacity += COLORMAP_BANDS_GROW_SIZE;
	}


	/* Set aside (index+1) for the new band */
	memmove( map->bands+index+2, map->bands+index+1, sizeof(EGI_COLOR_BAND)*(map->size-index-1));

	/* Insert the new band data */
	map->bands[index+1].pos=pos;
	map->bands[index+1].len=map->bands[index].pos+map->bands[index].len-pos; /* part of old band length */

	/* Modify the old band length, shorten. */
	map->bands[index].len=pos-map->bands[index].pos;

	/* Upate map->size */
	map->size++;

	return 0;
}


#if 0
/*----------------------------------------------------------------
Insert color band into the color map, and modify/delete map->bands
in order to keep color map consistent.
map->len will be modified if the new band exceeds its range.
TODO: A faster way!

@map:	An EGI_COLOR_BANDMAP pointer
@band:  An EGI_COLOR_BAND

Return:
	0	OK
	<0	Fails
----------------------------------------------------------------*/
int  egi_colorBandMap_updateBand(EGI_COLOR_BANDMAP *map, const EGI_COLOR_BAND *band)
{
	int i;
	unsigned int headIdx, endIdx; /* indice of bands, in which new band head_pos and end_pos are located. */

	if(map==NULL || map->bands==NULL || band==NULL)
		return -1;

	/* 0. Extand length of map ( last band ) to cover new band */
	/* Then headIdx/endIdx MUST be within the map range. */
	unsigned int len1=map->bands[map->size-1].pos+map->bands[map->size-1].len;
	unsigned int len2=band->pos+band->len;
	if( len2 > len2 )
		map->bands[map->size-1].len= len2;

	headIdx=egi_colorBandMap_get_bandIndex(map, band->pos);
	endIdx=egi_colorBandMap_get_bandIndex(map, band->pos+band->len-1);
	printf("%s: headIdx=%d, endIdx=%d\n", __func__, headIdx, endIdx);

	/* 1. If new band totally inside an old band, means old band is cut into TWO parts! */
	if( headIdx==endIdx ) {
		/* 1.1 If start_pos and len are both the same */
		if( band->pos==map->bands[headIdx]->pos && band->pos.len==map->bands[headIdx].len) {
			/* Just change the color */
			map->bands[headIdx]->color = band->color;
			return 0;
		}
		/* 1.2 If start pos is the same, end pos NOT the same, mean old band leaves tail part only. */
		else if(band->pos==map->bands[headIdx]->pos) {

		}

		return 0;
	}

	/* 2. ELSE If new band head PARTIALLY overlaps with an old band, means old band's some tail part is to be cut off */
	if( band->pos > map->bands[headIdx].pos ) { /* == means cover the old band head totally! */
		 /* Adjust old band length */
		 map->bands[headIdx].len= band->pos-map->bands[headIdx].pos;
	}



#if 0 ///////////////////////
	/* 1.Check band range and delete overlapped bands */
	for(i=0; i< map->size; i++){
		/* 1.1 If new band totally inside an old band, means old band is cut into TWO parts! */
		if( map->bands[i].pos < band->pos
		    && map->bands[i].pos+map->bands[i].len > band->pos+band->len ) {


			break;
		}

		/* 1.2 If new band head overlaps with tail of an old band, means old band's some tail part is to be cut off */
		if( map->bands[i].pos < band->pos
		    && map->bands[i].pos+map->bands[i].len > band->pos ) {

			/* Adjust old band */
			map->bands[i].len =

		}

		/* 1.3 If old band is totally inside the new band, delet the old band */

		/* 1.4 If new band's tail overlaps with head of and old band, means old band's some head part is to be cut off */

	}

	/* 2.Check capacity */

	/* 3.Inster the band */

	/* 4.Modify map->len if necessary */
#endif //////////////////////////////////////////////

}
#endif
