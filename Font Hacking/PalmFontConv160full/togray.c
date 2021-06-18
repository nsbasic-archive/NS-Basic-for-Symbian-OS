/*
 * $Id: togray.c,v 1.22 2004/12/04 20:48:56 arpruss Exp $
 *
 * The true home of this program is palmfontconv.sourceforge.net.  If you
 * find it elsewhere and improve it, please send a patch to
 * Alexander R. Pruss <ap85@georgetown.edu>.
 *
 * palmfontconv - Copyright (c) 2003, Alexander R. Pruss
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <assert.h>

#include "palmtypes.h"
#include "fontconv.h"

#define HIRES 144
#define LORES 72
#ifdef __ELF__
#define STRNICMP strncasecmp
#define STRICMP strcasecmp
#else
#define STRNICMP strnicmp
#define STRICMP stricmp
#endif

#define SWAPBYTES(x,y) { unsigned char __store; __store=(y); (y)=(x); (x)=__store; }

Word fontVersion;
Word fRectWidth;
Word fRectHeight;
Word ascent;
Word descent;
Word firstChar;
Word lastChar;
Word leading;
SWord minLeftKerning;
SWord maxRightOverhang;
char grayMap[257];
Boolean doreverse;
Boolean armVersion  = 0;
Boolean setVersion3 = 0;
Word   curGlyphNumber;
Word   curGlyphWidth;
SWord  curLeftKerning;
SWord  curAdvance;
Word   curBitmapResourceID = 1;
int    resourceNumber;
int    curVerbosity = 0;
char   resourceType[5]="\0\0\0\0\0";
char   workingDir[2048]="";


typedef struct {
     char*    name;
     int      orientation;  /* L = +1, R = -1, U = 0 */
     int      version;
     int      depth;
     Word     mask;
     Boolean  compress;
     int      compressFlagShift;
} BitmapFormat;

BitmapFormat formats[] = {
  { "GU14", 0,  1, 4, 1, 1, 0 },
  { "GU24", 0,  2, 4, 2, 1, 0 },
  { "GU34", 0,  3, 4, 4, 1, 0 },
  { "GL14", 1,  1, 4, 8, 1, 1 },
  { "GL24", 1,  2, 4, 16, 1, 1 },
  { "GL34", 1,  3, 4, 32, 1, 1 },
  { "GR14", -1, 1, 4, 64, 1, 2 },
  { "GR24", -1, 2, 4, 128, 1, 2 },
  { "GR34", -1, 3, 4, 256, 1, 2 }
};
#define NUM_FORMATS (sizeof formats / sizeof *formats)

#define FONTTYPEV1_MASKS ( 1 | 2 | 8 | 16 | 64 | 128 )
#define FONTTYPEV2_MASKS ( 4 | 32 | 256 )

#define GLYPH_FLAG_COMPRESSED   1

Word partLength[ NUM_FORMATS ];

typedef struct GlyphInfo_ {
    Word   glyphNumber;
    SWord  leftKerning;
    SWord  advance;
    Word   bitmapWidth;
    Word   resourceNumber;
//    Word   positionInResourceIndex;
    Word   bitmapSizes[ NUM_FORMATS ];
    struct GlyphInfo_* nextGlyph;
    void*  bitmaps[ NUM_FORMATS ];
    struct GlyphInfo_* duplicate;
    Byte   flags;
    Byte   bitmap[];
} GlyphInfo;

typedef struct {
    Word firstGlyph;   /* first glyph found in resource */
    Word lastGlyph;    /* last glyph found in resource */
    Word resourceID;   /* the ID of the resource */
    Word reserved;     /* should be zero for now */
} GrayFontBitmapsInfo;

typedef struct {
    Word  fontVersion;
    Word  firstChar;
    Word  lastChar;
    Word  fRectWidth;
    Word  fRectHeight;
    SWord ascent;
    SWord descent;
    SWord leading;
    Word  numberOfBitmapResources;
    Word  bitmapResourceTableOffset;
    Word  glyphInfoTableOffset;
    SWord minLeftKerning;
    SWord maxRightOverhang;
    Word  reserved[4];
} GrayFontType;

