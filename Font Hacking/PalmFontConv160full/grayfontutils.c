/*
 * $Id: grayfontutils.c,v 1.1 2004/02/13 16:21:55 arpruss Exp $
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

/* This is a compatibility module for grayfont.c to make it work outside
   of the Plucker project */

#include "grayfontutils.h"

typedef UInt8 FontResourceKind;
typedef struct {
    UInt8 screenDepth;
} GrayFontPrefsType;
#define GetFontResource( resourceType, resourceID, kind ) \
           ( *( kind ) = 0, DmGetResource( ( resourceType ), ( resourceID ) ) )
#define ReleaseFontResource( handle, kind ) \
           DmReleaseResource( handle )
#define GetMaxBitDepth()   16     // Only support OS5 hires at present
#define Have68K()          false  // Only support OS5 hires at present
#define Support35()        true   // Only support OS5 hires at present
#define SafeMemPtrNew      MemPtrNew // For now
#define SafeMemPtrFree     MemPtrFree // For now

#define NATIVE   kCoordinatesNative
#define STANDARD kCoordinatesStandard

#define errOSVersionTooLow      ( appErrorClass | 11 )

void HiResAdjustNativeToCurrent ( Coord* x );
void HiResAdjustCurrentToNative ( Coord* x );
void AdjustCoordStandardToNative ( Coord* coord );

static Boolean           force8Bits = false;
static GrayFontPrefsType pref = { 16 };
#define Prefs() ( &pref )

/* Get the nativeDensity. */
static UInt16 PalmGetDensity( void )
{
    UInt32 densityValue;
    Err    err;
    err = WinScreenGetAttribute( winScreenDensity,
              &densityValue );
    if ( err == errNone )
        return ( UInt16 ) densityValue;
    else
        return kDensityLow;
}


void HiResAdjustCurrentToNative
     (
     Coord* x
     )
{
    if ( WinGetCoordinateSystem() != STANDARD )
        return;
    AdjustCoordStandardToNative( x );
}




/* Adjust native coordinates to standard if needed.  This may be an
   overestimate. */
void HiResAdjustNativeToCurrent
     (
     Coord* x
     )
{
    UInt32 nativeDensity;
    Err    err;
    if ( WinGetCoordinateSystem() != STANDARD )
        return;
    err = WinScreenGetAttribute( winScreenDensity,
              &nativeDensity );
    switch ( nativeDensity ) {
        case kDensityOneAndAHalf:
            *x = ( ( *x ) * 2 + 2 ) / 3;
            break;
        case kDensityDouble:
            *x = ( ( *x ) + 1 ) / 2;
            break;
        case kDensityTriple:
            *x = ( ( *x ) + 2 ) / 3;
            break;
        case kDensityQuadruple:
            *x = ( ( *x ) + 3 ) / 4;
            break;
        default:
            break;
    }
    return;
}



/* Convert a coord from kCoordinatesStandard to kCoordinatesNative */
void AdjustCoordStandardToNative
    (
    Coord* coord
    )
{
    UInt32 nativeDensity;
    Err    err;
    err = WinScreenGetAttribute( winScreenDensity,
              &nativeDensity );
    switch ( nativeDensity ) {
        case kDensityOneAndAHalf:
            *coord += *coord / 2;
            break;
        case kDensityDouble:
            *coord *= 2;
            break;
        case kDensityTriple:
            *coord *= 3;
            break;
        case kDensityQuadruple:
            *coord *= 4;
            break;
        default:
            break;
    }
}



/* set the coordinate system to be used for palm hires devices */
UInt16 PalmSetCoordinateSystem
    (
    UInt16 coordSys
    )
{
    UInt16 prevCoordSys;

    prevCoordSys = WinSetCoordinateSystem( coordSys );

    return prevCoordSys;
}



/* Force 8 bit character set */
void GrayFntForce8Bits( Boolean state )
{
    force8Bits = state;
}



static UInt16 MyTxtGlueGetNextChar( const Char* chars, UInt16 inOffset, WChar* chP )
{
    if ( force8Bits ) {
        *chP = ( UInt8 )( chars[ inOffset ] );
        return 1;
    }
    else
        return TxtGlueGetNextChar( chars, inOffset, chP );
}



#define TxtGlueGetNextChar MyTxtGlueGetNextChar
