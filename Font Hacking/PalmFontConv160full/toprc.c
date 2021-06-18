/*
 * $Id: toprc.c,v 1.12 2004/10/17 13:17:50 arpruss Exp $
 *
 * The true home of this program is palmfontconv.sourceforge.net.  If you
 * find it elsewhere and improve it, please send a patch to
 * Alexander R. Pruss <ap85@georgetown.edu>.
 *
 * fontconv - Copyright (c) 2003, Alexander R. Pruss
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
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include "palmtypes.h"

#ifdef __GNUCC__
#define STRICMP strcasecmp
#else
#define STRICMP stricmp
#endif

struct {
    char*          name;
    unsigned short fontID;
} fontList[] = {
	{ "hrTinyFont", 0 },
	{ "hrTinyBoldFont", 1 },
	{ "hrSmallFont", 2 },
	{ "hrSmallSymbolFont", 3 },
	{ "hrSmallSymbol11Font", 4 },
	{ "hrSmallSymbol7Font", 5 },
	{ "hrSmallLedFont", 6 },
	{ "hrSmallBoldFont", 7 },
	{ "hrStdFont", 8 },
	{ "hrBoldFont", 9 },
	{ "hrLargeFont", 10 },
	{ "hrSymbolFont", 11 },
	{ "hrSymbol11Font", 12 },
	{ "hrSymbol7Font", 13 },
	{ "hrLedFont", 14 },
	{ "hrLargeBoldFont", 15 },
    { "stdFont", 0 },
 { "boldFont", 1 },
 { "largeFont", 2 },
 { "symbolFont", 3  },
 { "symbol11Font", 4 },
 { "symbol7Font", 5 },
 { "ledFont", 6 },
 { "largeBoldFont", 7 },
 { "stdFixedFont_sony", 137 },
 { "stdFixedFont_palm", 144 },
 { "narrowFixedFont", 129 }
};

#define NUM_FONTS ( sizeof( fontList ) / sizeof( *fontList ) )

#define TIME_UNIX_TO_PALM  2082844800
#define resourceAttribute  0x0001
DatabaseHdrType header;
RsrcEntryType   resourceEntries[ MAX_RESOURCES ];
RecordEntryType dbEntries[ MAX_RESOURCES ];
void*           resourceData[ MAX_RESOURCES ];
Word            resourceSizes[ MAX_RESOURCES ];
int             numResources = 0;
int             pdbFormat = 0;
int             littleEndian = 0;
int             version = 1;
int             splitUp = 0;
int             ignoreMissing = 0;

void err(char *s, ...)
{
    va_list argptr;
    va_start(argptr,s);
    vfprintf(stderr,s,argptr);
    va_end(argptr);
    fputc('\n',stderr);
    exit(1);
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



Boolean IsLittleEndian( void )
{
    Word w = 1;
    return *(Byte*)&w;
}


void ReverseDWord( Byte* data )
{
    Byte temp;

    temp      = data[ 3 ];
    data[ 3 ] = data[ 0 ];
    data[ 0 ] = temp;

    temp      = data[ 2 ];
    data[ 2 ] = data[ 1 ];
    data[ 1 ] = temp;
}


void ReverseWord( Byte* data )
{
    Byte temp;
    temp      = data[ 1 ];
    data[ 1 ] = data[ 0 ];
    data[ 0 ] = temp;
}



/* Makes sure p, which points to a structure in the
   native format on which this code is executing is
   put into big endian format.  sizes specifies the
   variable sizes in p.  The format of "sizes" is an
   ASCII string of size-identifiers: B for byte (no fix-up),
   W for word (two bytes) and D for dword (four bytes),
   each optionally followed by a count.  Thus, W3BBD means:
   three words, two bytes, and a dword. */
int ToBigEndian( void* p, char* sizes )
{
    Byte *data;
    int count;
    data = p;
    while ( *sizes ) {
        char type;
        type = sizes[ 0 ];
        sizes++;
        if ( *sizes && isdigit( *sizes ) ) {
            count = ( *sizes - '0' );
            sizes++;
            while ( *sizes && isdigit( *sizes ) ) {
                count = count * 10 + ( *sizes - '0' );
                sizes++;
            }
        }
        else
            count = 1;
        while ( count-- ) {
            switch( type ) {
                case 'B':
                     data++;
                     break;
                case 'W':
                     if ( littleEndian )
                         ReverseWord( data );
                     data += 2;
                     break;
                case 'D':
                     if ( littleEndian )
                         ReverseDWord( data );
                     data += 4;
                     break;
                default:
                    err( "Internal error: Unknown size type %c.\n", type );
            }
        }
    }
    return data - ( Byte* ) p;
}






