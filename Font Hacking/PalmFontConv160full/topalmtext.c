/*
 * $Id: topalmtext.c,v 1.54 2005/04/30 00:47:38 arpruss Exp $
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


/* Needs FreeType 2 library. */

#include <ctype.h>
#include <stdio.h>
#include <ft2build.h>
#include <freetype/ftglyph.h>
#include <freetype/ttunpat.h>
#include FT_FREETYPE_H
#include "encodings.c"
#include "fontconv.h"

#define MAX_WIDTH  1000
#define MAX_HEIGHT 1000

#ifdef __ELF__
#define STRNICMP strncasecmp
#define STRICMP strcasecmp
#else
#define STRNICMP strnicmp
#define STRICMP stricmp
#endif

#define MAX_ADJUSTMENTS 256

FT_Render_Mode grayMode = FT_RENDER_MODE_NORMAL;

int kerningAdjustments[ MAX_ADJUSTMENTS ];
int advanceAdjustments[ MAX_ADJUSTMENTS ];
int verticalAdjustments[ MAX_ADJUSTMENTS ];

typedef enum {
    false = 0,
    true  = 1
} Boolean;
typedef unsigned char Byte;

Boolean forceWidthEven     = false;
Boolean forceHeightEven    = false;
Boolean skip                = false;
int     forceWidth;
int     forceHeight;
int     lastGlyphWidth;
int     lastGlyphHeight;
int     trimTop;
int     trimBottom;

Boolean gray     = false;
Boolean msHebrew = false;
FT_Bitmap  canvas;

const char grayMap[17] =  "-123456789ABCDE#";
#define MONO_1 ( grayMap[15] )
#define MONO_0 ( grayMap[0] )

#define START_CHAR 0
#define END_CHAR   1

char*       encoding    = "palmLatin";
unsigned encodingNum;
Boolean  disableKerning = false;
Boolean  isGreekKeys    = false;
Boolean  isPBHebrew     = false;
Boolean  fakeCGJ        = false;
FT_Library  library;   /* handle to library     */

Byte bitmapData[ MAX_WIDTH * MAX_HEIGHT ];

void SetEncoding( char* name )
{
    int i;
    isGreekKeys = !STRICMP( name, "GreekKeys" );
    isPBHebrew  = !STRICMP( name, "PBPlus_Hebrew" );
	for ( i = sizeof encodings / sizeof *encodings - 1 ; 0 <= i ; i-- ) {
	     if ( !STRICMP( encodings[ i ].name, name ) )
	         break;
	}
    if ( i < 0 )
         i = NULL_ENCODING;
    encodingNum = i;
}



unsigned Decode( FT_Face face, unsigned c )
{
    unsigned i;
    fakeCGJ = false;
    if ( encodingNum == NULL_ENCODING )
        return c;
    for ( i = 0 ; i < encodings[ encodingNum ].numEntries ; i++ ) {
         if ( encodings[ encodingNum ].table[ i ].src == c ) {
             long value = encodings[ encodingNum ].table[ i ].unicode;
             if ( value == UNDEFINED_CHAR )
                 return c;
             else {
                 if ( isPBHebrew && value == CGJ ) {
                     fakeCGJ = true;
                     return FT_Get_Char_Index( face, ' ' );
                 }
                 if ( isPBHebrew && msHebrew && ( value & EXTENSION ) ) {
                     unsigned int alefIndex;
                     alefIndex = FT_Get_Char_Index( face, HEBREW_ALEF );
                     switch( value ) {
                         case HEBREW_SHIN_SIN | HEBREW_SHIN_DOT:
                             return MS_HEBREW_SHIN - MS_HEBREW_ALEF + alefIndex;
                         case HEBREW_SHIN_SIN | HEBREW_SIN_DOT:
                             return MS_HEBREW_SIN - MS_HEBREW_ALEF + alefIndex;
                         case HEBREW_SHIN_SIN | HEBREW_SHIN_DOT | HEBREW_DAGESH:
                             return MS_HEBREW_SHIN_DAGESH - MS_HEBREW_ALEF + alefIndex;
                         case HEBREW_SHIN_SIN | HEBREW_SIN_DOT | HEBREW_DAGESH:
                             return MS_HEBREW_SIN_DAGESH - MS_HEBREW_ALEF + alefIndex;
                         case HEBREW_KAF_FINAL | HEBREW_KAMATZ:
                             return MS_HEBREW_KAF_FINAL_KAMATZ - MS_HEBREW_ALEF + alefIndex;
                         case HEBREW_KAF_FINAL | HEBREW_SHVA:
                             return MS_HEBREW_KAF_FINAL_SHVA - MS_HEBREW_ALEF + alefIndex;
                         case HEBREW_MEM | HEBREW_DAGESH:
                             return MS_HEBREW_MEM_DAGESH - MS_HEBREW_ALEF + alefIndex;
                         case HEBREW_NUN | HEBREW_DAGESH:
                             return MS_HEBREW_NUN_DAGESH - MS_HEBREW_ALEF + alefIndex;
                         case HEBREW_SAMEK | HEBREW_DAGESH:
                             return MS_HEBREW_SAMEK_DAGESH - MS_HEBREW_ALEF + alefIndex;
                         default:
                             if ( ( HEBREW_ALEF | HEBREW_DAGESH ) <= value &&
                                  value <= ( HEBREW_ZAYIN | HEBREW_DAGESH ) ) {
                                 return ( value - ( HEBREW_ALEF | HEBREW_DAGESH ) ) +
                                            MS_HEBREW_ALEF_DAGESH - MS_HEBREW_ALEF + alefIndex;
                             }
                             else if ( ( HEBREW_TET | HEBREW_DAGESH ) <= value &&
                                  value <= ( HEBREW_LAMED | HEBREW_DAGESH ) ) {
                                 return ( value - ( HEBREW_TET | HEBREW_DAGESH ) ) +
                                            MS_HEBREW_TET_DAGESH - MS_HEBREW_ALEF + alefIndex;
                             }
                             else if ( ( HEBREW_PE_FINAL | HEBREW_DAGESH ) <= value &&
                                  value <= ( HEBREW_SAMEK | HEBREW_DAGESH ) ) {
                                 return ( value - ( HEBREW_PE_FINAL | HEBREW_DAGESH ) ) +
                                            MS_HEBREW_PE_FINAL_DAGESH - MS_HEBREW_ALEF + alefIndex;
                             }
                             else if ( ( HEBREW_TSADE | HEBREW_DAGESH ) <= value &&
                                  value <= ( HEBREW_TAV | HEBREW_DAGESH ) ) {
                                 return ( value - ( HEBREW_TSADE | HEBREW_DAGESH ) ) +
                                            MS_HEBREW_TSADE_DAGESH - MS_HEBREW_ALEF + alefIndex;
                             }
                             else
                                  return FT_Get_Char_Index( face, value & 0xFFFFu );
                     }
                 }
                 return FT_Get_Char_Index( face, value & 0xFFFFu );
             }
         }
    }
    return c;
}