typedef struct {
    Word offset;
    Word length;
} GrayFontResourceIndexEntry;

typedef struct {
    SWord leftKerning;
    SWord advance;
    Word  bitmapWidth;
    Word  resourceNumber;
    Word  positionInResourceIndex;
    Byte  flags;
    Byte  reserved;
} GrayFontGlyphInfo;

GlyphInfo* firstGlyph = NULL;



typedef enum { LfontVersion=0, LfRectHeight,
     Lascent, Ldescent, Lleading, Lgraymap, LleftKerning, Ladvance,
     Lglyph, Lwidth } Label;

struct {
   char* fieldname;
   char* format;
   void* location;
   Byte have;
} values[] = {
               {"fontVersion","%hu",&fontVersion,0},
               {"fRectHeight","%hu",&fRectHeight,0},
               {"ascent", "%hd", &ascent, 0},
               {"descent", "%hd", &descent, 0},
               {"leading", "%hd", &leading, 0},
               {"graymap", "%s", grayMap, 0},
               {"leftKerning", "%hd", &curLeftKerning, 0 },
               {"advance", "%hd", &curAdvance, 0 },
               {"glyph", "%hu", &curGlyphNumber, 0 },
               {"width", "%hu", &curGlyphWidth, 0 }
};
#define NUM_VALUES (sizeof values / sizeof *values)


Label* forbiddenLabels = NULL;
int numForbiddenLabels;

Label forbiddenInGlyphPart[] = { LfontVersion, LfRectHeight,
      Lascent, Ldescent, Lleading };


void delete_glyphs( void )
{
    GlyphInfo* glyph;
    GlyphInfo* next;
    int i;
    glyph = firstGlyph;
    while ( glyph != NULL ) {
        next = glyph->nextGlyph;
        for ( i = 0 ; i < NUM_FORMATS ; i++ )
             if ( glyph->bitmaps[ i ] != NULL )
                 free( glyph->bitmaps[ i ] );
        free( glyph );
        glyph = next;
    }
}


void err(char *s, ...)
{
    va_list argptr;
    va_start(argptr,s);
    vfprintf(stderr,s,argptr);
    va_end(argptr);
    fputc('\n',stderr);
    delete_glyphs();
    exit(1);
}



void msg(int verbosity,char *s, ...)
{
    va_list argptr;
    va_start(argptr,s);
    if ( verbosity <= curVerbosity ) {
        vfprintf(stderr,s,argptr);
        fputc('\n',stderr);
    }
    va_end(argptr);
}



void reverse_word_array(void *p,unsigned n)
{
    unsigned char *s=p;
    if(!doreverse) return;
    n/=2;
    while(n--)
    {
        SWAPBYTES(s[0],s[1]);
        s+=2;
    }
}



Word reverse_word(Word w)
{
    return doreverse?(w<<8) | (w>>8):w;
}



DWord reverse_dword(DWord w)
{
    return doreverse?(w>>24)|(((w>>16)&0xff)<<8)|(((w>>8)&0xff)<<16)|(w<<24):w;
}



void setreverse(int c)
{
    /* on LSB first systems, set doreverse to c, otherwise
       set it to !c */
    Word w=0;
    *(char*)&w=1;
    if(w==1) doreverse=c; else doreverse=!c;
}


char *trim(char *s)
{
    char *t;
    t=strchr(s,'\n');
    if(t!=NULL) *t=0;
    t=strstr(s,"//");
    if(t!=NULL) *t=0;
    while(*s&&isspace(*s)) s++;
    if(!*s) return "";
    t=s+strlen(s)-1;
    while(*t&&isspace(*t)&&t>=s) *t--=0;
    if(t<s) return "";
    return s;
}


void *mymalloc(unsigned length)
{
    void *p;
    if ( length == 0 ) err("malloc(0) requested.");
    p=malloc(length);
    if(NULL==p) err("Out of memory: %u bytes requested.",length);
    return p;
}

