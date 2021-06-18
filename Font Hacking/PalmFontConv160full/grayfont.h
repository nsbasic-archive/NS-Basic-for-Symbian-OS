/*
    Copyright (c) 2004, Alexander R. Pruss
    All rights reserved.

    This license applies to grayfont.h only.

    Redistribution and use in source and binary forms, with or without modification,
    are permitted provided that the following conditions are met:

        Redistributions of source code must retain the above copyright notice, this
        list of conditions and the following disclaimer.

        Redistributions in binary form must reproduce the above copyright notice, this
        list of conditions and the following disclaimer in the documentation and/or
        other materials provided with the distribution.

        Neither the name of the PalmResize Project nor the names of its
        contributors may be used to endorse or promote products derived from this
        software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
    ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
    ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#ifndef GRAYFONT_H
#define GRAYFONT_H

// The following three lines are for use within Plucker
//#include "config.h"
//#include "viewer.h"
//#include "hires.h"

#define GRAY_FONT_LEFT   'L'
#define GRAY_FONT_RIGHT  'R'
#define GRAY_FONT_NORMAL 'U'

#ifndef GRAYFONT_SECTION
# define GRAYFONT_SECTION
#endif

#ifdef HAVE_GRAY_FONT
/* Start the gray-scale font functions */
void GrayFntStart( void ) GRAYFONT_SECTION;

/* Stop them and clear memory */
void GrayFntStop( void ) GRAYFONT_SECTION;

Err GrayFntDefineFont ( FontID font, void*  fontP ) GRAYFONT_SECTION;

FontID GrayFntGetFont( void ) GRAYFONT_SECTION;

FontID GrayFntSetFont ( FontID font ) GRAYFONT_SECTION;

Int16 GrayFntLineHeight( void ) GRAYFONT_SECTION;

Int16 GrayFntCharHeight( void ) GRAYFONT_SECTION;

Int16 GrayFntWCharWidth ( WChar ch ) GRAYFONT_SECTION;

Int16 GrayFntCharWidth ( Char ch ) GRAYFONT_SECTION;

Int16 GrayFntCharsWidth ( Char const* chars, Int16 length ) GRAYFONT_SECTION;

Int16 GrayFntWidthToOffset ( Char* const chars, UInt16 length, Int16
      pixelWidth, Boolean* leadingEdge, Int16* truncWidth ) GRAYFONT_SECTION;


Char GrayFntSetOrientation ( Char o ) GRAYFONT_SECTION;

void GrayWinDrawCharsGeneral ( Char const* chars, Int16 length, Coord x,
         Coord y, Boolean invert ) GRAYFONT_SECTION;

#define GrayWinDrawChars(c,l,x,y) GrayWinDrawCharsGeneral(c,l,x,y,false)
#define GrayWinDrawInvertedChars(c,l,x,y) GrayWinDrawCharsGeneral(c,l,x,y,true)

void GrayWinDrawChar ( WChar ch, Coord x, Coord y ) GRAYFONT_SECTION;

Boolean GrayFntIsCurrentGray ( void ) GRAYFONT_SECTION;

/* Handle font substitution */
void GrayFntSubstitute ( FontID oldFont, FontID newFont ) GRAYFONT_SECTION;

void GrayFntClearSubstitutionList( void ) GRAYFONT_SECTION;

void GrayFntSetBackgroundErase( Boolean state ) GRAYFONT_SECTION;

extern Coord GrayFntMinLeftKerning( FontID font ) GRAYFONT_SECTION;

extern Coord GrayFntMaxRightOverhang( FontID font ) GRAYFONT_SECTION;

#else /* HAVE_GRAY_FONT */

#include <FntGlue.h>

#define GrayFntMinLeftKerning( a ) 0
#define GrayFntMaxRightOverhang( a ) 0
#define GrayFntStart()
#define GrayFntStop()
#define GrayFntDefineFont  FntDefineFont
#define GrayFntGetFont     FntGetFont
#define GrayFntSetFont     FntSetFont
#define GrayFntLineHeight  FntLineHeight
#define GrayFntCharHeight  FntCharHeight
#define GrayFntCharsWidth  FntCharsWidth
#define GrayFntWCharHeight FntGlueWCharHeight
#define GrayFntWidthToOffset FntWidthToOffset
#define GrayFntSetOrientation( o ) GRAY_FONT_NORMAL
#define GrayWinDrawChars   WinDrawChars
#define GrayWinDrawInvertedChars   WinDrawInvertedChars
#define GrayWinDrawChar    WinDrawChar
#define GrayFntIsCurrentGray() false
#define GrayFntSubstitute( oldFont, newFont )
#define GrayFntClearSubstitutionList()
#define GrayFntSetBackgroundErase( a )

#define NO_GRAY_FONT_SUBSTITUTION

#endif /* HAVE_GRAY_FONT */

#ifndef NO_GRAY_FONT_SUBSTITUTION
#define FntDefineFont  GrayFntDefineFont
#undef  FntGetFont
#define FntGetFont     GrayFntGetFont
#undef  FntSetFont
#define FntSetFont     GrayFntSetFont
#define FntLineHeight  GrayFntLineHeight
#define FntCharHeight  GrayFntCharHeight
#define FntWCharHeight GrayFntGlueWCharHeight
#define FntCharsWidth  GrayFntCharsWidth
#undef  WinDrawChars
#define WinDrawChars   GrayWinDrawChars
#undef  WinDrawInvertedChars
#define WinDrawInvertedChars   GrayWinDrawInvertedChars
#undef  WinDrawChar
#define WinDrawChar    GrayWinDrawChar
#define TxtGlueCharWidth GrayFntWCharWidth
#undef  FntWidthToOffset 
#define FntWidthToOffset GrayFntWidthToOffset
#undef  FntCharWidth
#define FntCharWidth     GrayFntCharWidth
#endif /* NO_GRAY_FONT_SUBSTITUTION */

#endif /* GRAYFONT_H */

