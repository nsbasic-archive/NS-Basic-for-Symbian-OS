#include <stdio.h>
#include <string.h>

#ifdef __ELF__
#define STRNICMP strncasecmp
#define STRICMP strcasecmp
#else
#define STRNICMP strnicmp
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

int
main( int argc, char** argv )
{
   unsigned short fontID;
   int i;
   FILE* f;
   if ( ( argc != 3  && argc != 2 ) || NULL == ( f = fopen( argv[1],"wb" ) ) ) {
       fprintf(stderr, "%s filename [FontID in hex | name]\n", argv[0] );
       return 3;
   }
   fputc( 0x40, f );
   fputc( 0, f );
   fontID = 0;
   if ( argc == 3 ) {
      for ( i = 0 ; i < NUM_FONTS ; i++ ) {
           if ( !STRICMP( argv[2], fontList[i].name ) ) {
               fontID = fontList[i].fontID;
           }
      }
      if ( i == NUM_FONTS )
         sscanf( argv[2],"%hx",&fontID );
   }
   fputc( fontID >> 8, f );
   fputc( fontID & 0xFF, f );
   fclose( f );
   return 0;
}