FILE *myfopen(char *s,char *t)
{
    FILE *f=fopen(s,t);
    if(NULL==f)
        err("Error opening %s.\n",s);
    return f;
}


/* returns 1 if did something */
Boolean parseline(char *s)
{
    int i;
    int j;
    while(isspace(*s) && *s) s++;
    if(*s=='/') return 0;
    for(i=0;i<NUM_VALUES;i++)
    {
        if(!STRNICMP(values[i].fieldname,s,strlen(values[i].fieldname)))
        {
        	if ( forbiddenLabels != NULL )
                for ( j = 0 ; j < numForbiddenLabels ; j++ )
                     if ( i == forbiddenLabels[ j ] )
                         err( "`%s' forbidden in this context.\n",
                                values[ i ].fieldname );
            sscanf(s+strlen(values[i].fieldname),values[i].format,values[i].location);
            values[i].have=1;
            return 1;
        }
    }
    return 0;
}


void Rotate( int width, int height, int x, int y, int *rotx, int *roty, int orientation )
{
    switch( orientation ) {
        case 0:
            *rotx = x;
            *roty = y;
            return;
        case 1:
            *rotx = y;
            *roty = height - 1 - x;
            return;
        case -1:
            *rotx = width - 1 - y;
            *roty = x;
            return;
    }
}



Word ScanLineCompressRow( int y, Byte* out, Byte* in, Word rowBytes )
{
    Byte* startOut;
    Byte* endIn;
    Byte* prevLine;
    Byte* startIn;
    static Byte line[ 65536 ];
    startIn  = in;
    startOut = out;
    prevLine = line;
    endIn    = in + rowBytes;
    while ( in < endIn ) {
        Byte* octupleMarker;
        Byte mask;
        octupleMarker = out;
        *octupleMarker = 0;
        out++;
        for ( mask = 0x80 ; mask != 0 && in < endIn ; mask >>= 1 ) {
            if ( y == 0 || *in != *prevLine ) {
                *octupleMarker |= mask;
                *out++ = *in;
            }
            in++;
            prevLine++;
        }
    }
    memcpy( line, startIn, rowBytes );
    return out - startOut;
}



Boolean CompressBitData( Byte* bitmap, Word rowBytes, Word height,
    Word header, Word* size, Byte version, Byte* compressType )
{
    int   y;
    int   length;
    int   lengthDataSize;
    Byte* in;
    Byte* out;
    Byte* startOut;
    in       = bitmap;
    out      = mymalloc( 2 * rowBytes * height );
    startOut = out;
    for ( y = 0 ; y < height ; y++ ) {
        out += ScanLineCompressRow( y, out, in, rowBytes );
        in  += rowBytes;
    }
    length = out - startOut;
    *compressType = BitmapCompressionTypeScanLine;
    lengthDataSize = version < 3 ? 2 : 4;
    if ( lengthDataSize + length + header < *size ) {
        if ( lengthDataSize == 4 ) {
            * ( DWord* )bitmap = reverse_dword( lengthDataSize + length );
        }
        else {
            * ( Word* )bitmap = reverse_word( lengthDataSize + length );
        }
        memcpy( bitmap + lengthDataSize, startOut, length );
        free( startOut );
        *size = lengthDataSize + length + header;
        return 1;
    }
    free( startOut );
    return 0;
}