Byte GetRowPixel( Byte* row, int pixel, int width, int bold, Boolean gray )
{
    int i;
    Byte maxValue;

    if ( pixel - bold < 0 ) {
        i = 0;
    }
    else {
        i = pixel - bold;
    }
    maxValue = 0;
    for ( ; i <= pixel && i < width - bold ; i++ ) {
        Byte value;
        if ( gray )
            value = row[ i ] / 16;
        else
            value = ( row[ i / 8 ] >> ( 7 - ( i & 7 ) ) ) & 1;
        if ( maxValue < value )
            maxValue = value;
    }
    return maxValue;
}



void BlankLine( int size )
{
    if ( skip )
        return;
    while ( 0 < size-- )
        putchar( '-' );
    putchar( '\n' );
}


void EmitChar( char c )
{
    static int x, y;

    if ( c == START_CHAR ) {
        x = y = 0;
        lastGlyphWidth = 0;
        lastGlyphHeight = 0;
        return;
    }
    else if ( c == END_CHAR ) {
        if ( forceHeightEven && y % 2 ) {
            y++;
            BlankLine( lastGlyphWidth );
        }
        for ( ; y < forceHeight ; y++ ) {
            BlankLine( lastGlyphWidth );
        }
        lastGlyphHeight = y;
        x = y = 0;
        if ( ! skip )
            putchar( '\n' );
        return;
    }
    if ( forceHeight && forceHeight <= y )
        return;
    if ( c == '\n' ) {
        if ( forceWidthEven && x % 2 ) {
            x++;
            if ( ! skip )
                putchar( '-' );
        }
        for ( ; x < forceWidth ; x++ )
             if ( ! skip )
                 putchar( '-' );
        if ( ! skip )
            putchar( '\n' );
        lastGlyphWidth = x;
        x = 0;
        y++;
    }
    else if ( ! forceWidth || x < forceWidth ) {
        if ( ! skip )
            putchar( c );
        x++;
    }
}



void DumpRow( int left0, int right0, Byte* row, int width0, int leftSkip0, Boolean gray, int multiplicity, int bold )
{
    int pixel;
    int left;
    int right;
    int width;
    int leftSkip;

    while( 0 < multiplicity-- ) {
        left     = left0;
        right    = right0;
        width    = width0;
        leftSkip = leftSkip0;
        pixel    = 0;

        while( 0 < left-- )
            EmitChar( gray ? grayMap[ 0 ] : MONO_0 );
        while( 0 < width-- ) {
            if ( leftSkip <= 0 ) {
                Byte value;
                value = GetRowPixel( row, pixel, width0, bold, gray );
                if ( ! gray )
                    EmitChar( value ? MONO_1 : MONO_0 );
                else
                    EmitChar( grayMap[ value ] );
            }
            else
                leftSkip--;
            pixel++;
        }
        while( 0 < right-- )
            EmitChar( MONO_0 );
    }
    EmitChar('\n');
}


