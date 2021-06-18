#include <stdio.h>
#include <string.h>

/* creates tAIN0000.bin resource for FontSubst
   and converts the four Fonts4OS5 font components
   to FontSubst format */
   
   
void Convert( char* dir, char* _src, char* _dest, char secondByte )
{
	int length;
	char font[65536];
	char src[1000];
	char dest[1000];
	FILE* in;
	FILE* out;
	strcpy( src, dir );
	strcat( src, _src );
	strcpy( dest, dir );
    strcat( dest, _dest );
	in = fopen( src, "rb" );
	if ( in == NULL ) {
	    fprintf( stderr, "Cannot open %s.\n", src );
	    return;
	}
	out = fopen( dest, "wb" );
	length = fread( font, 1, 65536,  in );
	font[ 1 ] = secondByte;
	fwrite( font, 1, length, out );
	fclose( in );
	fclose( out );
	unlink( src );
}


int main( int argc, char** argv )
{
    int i;
    FILE* f;
    char name[32];
    char fname[ 1000 ];
    int  fromFontSubst;
    if ( argc != 3 ) {
        puts("fsconvert dir/ [\"FontSubstName\" | \"-\"]\n"
             "   If the second argument is -, convert to Fonts4os5 format.\n"
             "   Else convert to FontSubst format.");
        return 3;
    }
    fromFontSubst = ! strcmp( argv[ 2 ],"-" );
    if ( ! fromFontSubst ) {
        strncpy( name, argv[ 2 ], 31 );
        name[ 31 ] = 0;
        for ( i = strlen( name ) + 1 ; i < 32 ; i++ ) name[ i ] = ' ';
        strcpy( fname, argv[ 1 ] );
        strcat( fname, "tAIN0000.bin" );
        f = fopen( fname, "wb" );
        fwrite(name, 32, 1, f );
        fclose( f );
        Convert( argv[ 1], "afnx7d00.bin", "afnx0000.bin", 0x02 );
        Convert( argv[ 1], "afnx7d01.bin", "afnx0001.bin", 0x02 );
        Convert( argv[ 1], "afnx7d02.bin", "afnx0002.bin", 0x02 );
        Convert( argv[ 1], "afnx7d03.bin", "afnx0007.bin", 0x02 );
    }
    else {
        Convert( argv[ 1], "afnx0000.bin", "afnx7d00.bin", 0x92 );
        Convert( argv[ 1], "afnx0001.bin", "afnx7d01.bin", 0x92 );
        Convert( argv[ 1], "afnx0002.bin", "afnx7d02.bin", 0x92 );
        Convert( argv[ 1], "afnx0007.bin", "afnx7d03.bin", 0x92 );
    }
    return 0;
}