void MakeBitmaps( GlyphInfo* glyph, Word formatsToGenerate )
{
    int i;
    int width;
    int height;
    int header;
    int rowBits;
    int rowBytes;
    int x;
    int y;
    int rotx;
    int roty;
    int mod;
    Byte* bitData;
    Byte compressType;

    glyph->flags = 0;
    msg(3,"Dumping bitmap for character %u.",glyph->glyphNumber);
    for ( i = 0 ; i < NUM_FORMATS ; i++ ) {
         if ( ! ( formats[ i ].mask & formatsToGenerate ) ) {
             glyph->bitmaps[ i ] = NULL;
             continue;
         }
         if ( formats[ i ].orientation != 0 ) {
             width  = fRectHeight;
             height = glyph->bitmapWidth;
         }
         else {
             width  = glyph->bitmapWidth;
             height = fRectHeight;
         }

         if ( fontVersion == 3 )
             header = 0;
         else if ( formats[ i ].version < 3 )
             header = sizeof( BitmapTypeV1 );
         else
             header = sizeof( BitmapTypeV3 );

         rowBits    = width * formats[ i ].depth;
         rowBytes   = ( rowBits + 7 ) / 8;
         if ( rowBytes % 2 )
             rowBytes++;
         glyph->bitmapSizes[ i ] = header + rowBytes * height;

         glyph->bitmaps[ i ] = mymalloc( glyph->bitmapSizes[ i ] );
         memset( glyph->bitmaps[ i ], 0, glyph->bitmapSizes[ i ] );

         if ( fontVersion < 3 ) {
             BitmapTypeV2* p;
    
             p = glyph->bitmaps[ i ];
             p->width    = reverse_word( width );
             p->height   = reverse_word( height );
             p->rowBytes = reverse_word( rowBytes );
             p->pixelSize = formats[ i ].depth;
             p->version   = armVersion ? 0x83 : formats[ i ].version;
             if ( formats[ i ].version == 3 ) {
                 ( ( BitmapTypeV3* )p )->size    = sizeof( BitmapTypeV3 );
                 ( ( BitmapTypeV3* )p )->density = reverse_word( kDensityDouble );
            }
        }

        bitData = ( Byte* )( glyph->bitmaps[ i ] ) + header;
        mod = 8 / formats[ i ].depth;
        for ( x = 0 ; x < glyph->bitmapWidth ; x++ )
            for ( y = 0 ; y < fRectHeight ; y++ ) {
                int col;
                int shift;
                Rotate( width, height, x, y, &rotx, &roty, formats[ i ].orientation );
                col   = rotx / mod;
                shift = 8 - formats[ i ].depth - ( rotx % mod ) * formats[ i ].depth;
                bitData[ roty * rowBytes + col ] |=
                                         glyph->bitmap[ x + y * glyph->bitmapWidth ] << shift;
            }
        if ( formats[ i ].compress &&
                 CompressBitData( bitData, rowBytes, height, header,
                     &glyph->bitmapSizes[ i ], formats[ i ].version,
                     &compressType ) ) {
            if ( fontVersion < 3 ) {
                ( ( BitmapTypeV2* )( glyph->bitmaps[ i ] ) )->flags
                     = armVersion ? compressedLE : compressed;
                if ( 1 < formats[ i ].version )
                    ( ( BitmapTypeV2* )( glyph->bitmaps[ i ] ) )->compressionType
                                                                    = compressType;
            }
            else {
                glyph->flags = 1 << formats[ i ].compressFlagShift;
//                printf( " %d ", glyph->flags );
            }
        }
    }
}



void GrabGrayLine( Byte* bitmap, char* line, int width )
{
    while ( 0 < width ) {
        char* index;
        if ( ! *line ) break;
        index = strchr( grayMap, *line );
        if ( index == NULL )
            err( "Invalid character `%c' in gray map", *line);
        else
            *bitmap++ = index - grayMap;
        width--;
        line++;
    }
    while ( 0 < width-- )
        *bitmap++ = 0;
}



void FixDuplicate( GlyphInfo* curGlyph, Word height )
{
    GlyphInfo* glyph;
    glyph = firstGlyph;
    while ( glyph != curGlyph ) {
        if ( glyph->advance     == curGlyph->advance &&
             glyph->bitmapWidth == curGlyph->bitmapWidth &&
             glyph->leftKerning == curGlyph->leftKerning &&
             !memcmp( glyph->bitmap, curGlyph->bitmap, curGlyph->bitmapWidth * ( int )height ) ) {
            msg(3,"Character %d is a duplicate of %d.\n",curGlyph->glyphNumber,glyph->glyphNumber);
            curGlyph->duplicate = glyph;
            return;
        }
        glyph = glyph->nextGlyph;
    }
    curGlyph->duplicate = NULL;
}