int EmitBitmap( int n, FT_Bitmap* bitmap, int left, int xadvance, int top, int height, int shift, int multiplicity,
    Boolean getWidth, Boolean forceNarrow, Boolean bold )
{
    int            i;
    int            j;
    int            y;
    unsigned char* buffer;
    int            right;
    Boolean        gray;
    int            blankColsLeft;
    int            rightMostCol;
    int            leftSkip;
    int            leftKerning;
    int            advance;
    int            noKerning;
    int            actualWidth;

    gray = ( bitmap->pixel_mode == ft_pixel_mode_grays );

    if ( fakeCGJ ) {
        xadvance = 1;
        left     = 0;
        bitmap->width = 1;
    }

    if ( 1 < multiplicity )
        noKerning = 1;
    else
        noKerning = disableKerning;
    actualWidth    = bitmap->width + bold;
    xadvance      += bold;

    if ( ! getWidth && ! skip )
        printf("GLYPH %d\n// left %d, top %d, height %d, xadvance %d, width %d\n",
            n,left,top,height,xadvance,actualWidth);
    y = 0;
    blankColsLeft  = actualWidth;
    rightMostCol   = -1;
    if ( bitmap->pitch < 0 )
        buffer = bitmap->buffer + ( bitmap->rows - 1 ) * bitmap->pitch;
    else
        buffer = bitmap->buffer;
    for ( i = 0 ; i < bitmap->rows ; i++, buffer += bitmap->pitch ) {
        for ( j = 0 ; j < actualWidth ; j++ ) {
            if ( 0 != GetRowPixel( buffer, j, actualWidth, bold, gray ) ) {
                if ( j < blankColsLeft )
                    blankColsLeft = j;
                if ( rightMostCol < j )
                    rightMostCol = j;
            }
        }
    }
    if ( forceNarrow ) {
        if ( rightMostCol < blankColsLeft ) {
            blankColsLeft = 0;
            rightMostCol  = -1;
        }
        leftSkip    = blankColsLeft;
        xadvance    = advance = rightMostCol - blankColsLeft + 1;
        actualWidth = rightMostCol + 1;
        left = right = 0;
        if ( getWidth )
            return xadvance * multiplicity;
    }
    else {
        if ( left < 0 ) {
            if ( blankColsLeft < -left ) {
                leftSkip = blankColsLeft;
            }
            else {
                leftSkip = -left;
            }
            left += leftSkip;
        }
        else {
            leftSkip = 0;
        }
        if ( ! noKerning ) {
            if ( left <= 0 ) {
                leftKerning = left;
                advance     = xadvance;
            }
            else {
                leftKerning = 0;
                advance     = xadvance;
            }
            if ( ! getWidth && ! skip && ! disableKerning && gray )
                printf( "leftKerning %d\n"
                    "advance     %d\n",
                    leftKerning,
                    advance );
        }
        if ( left < 0 ) {
            xadvance -= left;
            left      = 0;
        }
        if ( xadvance < left + actualWidth - leftSkip ) {
            xadvance = left + actualWidth - leftSkip;
            right    = 0;
        }
        else {
            right = xadvance - ( left + actualWidth - leftSkip );
        }
        if ( getWidth )
            return xadvance * multiplicity;
    }
    if ( ! skip )
        printf("//Processed: xadvance = %d, leftSkip = %d, left = %d, right = %d\n",
            xadvance,leftSkip,left,right);
    EmitChar( START_CHAR );
    while ( 0 < shift-- && y < height ) {
        DumpRow( xadvance, 0, NULL, 0, 0, gray, multiplicity, bold );
        y++;
    }
    for ( i = 0 ; i < top && y < height ; i++ ) {
        if ( 0 <= shift++ ) {
            DumpRow( xadvance, 0, NULL, 0, 0, gray, multiplicity, bold );
            y++;
        }
    }
    if ( bitmap->pitch < 0 )
        buffer = bitmap->buffer + ( bitmap->rows - 1 ) * bitmap->pitch;
    else
        buffer = bitmap->buffer;
    for ( i = 0 ; i < bitmap->rows && y < height ; i++, buffer += bitmap->pitch ) {
        if ( 0 <= shift++ ) {
            DumpRow( left, right, buffer, actualWidth, leftSkip, gray, multiplicity, bold );
            y++;
        }
    }
    while ( y < height ) {
        if ( 0 <= shift++ ) {
            DumpRow( xadvance, 0, NULL, 0, 0, gray, multiplicity, bold );
            y++;
        }
    }
    EmitChar( END_CHAR );
    return xadvance * 3;
}



int CheckNarrow( int c )
{
    return ( isGreekKeys && 128 <= c && c <= 139 ) ||
           ( isPBHebrew && ( ( 0xB7 <= c && c <= 0xC2 ) || c == 0xFE ) );
}