void PRCReset( void )
{
    int i;
    for ( i = 0 ; i < numResources ; i++ ) {
        free( resourceData[ i ] );
    }
    numResources = 0;
}


void PRCInit( char* dbName, char* type, char* creator )
{
    DWord t;
    PRCReset();
    t = TIME_UNIX_TO_PALM + time( NULL );
    memset( &header, 0, sizeof( header ) );
    strncpy( header.name, dbName, dmDBNameLength - 1 );
    header.name[ dmDBNameLength - 1 ] = 0;
    header.attributes                 = pdbFormat ? 0 : resourceAttribute;
    header.creationDate               = t;
    header.modificationDate           = t;
    header.lastBackupDate             = 0;
    header.version                    = version;
    strncpy( header.type, type, 4 );
    strncpy( header.creator, creator, 4 );
}



void PRCDump( char* fileName )
{
    FILE* f = myfopen( fileName, "wb" );
    int i;
    DWord sizeSoFar;
    DWord resourceListSize;
    DWord padding;
    int header_size;
    header.recordList.numRecords = numResources;
    header_size = ToBigEndian( &header, DatabaseHdrType_sizes );
    fwrite( &header, 1, header_size, f );
    if ( pdbFormat )
        resourceListSize = RecordEntryType_size * numResources;
    else
        resourceListSize = RsrcEntryType_size * numResources;
    if ( ( resourceListSize + header_size ) % 4 != 0 ) {
        padding = 4 - ( ( resourceListSize + header_size ) % 4 );
    }
    else
        padding = 0;
    sizeSoFar = header_size + resourceListSize + padding;
    for ( i = 0; i < numResources ; i++ ) {
        if ( pdbFormat ) {
            dbEntries[ i ].localChunkID = sizeSoFar;
            dbEntries[ i ].uniqueID[ 2 ] = ( unsigned )i % 256;
            dbEntries[ i ].uniqueID[ 1 ] = ( ( unsigned )i / 256 ) % 256;
            dbEntries[ i ].uniqueID[ 0 ] = ( unsigned )i / 65536;
        }
        else {
            resourceEntries[ i ].localChunkIDHigh = sizeSoFar >> 16;
            resourceEntries[ i ].localChunkIDLow  = sizeSoFar & 0xFFFF;
        }
        sizeSoFar +=  resourceSizes[ i ];
    }
    for ( i = 0 ; i < numResources ; i++ ) {
        if ( pdbFormat ) {
            ToBigEndian( &dbEntries[ i ], RecordEntryType_sizes );
            fwrite( &dbEntries[ i ], RecordEntryType_size, 1, f );
        }
        else {
            ToBigEndian( &resourceEntries[ i ], RsrcEntryType_sizes );
            fwrite( &resourceEntries[ i ], RsrcEntryType_size, 1, f );
        }
    }
    while ( padding-- )
        fputc( 0, f );
    for ( i = 0 ; i < numResources ; i++ ) {
        fwrite( resourceData[ i ], 1, resourceSizes[ i ], f );
    }
    fclose( f );
    PRCReset();
}



void PRCAdd( Byte* type, Word id, void* record, Word size )
{
    resourceData[ numResources ] = mymalloc( size );
    memcpy( resourceData[ numResources ], record, size );
    if ( pdbFormat ) {
        dbEntries[ numResources ].attributes = 0;
    }
    else {
        resourceEntries[ numResources ].id = id;
        memcpy( resourceEntries[ numResources ].type, type, 4 );
    }
    resourceSizes[ numResources ] = size;
    numResources++;
}


char buffer[ MAX_RESOURCE_SIZE ];


unsigned short GetFontID( char* s )
{
    int            j;
    unsigned short fontID;
    for ( j = 0 ; j < NUM_FONTS ; j++ ) {
        if ( ! STRICMP( s, fontList[ j ].name ) ) {
            fontID = fontList[ j ].fontID;
            break;
        }
    }
    if ( j == NUM_FONTS ) {
        for ( j = 0 ; s[ j ] ; j++ )
            if ( ! isxdigit( s[ j ] ) )
                err( "Error in font specification `%s'.\n", s );
        sscanf( s, "%hx", &fontID );
    }
    return fontID;
}