int ReadTextFont( FILE* f, Word formatsToGenerate )
{
    int  numInIndex;
    int  indexLength;
    int  i;
    int  row;
    int  width;
    char line[3000];
    GlyphInfo* glyph;
    GlyphInfo* prevGlyph;
    Word resourceSize[ NUM_FORMATS ];

    forbiddenLabels = NULL;
    for ( i = 0 ; i < NUM_VALUES ; i++ )
         values[ i ].have = 0;
    while ( !feof( f ) && ! values[ Lglyph ].have ) {
         if ( NULL == fgets( line, 3000, f ) )
             break;
         parseline( line );
    }
    if ( feof( f ) )
        err("No glyphs in file!");
    if ( ! values[ Lgraymap ].have )
        return 1;
    if ( values[ Lascent ].have && values[ Ldescent ].have &&
         values[ LfRectHeight ].have ) {
        if ( ascent + descent != fRectHeight )
            err("ascent+descent must equal fRectHeight.");
    }
    else if ( values[ Lascent ].have && values[ Ldescent ].have )
         fRectHeight = ascent + descent;
    else if ( values[ Lascent ].have && values[ LfRectHeight ].have )
         descent = fRectHeight - ascent;
    else if ( values[ Ldescent ].have && values[ LfRectHeight ].have )
         ascent = fRectHeight - descent;
    else
         err("Must specify at least two of ascent, descent and fRectHeight");
    if ( ! values[ Lascent ].have )
         leading = 0;
    if ( ! values[ LfontVersion ].have )
         fontVersion = 2;
    if ( fontVersion != 1 && fontVersion != 2 && fontVersion != 3 )
         err("Only version 1-3 input fonts supported by this version of togray");
    if ( setVersion3 )
         fontVersion = 3;
    msg(2, "descent = %hu, ascent = %hu, fRectHeight = %hu", descent,ascent,fRectHeight );

    prevGlyph  = NULL;
    firstChar  = 0xFFFF;
    lastChar   = 0;
    fRectWidth = 0;
    resourceNumber = 1;
    numInIndex = 0;
    minLeftKerning   = 0;
    maxRightOverhang = 0;
    firstGlyph = NULL;
    for ( i = 0 ; i < NUM_FORMATS ; i++ )
         resourceSize[ i ] = 0;

    while ( !feof( f ) && values[ Lglyph ].have ) {
         Word lastGlyphNumber;
         for ( i = 0 ; i < NUM_VALUES ; i++ )
              values[ i ].have = 0;
         forbiddenLabels = forbiddenInGlyphPart;
         numForbiddenLabels = sizeof forbiddenInGlyphPart / sizeof *forbiddenInGlyphPart;
         row = 0;
         glyph = NULL;
         width = 0;
         lastGlyphNumber = curGlyphNumber;
         while ( ! values[ Lglyph ].have && !feof( f ) ) {
             if ( NULL == fgets( line, 3000, f ) )
                 break;
             if ( parseline( line ) )
                 continue;
             trim( line );
             if ( !*line )
                 continue;
             if ( fRectHeight <= row )
                 err ("Glyph %hu too tall.", lastGlyphNumber );
             if ( values[ Lwidth ].have && row == 0 ) {
                 width = curGlyphWidth;
                 values[ Lwidth ].have = 0;
             }
             else if ( values[ Lwidth ].have && 0 < row )
                 err( "width specifier in wrong place for glyph %hu.", lastGlyphNumber );
             else if ( row == 0 )
                 width = strlen( line );
             if ( glyph == NULL ) {
                 msg(3, "Glyph %hu, width %hu",lastGlyphNumber, width );
                 glyph = mymalloc( sizeof( GlyphInfo ) + width * fRectHeight );
                 if ( prevGlyph != NULL )
                     prevGlyph->nextGlyph = glyph;
                 prevGlyph = glyph;
                 memset( glyph, 0, sizeof( GlyphInfo ) + width * fRectHeight );
             }
             GrabGrayLine( glyph->bitmap + row * width, line, width );
             row++ ;

         }
         if ( row < fRectHeight )
             err( "Not enough data for glyph %u.", lastGlyphNumber );
         glyph->glyphNumber = lastGlyphNumber;
         if ( firstGlyph == NULL )
             firstGlyph = glyph;
         if ( lastGlyphNumber < firstChar )
             firstChar = lastGlyphNumber;
         if ( lastChar < lastGlyphNumber )
             lastChar  = lastGlyphNumber;
         if ( fRectWidth < width )
             fRectWidth = width;
         if ( values[ LleftKerning ].have ) {
             glyph->leftKerning = curLeftKerning;
             if ( curLeftKerning < minLeftKerning )
                 minLeftKerning = curLeftKerning;
         }
         else
             glyph->leftKerning = 0;
         if ( values[ Ladvance ].have )
             glyph->advance = curAdvance;
         else
             glyph->advance = width;
         if ( maxRightOverhang < width - glyph->advance + glyph->leftKerning )
             maxRightOverhang = width - glyph->advance + glyph->leftKerning;
         glyph->bitmapWidth = width;
         FixDuplicate( glyph, fRectHeight );
         if ( glyph->duplicate == NULL ) {
             MakeBitmaps( glyph, formatsToGenerate );
             numInIndex++;
             indexLength = numInIndex * sizeof( GrayFontResourceIndexEntry );
             for ( i = 0 ; i < NUM_FORMATS ; i++ )
                  if ( formats[ i ].mask & formatsToGenerate ) {
                      resourceSize[ i ] += ( glyph->bitmapSizes[ i ] + 3 )/4*4;
                      if ( 64000 < indexLength + resourceSize[ i ] )
                          break;
                  }
             if ( i < NUM_FORMATS ) {
                  msg(2,"Beginning new resource");
                  resourceNumber++;
                  for ( i = 0 ; i < NUM_FORMATS ; i++ )
                       resourceSize[ i ] = glyph->bitmapSizes[ i ];
                  numInIndex = 0;
             }
             glyph->resourceNumber = resourceNumber;
         }
         else {
             glyph->resourceNumber = glyph->duplicate->resourceNumber;
         }
         glyph->nextGlyph = NULL;
    }
    fclose( f );
    msg(1,"Produced bitmaps");
    return 0;
}