Boolean LoadFace( FT_Face* faceP, char* name, int pixelSize, Boolean dumpInfo )
{
    FT_Face face;
    int     scalable;
    FT_Open_Args args;
    FT_Parameter param;

    param.tag  = FT_PARAM_TAG_UNPATENTED_HINTING;
    param.data = NULL;

    args.flags = FT_OPEN_PATHNAME | FT_OPEN_PARAMS;

    args.num_params = 1;
    args.params     = &param;

    args.pathname   = name;

    if ( FT_Open_Face( library,
                       &args,
                       0,
                       &face ) ) {
        if ( FontConv( name, "t", "-", NULL, NULL, NULL, NULL, NULL ) ) {
            FILE* f = fopen( name, "r" );
            int   c;
            if ( f == NULL ) {
                fprintf(stderr,"Cannot open font file %s.\n", name );
                exit( 3 );
            }
            while ( -1 != ( c = fgetc( f ) ) )
                putchar( c );
            fclose( f );
        }
        exit( 0 );
    }

    if ( ! STRICMP( name + strlen( name ) - 4, ".pfa" ) ||
         ! STRICMP( name + strlen( name ) - 4, ".pfb" ) ) {
        char    attachName[5000];
        strcpy( attachName, name );
        strcpy( attachName + strlen( name ) - 3, "afm" );
        if ( FT_Attach_File( face, attachName ) ) {
            printf( "// (no %s file)\n", attachName );
        }
    }

    scalable = FT_IS_SCALABLE( face );

    if ( scalable ) {
        FT_Set_Pixel_Sizes( face, 0, pixelSize );
    }

    if ( FT_Select_Charmap( face, FT_ENCODING_UNICODE ) ) {
        SetEncoding( "none" );
        if ( FT_Select_Charmap( face, ft_encoding_latin_1 ) ) {
            if ( face->num_charmaps == 0 ) {
                if ( dumpInfo )
                    printf("// No charmap!\n");
            }
            else {
                if ( dumpInfo )
                    printf("// Using first charmap of %d.\n", face->num_charmaps);
                FT_Set_Charmap( face, face->charmaps[ 1 ] );
            }
        }
        else {
            if ( dumpInfo )
                printf("// Setting encoding to 8859-1\n");
        }
    }
    else {
        SetEncoding( encoding );
        if ( dumpInfo )
            printf("// Setting encoding to %s\n", encoding);
    }

    *faceP = face;
    return scalable;
}


void GetFaceMetrics( FT_Face face, int* highestTopP, int* lowestBottomP, Boolean gray, Boolean forceL1Chars )
{
    int highestTop;
    int lowestBottom;
    int i;

    highestTop   = 0;
    lowestBottom = 0;
    for ( i = 0 ; i < 256 ; i++ ) {
        /* load glyph image into the slot (erase previous one) */
        int c;
        if ( forceL1Chars && i == 0xA0 )
            c = ' ';
        else if ( forceL1Chars && i == 0x85 )
            c = '.';
        else
            c = i;
        if ( 0 == Decode( face, c ) ||
             FT_Load_Glyph( face, Decode( face, c ),
                    FT_LOAD_RENDER | ( gray ? grayMode : FT_LOAD_TARGET_MONO ) ) )
            continue;
        if ( highestTop < face->glyph->bitmap_top ) {
            highestTop = face->glyph->bitmap_top;
        }
        if ( face->glyph->bitmap_top -  face->glyph->bitmap.rows < lowestBottom ) {
            lowestBottom = face->glyph->bitmap_top - face->glyph->bitmap.rows;
        }
    }
    *highestTopP   = highestTop;
    *lowestBottomP = lowestBottom;
}


void HandleAdjustment( int* adj, char* s )
{
    unsigned c1;
    unsigned c2;
    int      delta;
    if ( s[ 0 ] == '/' ) {
        /*  0123     012345 */
        /*  /c/:  or /c-c/: */
        if ( s[ 2 ] == '/' ) {
            c1 = ( unsigned char )s[ 1 ];
            c2 = c1;
            s += 4;
        }
        else {
            c1 = ( unsigned char )s[ 1 ];
            c2 = ( unsigned char )s[ 3 ];
            s += 6;
        }
    }
    else {
        char* colon;
        char* dash;

        colon = strchr( s, ':' );
        if ( colon == NULL )
            return;
        dash  = strchr( s, '-' );
        if ( dash == NULL || colon < dash ) {
            c1 = atoi( s );
            c2 = c1;
        }
        else {
            c1 = atoi( s );
            c2 = atoi( dash + 1 );
        }
        s = colon + 1;
    }

    if ( ! *s )
        return;
    delta = atoi( s );

    while ( c1 <= c2 ) {
        adj[ c1 ] = delta;
        c1++;
    }
}



unsigned char GetPixel( FT_Bitmap* bitmap, int x, int y )
{
    if ( y < 0 || bitmap->rows <= y || x < 0 || bitmap->width <= x )
        return 0;
    if ( bitmap->pixel_mode == ft_pixel_mode_grays ) {
        return bitmap->buffer[ x + y * bitmap->pitch ];
    }
    else {
        return ( bitmap->buffer[ ( x >> 3 ) + y * bitmap->pitch ] >> ( 7 - ( x & 7 ) ) ) & 1;
    }
}



void PutPixel( FT_Bitmap* bitmap, int x, int y, unsigned char value )
{
    int delta;
    
    if ( y < 0 || bitmap->rows <= y || x < 0 || bitmap->width <= x )
        return;
        
    if ( bitmap->pitch < 0 )
        delta = ( bitmap->rows - 1 ) * -bitmap->pitch;
    else
        delta = 0;

    if ( bitmap->pixel_mode == ft_pixel_mode_grays ) {
        bitmap->buffer[ delta + x + y * bitmap->pitch ] = value;
    }
    else {
        unsigned char* p;
        unsigned char  mask;
        p = bitmap->buffer + delta + ( x >> 3 ) + y * bitmap->pitch;

        mask = 1 << ( 7 - ( x & 7 ) );

        if ( value )
            *p |= mask;
        else
            *p &= ~mask;
    }
}