int main( int argc, char** argv )
{
   int i;
   int firstArg;
   char *fnameStart;
   Word size;
   FILE* f;
   if ( argc < 6 )
       err( "Usage:\n"
            "  %s outname.prc dataBaseName TypeID CreatorID [ rsrcXXXX.bin | -f rsrcXXXX toFont ] ...\n"
            "or:\n"
            "  %s -d outname.pdb dataBaseName TypeID CreatorID record0.bin ...\n\n"
            "Fonts may be specified either by name identifier or in hex.  This is only for\n"
            "generating font remap packages for software using Plucker font packages.\n",
            argv[ 0 ], argv[ 0 ] );

   littleEndian = IsLittleEndian();

   firstArg = 1;
   while ( argv[ firstArg ][ 0 ] == '-' ) {
       if ( ! strcmp( argv[ firstArg ], "-d" ) ) {
            pdbFormat = 1;
            firstArg++;
       }
       else if ( ! strcmp( argv[ firstArg ], "-s" ) ) {
            firstArg++;
            splitUp = 1;
       }
       else if ( ! strcmp( argv[ firstArg ], "-i" ) ) {
            firstArg++;
            ignoreMissing = 1;
       }
   }

   if ( pdbFormat )
       splitUp = 0;

   PRCInit( argv[ firstArg + 1 ], argv[ firstArg + 2 ], argv[ firstArg + 3 ] );
   for ( i = firstArg + 4 ; i < argc ; i++ ) {
        unsigned id;
        char hex[ 5 ];
        if ( ! pdbFormat && ! strncmp( argv[ i ], "-f", 2 ) ) {
            unsigned short fontID;

            if ( argc <= i + 2 ) {
                err( "Need to specify both fromFont and toFont.\n" );
            }

            fnameStart  = argv[ i + 1 ];
            fontID      = GetFontID( argv[ i + 2 ] );
            i          += 2;
            buffer[ 0 ] = 0x40;
            buffer[ 1 ] = 0;
            buffer[ 2 ] = fontID >> 8;
            buffer[ 3 ] = fontID & 0xFF;
            size        = 4;
        }
        else {
            char *p;
            if ( ! pdbFormat && strlen( argv[ i ] ) < 8 )
                err( "Filename %s not in required rsrcXXXX format.\n", argv[ i ] );
            fnameStart = argv[ i ];
            if ( NULL != ( p = strrchr( fnameStart, ':' ) ) ) {
                fnameStart = p + 1;
            }
            if ( NULL != ( p = strrchr( fnameStart, '\\' ) ) ) {
                fnameStart = p + 1;
            }
            if ( NULL != ( p = strrchr( fnameStart, '/' ) ) ) {
                fnameStart = p + 1;
            }
            if ( ignoreMissing ) {
                f = fopen( argv[ i ], "rb" );
                if ( f == NULL )
                    continue;
            }
            else {
                f = myfopen( argv[ i ], "rb" );
            }    
            if ( splitUp ) {
                char type[ 5 ];

                size = fread( buffer, 1, MAX_SAFE_RESOURCE_SIZE, f );

                strncpy( hex, fnameStart+4, 4 );
                hex[ 4 ] = 0;
                sscanf( hex, "%x", &id );
                strncpy( type, fnameStart, 4 );
                PRCAdd( type, ( Word )id, buffer, size );

                if ( MAX_SAFE_RESOURCE_SIZE <= size ) {
                    size = fread( buffer, 1, MAX_RESOURCE_SIZE, f );
                    type[ 0 ] = '2';
                    PRCAdd( type, ( Word )id, buffer, size );
                }

                fclose( f );
                continue;
            }
            else {
                size = fread( buffer, 1, MAX_RESOURCE_SIZE, f );
            }
            fclose( f );
        }
        strncpy( hex, fnameStart+4, 4 );
        hex[ 4 ] = 0;
        sscanf( hex, "%x", &id );
        PRCAdd( fnameStart, ( Word )id, buffer, size );
   }
   PRCDump( argv[ firstArg ] );
   return 0;
}