void align( FILE* f, int modulo )
{
    long pos = ftell( f );
    while ( pos % modulo != 0 ) {
        fputc( 0, f );
        pos++;
    }
}



void GenerateResources( Word GrFnResourceID, Word toGenerate )
{
    FILE*                f;
    char                 name[3000];
    GlyphInfo*           glyph;
    GlyphInfo*           firstGlyphOfResource;
    GlyphInfo*           lastGlyphOfResource;
    GrayFontBitmapsInfo* bitmapsInfo;
    GrayFontType         header;
    GrayFontGlyphInfo*   glyphList;
    int                  curResourceNumber;
    int                  inResource;
    GrayFontResourceIndexEntry*
                         resourceIndex;
    int                  i;
    int                  j;
    Boolean              done;

    msg(1,"Generating resources");
    bitmapsInfo = mymalloc( resourceNumber * sizeof( GrayFontBitmapsInfo ) );

    glyphList   = mymalloc( ( lastChar - firstChar + 1 ) * sizeof( GrayFontGlyphInfo ) );

    memset( glyphList, 0, ( lastChar - firstChar + 1 ) * sizeof( GrayFontGlyphInfo ) );

    header.fontVersion = fontVersion;
    header.firstChar   = firstChar;
    header.lastChar    = lastChar;
    header.fRectWidth  = fRectWidth;
    header.fRectHeight = fRectHeight;
    header.ascent      = ascent;
    header.descent     = descent;
    header.leading     = leading;
    header.minLeftKerning = minLeftKerning;
    header.maxRightOverhang = maxRightOverhang;
    header.numberOfBitmapResources
                       = resourceNumber;
    header.bitmapResourceTableOffset
                       = ( sizeof header + 3 )/4*4;
    header.glyphInfoTableOffset
                       = ( ( sizeof header ) + resourceNumber * sizeof( GrayFontBitmapsInfo ) + 3)/4*4;

    lastGlyphOfResource = firstGlyphOfResource = glyph = firstGlyph;
    curResourceNumber = 0;
    inResource        = 0;

    assert( glyph != NULL );

    for (;;) {
        done = ( glyph == NULL );
        if ( done || ( glyph->duplicate == NULL && glyph->resourceNumber != curResourceNumber ) ) {
            if ( 0 < curResourceNumber ) {
                if ( done )
                    msg(2,"Dumping final resource set");
                else
                    msg(2,"Dumping resource set %d",curResourceNumber);
                bitmapsInfo[ curResourceNumber-1 ].firstGlyph = firstGlyphOfResource->glyphNumber;
                bitmapsInfo[ curResourceNumber-1 ].lastGlyph = lastGlyphOfResource->glyphNumber;
                bitmapsInfo[ curResourceNumber-1 ].resourceID = curBitmapResourceID;
                bitmapsInfo[ curResourceNumber-1 ].reserved = 0;
                for ( i = 0 ; i < NUM_FORMATS ; i++ ) {
                     int pos;
                     if ( ! ( formats[ i ].mask & toGenerate ) )
                         continue;
                     sprintf(name, "%s%s%04X.bin", workingDir, formats[ i ].name, curBitmapResourceID );
                     msg(3,"Dumping resource file %s [%d items]",name,inResource);
                     f = myfopen(name, "wb");
                     resourceIndex = mymalloc( sizeof( *resourceIndex ) * inResource );
                     pos = sizeof( *resourceIndex ) * inResource;
                     glyph = firstGlyphOfResource;
                     for ( j = 0 ; j < inResource ; j++ ) {
                          while ( glyph->duplicate != NULL )
                              glyph = glyph->nextGlyph;
                          while ( pos % 4 ) pos++;
                          resourceIndex[ j ].offset = pos;
                          resourceIndex[ j ].length = glyph->bitmapSizes[ i ];
                          pos += glyph->bitmapSizes[ i ];
                          glyph = glyph->nextGlyph;
                     }
                     reverse_word_array( resourceIndex, sizeof( *resourceIndex ) * inResource );
                     fwrite( resourceIndex, sizeof( *resourceIndex ), inResource, f );
                     free( resourceIndex );
                     glyph = firstGlyphOfResource;
                     for ( j = 0 ; j < inResource ; j++ ) {
                          while ( glyph->duplicate != NULL )
                              glyph = glyph->nextGlyph;
                          align( f, 4 );
                          fwrite( glyph->bitmaps[ i ], 1, glyph->bitmapSizes[ i ], f );
                          glyph = glyph->nextGlyph;
                     }
                     fclose( f );
                }
                curBitmapResourceID ++;
                firstGlyphOfResource = glyph;
                if ( done )
                    break;
            }
            curResourceNumber = glyph->resourceNumber;
            msg(2,"Beginning resource set %hu at glyph %hu",glyph->resourceNumber,glyph->glyphNumber);
            inResource = 0;
        }
        i = glyph->glyphNumber - firstChar;
        if ( glyph->duplicate != NULL ) {
            int j;
            j = glyph->duplicate->glyphNumber - firstChar;
            glyphList[ i ] = glyphList[ j ];
        }
        else {
            glyphList[ i ].positionInResourceIndex = inResource++;
            glyphList[ i ].advance = glyph->advance;
            glyphList[ i ].bitmapWidth = glyph->bitmapWidth;
            glyphList[ i ].leftKerning = glyph->leftKerning;
            glyphList[ i ].resourceNumber = curResourceNumber;
            glyphList[ i ].flags          = glyph->flags;
        }
        lastGlyphOfResource = glyph;
        glyph = glyph->nextGlyph;
    }

    sprintf(name,"%s%s%04X.bin",workingDir,resourceType,GrFnResourceID );
    f = myfopen(name, "wb" );
    reverse_word_array( &header, sizeof header );
    fwrite( &header, sizeof header, 1, f );
    align( f, 4 );
    reverse_word_array( bitmapsInfo, ( sizeof *bitmapsInfo ) * resourceNumber );
    fwrite( bitmapsInfo, sizeof *bitmapsInfo, resourceNumber, f );
    align( f, 4 );
    reverse_word_array( glyphList, ( sizeof *glyphList ) * ( lastChar - firstChar + 1 ) );
    fwrite( glyphList, sizeof *glyphList, lastChar - firstChar + 1, f );
    fclose( f );
}