void WriteBitmap( FT_Bitmap* outBitmap, int x0, int y0, int maxHeight, FT_Bitmap* inBitmap )
{
    int x;
    int y;

    for ( x = 0 ; x < inBitmap->width ; x++ )
        for ( y = 0 ; y < inBitmap->rows ; y++ ) {
             unsigned char value, value2;
             value  = GetPixel( outBitmap, x0 + x, y0 + y );
             value2 = GetPixel( inBitmap, x, y );
             if ( value < value2 )
                 value = value2;
             PutPixel( outBitmap, x0 + x, y0 + y, value );
        }
}



void EmitChars( FT_Face face, int* list, int highestTop, int height, int vshift, Boolean rtl,
    int* leftP, int* advanceP )
{
    int  minKern;
    int  maxRight;
    int  x;
    int* glyphNum;
    int  advance;
    int  i;

    minKern  = 0;
    maxRight = 0;

    x        = 0;

    glyphNum = list;

    for ( i = 0 ; i < MAX_HEIGHT * MAX_WIDTH ; i++ ) {
        bitmapData[ i ] = 0;
    }

    while ( -1 != *glyphNum ) {
        FT_Load_Glyph( face, *glyphNum, FT_LOAD_RENDER | ( gray ? grayMode : FT_LOAD_TARGET_MONO ) );
        if ( face->glyph->bitmap_left + x < minKern )
            minKern = face->glyph->bitmap_left + x;
        if ( maxRight < face->glyph->bitmap.width + x )
            maxRight = face->glyph->bitmap.width + x;
        if ( ! FT_HAS_KERNING( face ) || glyphNum[ 1 ] == -1 ) {
            x += ( face->glyph->advance.x + 63 ) / 64;
        }
        else {
            FT_Vector kerning;
            FT_Error  err;

            err = FT_Get_Kerning( face, FT_Get_Char_Index( face, glyphNum[ 0 ] ),
                      FT_Get_Char_Index( face, glyphNum[ 1 ] ), FT_KERNING_UNSCALED, &kerning );

            if ( err ) {
                kerning.x = 0;
            }

            x += ( face->glyph->advance.x + kerning.x + 63 ) / 64;
//            printf ( "%d %ld %ld\n", err, kerning.x, kerning.y );
        }
        glyphNum++;
    }

    advance  = x;

    x        = -minKern;
    glyphNum = list;

    canvas.rows  = height;
    canvas.width = maxRight;
    canvas.pitch = MAX_WIDTH;
    canvas.buffer = ( unsigned char* )bitmapData;
    canvas.num_grays = gray ? 128 : 1;
    canvas.pixel_mode = gray ? ft_pixel_mode_grays : ft_pixel_mode_mono;

    while ( -1 != *glyphNum ) {

        FT_Load_Glyph( face, *glyphNum, FT_LOAD_RENDER | ( gray ? grayMode : FT_LOAD_TARGET_MONO ) );

        WriteBitmap( &canvas, x + face->glyph->bitmap_left, highestTop - face->glyph->bitmap_top + vshift,
            height, &( face->glyph->bitmap ) );
        if ( ! FT_HAS_KERNING( face ) || glyphNum[ 1 ] == -1 ) {
            x += ( face->glyph->advance.x + 63 ) / 64;
        }
        else {
            FT_Vector kerning;
            FT_Error  err;

            err = FT_Get_Kerning( face, FT_Get_Char_Index( face, glyphNum[ 0 ] ),
                      FT_Get_Char_Index( face, glyphNum[ 1 ] ), FT_KERNING_UNSCALED, &kerning );

            if ( err ) {
                kerning.x = 0;
            }
            else {
                printf ( "kern %x %x : %ld\n", glyphNum[ 0 ], glyphNum[ 1 ], kerning.x );
            }

            x += ( face->glyph->advance.x + kerning.x + 63 ) / 64;

//            x += ( face->glyph->advance.x + 63 ) / 64;
//            x += kerning.x;
        }

        glyphNum++;
    }

    *leftP    = -minKern;
    *advanceP = x;
}




