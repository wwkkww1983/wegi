/*------------------------------------------------------------------
This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License version 2 as
published by the Free Software Foundation.


Midas Zhou
-------------------------------------------------------------------*/
#ifndef __EGI_FTSYMBOL_H__
#define __EGI_FTSYMBOL_H__

#include "egi_symbol.h"
#include <freetype2/ft2build.h>
#include <freetype2/ftglyph.h>
#include <freetype2/ftadvanc.h>
#include <arpa/inet.h>
#include FT_FREETYPE_H

typedef unsigned char   UFT8_CHAR;
typedef unsigned char * UFT8_PCHAR;
typedef wchar_t         EGI_UNICODE;

typedef struct FTsymbol_library EGI_FONTS;
struct FTsymbol_library {

	char ftname[256];

	/***	www.freetype.org/freetype2/docs
	 * " In multi-threaded applications it is easiest to use one FT_Library
	 *   object per thread, In case this is too cumbersome, a single FT_Library object
	 *   across threads is possible also, as long as a mutex lock is used around FT_New_Face
	 *   and FT_Done_Face. "
	 */
	FT_Library     library;		/* NOTE: typedef struct FT_LibraryRec_  *FT_Library */

	/* Regular type */
        FT_Face         regular;	/* NOTE: typedef struct FT_FaceRec_*  FT_Face */
	char 		*fpath_regular;

	/* Light type */
        FT_Face         light;
	char 		*fpath_light;

	/* Regular type */
        FT_Face         bold;
	char 		*fpath_bold;

	/* Special type */
        FT_Face         special;
	char 		*fpath_special;
};

/* To  buffer frequently used wchars  */
typedef struct FTsymbol_font_buffer  EGI_FONT_BUFFER;
struct FTsymbol_font_buffer {
	FT_Face face;	/* A face object in FreeType2 library, MUST be a ref. to an already loaded facetype in sys EGI_FONTS */
	int	fw; 	/* Font size */
	int	fh;

	/* TODO: lookup scatter/hash table */

	wchar_t unistart;		/* Start of wcode/unicode. */
	size_t  size;			/* Fontdata size, as of fontdata[], total number of wchars buffered, with glyphs/data */

	/* font image data */
	struct font_data {
		unsigned char	*alpha;  	/* as per slot->bitmap.buffer,symheight*ftwidth pixels */
		int		symheight;	/* as per slot->bitmap.rows, font height in pixels is bigger than bitmap.rows! */
		int		ftwidth;	/* as per slot->bitmap.width, ftwidth <= (slot->advance.x>>6) */
		int		advanceX;	/* as per advanceX = slot->advance.x>>6, OR self cooked width. */
	        /* bitmap position relative to boundary box */
        	int delX;			/* = slot->bitmap_left; */
        	int delY; 			/* = -slot->bitmap_top + fh */
	} *fontdata;				/* index 0 --> unistart, 1 --> unistart+1, ... size-1 --> unistart+size-1 */

};

EGI_FONT_BUFFER* FTsymbol_create_fontBuffer(FT_Face face, int fw, int fh, wchar_t unistart, size_t size);
void FTsymbol_free_fontBuffer(EGI_FONT_BUFFER **fontbuff);

extern EGI_SYMPAGE sympg_ascii;  /* default font  LiberationMono-Regular */

extern EGI_FONTS   egi_sysfonts; /* system font set */
extern EGI_FONTS   egi_appfonts; /* APP font set */


void	FTsymbol_set_TabWidth( float factor);
void	FTsymbol_set_SpaceWidth( float factor);
int 	FTsymbol_load_library( EGI_FONTS *symlib );
FT_Face FTsymbol_create_newFace( EGI_FONTS *symlib, const char *ftpath);
void 	FTsymbol_release_library( EGI_FONTS *symlib );
int	FTsymbol_load_allpages(void);
void 	FTsymbol_release_allpages(void);
int  	FTsymbol_load_sysfonts(void);
int  	FTsymbol_load_appfonts(void);
void	FTsymbol_release_allfonts(void);
int  	FTsymbol_load_asciis_from_fontfile( EGI_SYMPAGE *symfont_page, const char *font_path, int Wp, int Hp );
int 	FTsymbol_get_symheight(FT_Face face, const unsigned char *pstr, int fw, int fh );
int 	FTsymbol_get_FTface_Height(FT_Face face, int fw, int fh);
int 	FTsymbol_uft8string_getAdvances(FT_Face face, int fw, int fh, const unsigned char *ptr); /* Need check and test */
int 	FTsymbol_cooked_charWidth(wchar_t wcode, int fw);
void 	FTsymbol_unicode_print(wchar_t wcode);
void 	FTsymbol_unicode_writeFB(FBDEV *fb_dev, FT_Face face, int fw, int fh, wchar_t wcode, int *xleft,
				int x0, int y0, int fontcolor, int transpcolor,int opaque);

int  	FTsymbol_unicstrings_writeFB( FBDEV *fb_dev, FT_Face face, int fw, int fh, const wchar_t *pwchar,
			       unsigned int pixpl,  unsigned int lines,  unsigned int gap,
                               int x0, int y0,
			       int fontcolor, int transpcolor, int opaque );

int  	FTsymbol_uft8strings_writeFB( FBDEV *fb_dev, FT_Face face, int fw, int fh, const unsigned char *pstr,
			       unsigned int pixpl,  unsigned int lines,  unsigned int gap,
                               int x0, int y0,
			       int fontcolor, int transpcolor, int opaque,
 			       int *cnt, int *lnleft, int* penx, int* peny );

int  	FTsymbol_uft8strings_pixlen( FT_Face face, int fw, int fh, const unsigned char *pstr); /* FTsymbol_uft8string_getAdvances() will call it.*/


#endif
int     FTsymbol_uft8strings_writeFB( FBDEV *fb_dev, FT_Face face, int fw, int fh, const unsigned char *pstr,
                               unsigned int pixpl,  unsigned int lines,  unsigned int gap,
                               int x0, int y0,
                               int fontcolor, int transpcolor, int opaque,
                               int *cnt, int *lnleft, int* penx, int* peny );