int main( int argc, char **argv )
{
    FILE *f;
    int i;
    int j;
    Word resourceNumber;
    Word formatsRequested;

    if ( argc < 4 )
        err("Usage:\n"
            "  %s [-opts] [MainResourceType1]hexResourceNumber1 filename1 bitmapFormat11 [bitmapFormat12 ... ] "
            "[- [MainResourceType2]hexResourceNumber2 filename2 bitmapFormat22 [...] ...]",
            argv[ 0 ]);
    i = 1;
    while( argv[ i ][ 0 ] == '-' ) {
        if ( !strncmp( argv[ i ], "-W", 2 ) ) {
            strcpy( workingDir, argv[ i ]+2 );
            if ( 0 < strlen( workingDir ) && workingDir[ strlen( workingDir ) - 1 ] != '/' )
                strcat( workingDir, "/");
            i++;
        }
        else if ( !strncmp( argv[ i ], "-#", 2 ) ) {
            sscanf( argv[ i ] + 2 , "%hx", &curBitmapResourceID );
            i++;
        }
        else if ( !strcmp( argv[ i ], "-a" ) ) {
            armVersion = 1;
            i++;
        }
        else if ( !strcmp( argv[ i ], "-3" ) ) {
            setVersion3 = 1;
            i++;
        }
        else
            i++;
    }

    setreverse( ! armVersion );

    while ( i < argc ) {
        char* fname;
        if ( 5 <= strlen( argv[ i ] ) ) {
            strncpy( resourceType, argv[ i ], 4 );
            sscanf( 4+argv[ i ], "%hx", &resourceNumber );
        }
        else {
            strcpy( resourceType, "GrFn" );
            sscanf( 4+argv[ i ], "%hx", &resourceNumber );
        }
        i++;
        fname = argv[ i ];
        f = myfopen( argv[ i ], "r" );
        i++;
        formatsRequested = 0;
        while ( i < argc && strcmp( argv[ i ], "-" ) ) {
            for ( j = 0 ; j < NUM_FORMATS; j++ )
                 if ( ! STRICMP( argv[ i ], formats[ j ].name ) ) {
                     formatsRequested |= formats[ j ].mask;
                     break;
                 }
            if ( j == NUM_FORMATS )
                err("Invalid bitmap format %s.\n", argv[ i ]);
            i++;
        }
        i++;
        if ( formatsRequested == 0 )
            err("No formats requested!\n");
        if ( ReadTextFont( f, formatsRequested ) ) {
            /* Not a grayscale font--use FontConv() */
            char name[1024];

            msg(1,"Dumping non-grayscale font %s.\n", fname );
            sprintf(name, "%s%s%04x.bin", workingDir, resourceType, resourceNumber );
            fclose( f );
            FontConv( fname, ( formatsRequested & FONTTYPEV2_MASKS ) ? "H" : "l", name,   
                NULL, NULL, NULL, NULL, NULL );
        }
        else {
            GenerateResources( resourceNumber, formatsRequested );
        }
    }
    delete_glyphs();
    return 0;
}