int main( int argc, char** argv )
{
    FT_Face     face;      /* handle to face object */
    int         i;
    int         pixelSize;
    int         highestTop;
    int         lowestBottom;
    int         argIndex;
    int         bold;
    int         shift;
    char*       match;
    Boolean     scalable;
    Boolean     forceL1Chars;
    Boolean     forceEven;
    int         forceHighestTop;
    int         forceLowestBottom;
    Boolean     forceGraySetting   = false;
    Boolean     doForceGraySetting = false;
    Boolean     forceBW            = false;
    Boolean     halfSize           = false;
    int         actualHeight;
    int         height;
    int         ascent;
    int         descent;
    int         evenSpace;

    if ( argc < 2 ) {
        fprintf(stderr,"Usage:\n  %s [-optons] [-encoding] [-mfontname2] [-bN] fontname [pixelSize [gray]]\n"
                       "   -f : force 0xA0 to be a non-breaking space and 0x85 to be ellipsis.\n"
                       "   -e : make an attempt to scale to even width output.\n"
                       "   -E : brute force even width/height output.\n"
                       "   -V : brute force even height output.\n"
                       "   -s : brute force even width of space glyph.\n"
                       "   -B : emit black and white version but with grayscale metrics.\n"
                       "   -H : half-size black and white.\n"
                       "   -encoding  : set the encoding if the input font is unicode.\n"
                       "   -mfontname : match the baseline of fontname2.\n"
                       "   -n         : disable kerning.\n"
                       "   -ax:y      : adjust advance for x by y.\n"
                       "   -kx:y      : adjust kerning for x by y.\n"
                       "   -vx:y      : adjust vertical position for x by y.\n"
                       "   -Tx        : trim top by x.\n"
                       "   -Ux        : trim bottom by x.\n"
                       "   -g  : enable grayscale, ignoring \"gray\" parameter setting.\n"
                       "   -h  : enable Hebrew hack for MS TTF fonts.\n"
                       "   -bN : set the number of bold pixels to N.\n\n"
                       "The x parameter in the -a, -k and -v options can be:\n"
                       "  a     : decimal number\n"
                       "  a-b   : where a and b are decimal numbers, to specify a range\n"
                       "  /c/   : where c is a literal character\n"
                       "  /c-d/ : where c and d are literal characters, to specify a range\n"
                        ,argv[0]);
        fprintf(stderr,"The following encodings are supported:\n");
        for ( i = 0 ; i < NUM_ENCODINGS ; i++ )
             fprintf(stderr," %s",encodings[i].name);
        fprintf(stderr,"\n");
        return 5;
    }

    if ( FT_Init_FreeType( &library ) ) {
        fprintf(stderr,"Cannot initialize FreeType library.\n");
        return 4;
    }

    forceEven = false;

    forceL1Chars = false;

    bold = false;

    match = NULL;

    argIndex = 1;
    
    trimTop = trimBottom = 0;

    while ( argIndex < argc - 1 && argv[ argIndex ][ 0 ] == '-' ) {
        if ( !strcmp( argv[ argIndex ], "-f" ) ) {
            forceL1Chars = true;
        }
        else if ( !strcmp( argv[ argIndex ], "-e" ) ) {
            forceEven = true;
        }
        else if ( !strcmp( argv[ argIndex ], "-e-" ) ) {
            forceEven = false;
        }
        else if ( !strcmp( argv[ argIndex ], "-H" ) ) {
            halfSize = true;
        }
        else if ( !strcmp( argv[ argIndex ], "-H-" ) ) {
            halfSize = false;
        }
        else if ( !strcmp( argv[ argIndex ], "-s" ) ) {
            evenSpace = true;
        }
        else if ( !strcmp( argv[ argIndex ], "-s-" ) ) {
            evenSpace = false;
        }
        else if ( !strcmp( argv[ argIndex ], "-h" ) ) {
            msHebrew = true;
        }
        else if ( !strcmp( argv[ argIndex ], "-h-" ) ) {
            msHebrew = false;
        }
        else if ( !strcmp( argv[ argIndex ], "-n" ) ) {
            disableKerning = true;
        }
        else if ( !strcmp( argv[ argIndex ], "-E" ) ) {
            forceHeightEven = true;
            forceWidthEven  = true;
        }
        else if ( !strcmp( argv[ argIndex ], "-E-" ) ) {
            forceHeightEven = false;
            forceWidthEven  = false;
        }
        else if ( !strcmp( argv[ argIndex ], "-V" ) ) {
            forceHeightEven = true;
        }
        else if ( !strcmp( argv[ argIndex ], "-V-" ) ) {
            forceHeightEven = false;
        }
        else if ( !strcmp( argv[ argIndex ], "-B" ) ) {
            forceBW            = true;
            doForceGraySetting = true;
            forceGraySetting   = true;
        }
        else if ( !strcmp( argv[ argIndex ], "-B-" ) ) {
            forceBW = false;
        }
        else if ( !strcmp( argv[ argIndex ], "-f-" ) ) {
            forceL1Chars = false;
        }
        else if ( !strcmp( argv[ argIndex ], "-n-" ) ) {
            disableKerning = false;
        }
        else if ( !strncmp( argv[ argIndex ], "-m", 2 ) ) {
            match = argv[ argIndex ] + 2;
        }
        else if ( !strncmp( argv[ argIndex ], "-b", 2 ) ) {
            bold = atoi( argv[ argIndex ] + 2 );
        }
        else if ( !strncmp( argv[ argIndex ], "-T", 2 ) ) {
            trimTop    = atoi( argv[ argIndex ] + 2 );
        }
        else if ( !strncmp( argv[ argIndex ], "-U", 2 ) ) {
            trimBottom = atoi( argv[ argIndex ] + 2 );
        }
        else if ( !strncmp( argv[ argIndex ], "-g", 2 ) ) {
            doForceGraySetting = true;
            forceGraySetting   = argv[ argIndex ][ 2 ] != '-';
        }
        else if ( !strncmp( argv[ argIndex ], "-a", 2 ) && NULL != strchr( argv[ argIndex ], ':' ) ) {
            HandleAdjustment( advanceAdjustments, argv[ argIndex ] + 2 );
        }
        else if ( !strncmp( argv[ argIndex ], "-k", 2 ) && NULL != strchr( argv[ argIndex ], ':' ) ) {
            HandleAdjustment( kerningAdjustments, argv[ argIndex ] + 2 );
        }
        else if ( !strncmp( argv[ argIndex ], "-v", 2 ) && NULL != strchr( argv[ argIndex ], ':' ) ) {
            HandleAdjustment( verticalAdjustments, argv[ argIndex ] + 2 );
        }
        else {
            encoding = argv[ argIndex ]+1;
        }
        argIndex++;
    }


    if ( doForceGraySetting ) {
        gray = forceGraySetting;
    }
    else {
        gray     = ( argc - argIndex == 3 ) && tolower(argv[ argc - 1 ][0])=='g';
    }

    if ( ! gray )
        disableKerning = true;

    if ( ! disableKerning )
        forceEven = false;

    pixelSize = 22;
    if ( 3 <= argc )
        pixelSize = atoi( argv[ argIndex + 1 ] );

    printf( "// Converted by %s from %s%s\n\n", argv[ 0 ], argv[ argIndex ], bold ? " with fake bold." : "" );

    scalable = LoadFace( &face, argv[ argIndex ], pixelSize, true );

    if ( gray && ! forceBW )
        printf( "grayMap %s\nfontVersion 2\n", grayMap );
    else
        printf( "fontType %d\n", 0x9000 );

    GetFaceMetrics( face, &highestTop, &lowestBottom, gray, forceL1Chars );

    if ( match != NULL ) {
        FT_Face face2;

        LoadFace( &face2, match, pixelSize, false );
        GetFaceMetrics( face2, &forceHighestTop, &forceLowestBottom, gray, forceL1Chars );

        FT_Done_Face( face2 );
    }
    else {
        forceHighestTop   = highestTop;
        forceLowestBottom = lowestBottom;
    }
    shift             = forceHighestTop - highestTop;
    height            = forceHighestTop - forceLowestBottom;

    if ( forceHeightEven && ( height % 2 ) )
        actualHeight = height + 1;
    else
        actualHeight = height;
    actualHeight -= trimTop + trimBottom;
    if ( halfSize )
        actualHeight /= 2;
    printf( "fRectHeight %d\n", actualHeight );

    descent = -forceLowestBottom - trimBottom;
    ascent  = actualHeight + forceLowestBottom + trimBottom;

    if ( forceHeightEven && ( descent % 2 ) ) {
        descent--;
        ascent++;
    }

    if ( descent < 0 ) {
        ascent += descent;
        descent = 0;
    }

    if ( ascent < 0 ) {
        descent += ascent;
        ascent = 0;
    }

    if ( halfSize ) {
        printf( "descent %d\n", descent / 2 );
        printf( "ascent %d\n\n", actualHeight - descent / 2);
    }
    else {
        printf( "descent %d\n", descent );
        printf( "ascent %d\n\n", ascent );
    }

#if 0
    { int test[] = { 'T', '.', '7', 'V', 0x5B7, 0x5D1, -1,  'C', -1 };
      int left;
      int advance;
      int i,j;
      EmitChars( face, test, highestTop, height, 0, false, &left, &advance );
      for ( i = 0 ; i < canvas.rows ; i ++ ) {
           for ( j = 0 ; j < canvas.width ; j++ ) {
                printf( "%02X ", GetPixel( &canvas, j, i ) );
           }
           putchar( '\n' );
      }
    }
#endif

    for ( i = 0; i < 256; i++ ) {
        int c;
        int multiplicity;
        int pixelWidthUsed = 0;
        int advance;

        multiplicity = 1;
        if ( forceL1Chars && i == 0xA0 )
            c = ' ';
        else if ( forceL1Chars && i == 0x85 ) {
            c            = '.';
            multiplicity = 3;
        }
        else
            c = i;
        /* load glyph image into the slot (erase previous one) */
        if ( forceEven && scalable )
            FT_Set_Pixel_Sizes( face, pixelWidthUsed = 0, pixelSize );
        if ( 0 == Decode( face, c ) )
            continue;
        if ( FT_Load_Glyph( face, Decode( face, c ), FT_LOAD_RENDER | ( gray ? grayMode : FT_LOAD_TARGET_MONO ) ) )
            continue;
        if ( face->glyph->bitmap.width == 0 && face->glyph->advance.x == 0 )
            continue;
        if ( forceEven && scalable ) {
            int width;
            width = EmitBitmap( i, &(face->glyph->bitmap), face->glyph->bitmap_left,
                ( face->glyph->advance.x + 63 ) / 64,
                highestTop - face->glyph->bitmap_top,
                height, shift + verticalAdjustments[ i ], multiplicity, 1, CheckNarrow( c ), bold );
            if ( width % 2 ) {
                int oldWidth;
                /* readjust due to odd width */
                int delta;
                int sign;
                oldWidth = width;
                if ( 32 <= face->glyph->advance.x % 64 || face->glyph->advance.x % 64 == 0 ||
                    ( face->glyph->advance.x + 63 ) / 64 < 5 )
                    sign = 1;
                else
                    sign = -1;
                delta = ( ( pixelSize + width / 2 ) / width ) * sign;
                if ( delta == 0 )
                    delta = sign;
                printf( "// Even width fix from original width %d by using delta=%d.\n", width, delta );
                FT_Set_Pixel_Sizes( face, pixelWidthUsed = pixelSize + delta, pixelSize );
                FT_Load_Glyph( face, Decode( face, c ), FT_LOAD_RENDER | ( gray ? grayMode : FT_LOAD_TARGET_MONO ) );
                width = EmitBitmap( i, &(face->glyph->bitmap), face->glyph->bitmap_left,
                  ( face->glyph->advance.x + 63 ) / 64,
                  highestTop - face->glyph->bitmap_top,
                  height, shift + verticalAdjustments[ i ], multiplicity, 1, CheckNarrow( c ), bold );
                if ( width % 2 || 1 < width - oldWidth || 1 < oldWidth - width ) {
                    if ( 1 < width - oldWidth && 1 < delta )
                        delta--;
                    else if ( 1 < oldWidth - width && delta < -1 )
                        delta++;
                    else
                        delta = -delta;
                    printf( "// Fix failed; it generated width %d.  Trying delta=%d.\n", width, delta );
                    if ( delta < 0 && ( face->glyph->advance.x + 63 ) / 64 < 5 ) {
                        FT_Set_Pixel_Sizes( face, pixelWidthUsed = 0, pixelSize );
                        FT_Load_Glyph( face, Decode( face, c ), FT_LOAD_RENDER | ( gray ? grayMode : FT_LOAD_TARGET_MONO ) );
                    }
                    else {
                        FT_Set_Pixel_Sizes( face, pixelWidthUsed = pixelSize + delta, pixelSize );
                        FT_Load_Glyph( face, Decode( face, c ), FT_LOAD_RENDER | ( gray ? grayMode : FT_LOAD_TARGET_MONO ) );
                        width = EmitBitmap( i, &(face->glyph->bitmap), face->glyph->bitmap_left,
                          ( face->glyph->advance.x + 63 ) / 64,
                          highestTop - face->glyph->bitmap_top,
                          height, shift + verticalAdjustments[ i ], multiplicity, 1, CheckNarrow( c ), bold );
                        if ( width % 2 || 1 < width - oldWidth || 1 < oldWidth - width ) {
                            printf( "// Second fix attempt failed, too, generating width %d.\n", width );
                            FT_Set_Pixel_Sizes( face, pixelWidthUsed = 0, pixelSize );
                        }
                    }
                }
            }
        }
        /* now, draw to our target surface */

        skip = forceBW || halfSize;

        forceWidth  = 0;
        forceHeight = 0;

        advance = ( face->glyph->advance.x + 63 ) / 64  + advanceAdjustments[ i ];

        if ( ( forceWidthEven || ( evenSpace && i == ' ' ) ) && advance % 2 )
            advance++;
            
        EmitBitmap( i, &(face->glyph->bitmap), face->glyph->bitmap_left + kerningAdjustments[ i ],
              advance,
              highestTop - face->glyph->bitmap_top,
              height - trimBottom - trimTop, shift + verticalAdjustments[ i ] - trimTop, multiplicity, 0, CheckNarrow( c ), bold );

        if ( forceBW || halfSize ) {
            Boolean oldWidthEven;
            Boolean oldHeightEven;
            int     adjTrimTop;
            int     adjTrimBottom;
            int     adjVertical;

            adjTrimTop = trimTop;
            adjTrimBottom = trimBottom;
            adjVertical   = verticalAdjustments[ i ];

            oldHeightEven = forceHeightEven;
            oldWidthEven  = forceWidthEven;

            forceHeightEven = false;
            forceWidthEven  = false;

            if ( ! disableKerning ) {
                forceWidth = advance;
            }
            else {
                forceWidth = lastGlyphWidth;
            }

            forceHeight = lastGlyphHeight;

            printf( "//Forcing width=%d, height=%d.\n", forceWidth, forceHeight );

            if ( halfSize ) {
                forceWidth  /= 2;
                forceHeight /= 2;
                advance     /= 2;
                adjTrimTop  /= 2;
                adjTrimBottom /= 2;
                adjVertical   /= 2;
                FT_Set_Pixel_Sizes( face, 0, pixelSize /2 );
            }
            else {
                FT_Set_Pixel_Sizes( face, 0, pixelSize );
            }

            FT_Load_Glyph( face, Decode( face, c ), FT_LOAD_RENDER | FT_LOAD_TARGET_MONO );
            skip = false;
            EmitBitmap( i, &(face->glyph->bitmap), face->glyph->bitmap_left + kerningAdjustments[ i ],
                  advance,
                  ( halfSize ? ( highestTop + 1 ) / 2 : highestTop )  - face->glyph->bitmap_top,
                  height - adjTrimTop - adjTrimBottom,
                  shift + adjVertical - adjTrimTop, multiplicity, 0, CheckNarrow( c ), bold );
            if ( halfSize )
                FT_Set_Pixel_Sizes( face, 0, pixelSize );
            forceWidthEven = oldWidthEven;
            forceHeightEven = oldHeightEven;
        }
      }
      FT_Done_Face( face );
      FT_Done_FreeType( library );
     return 0;
}
