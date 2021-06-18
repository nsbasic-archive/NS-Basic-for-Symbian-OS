/*
 * $Id: fontconv.c,v 1.30 2005/04/30 00:47:38 arpruss Exp $
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
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <fcntl.h>
#include <setjmp.h>
#include "missingicons.c"

#include "palmtypes.h"
#include "fontconv.h"

#define HIRES 144
#define LORES 72
#ifdef __GNUCC__
#define STRNICMP strncasecmp
#else
#define STRNICMP strnicmp
#endif

#define MAX_REMAPS 256
#define MAX_MYMALLOCS (256*256)
#define MAX_OPEN_FILES  32

#define SKIP_GLYPH -2

#define SHORTHEADER 13
#define OWT_DELTA (2*8)

static Word  fRectHeight2;
static Word  rowWords2;
static DWord glyphBitsOffset;
static DWord locationTableOffset;
static DWord glyphLength,glyphLength2;
static Byte  *glyph,*glyph2,*glyph3;
static Word  *locationTable,*locationTable2;
static int numChars;
static Word  ignore;
static int numRemaps;

static FILE*    openFiles[ MAX_OPEN_FILES ];
static void*    memoryPointers[ MAX_MYMALLOCS ];
static int      numMemoryPointers = 0;
static jmp_buf  exitJump;

#define GRAY_HEADER_LENGTH ( 4 + 4 + 2 + 32 ) // 'Gray' + resType + resNum + dbName

static Byte grayHeader[ GRAY_HEADER_LENGTH ];

static int remapfrom[ MAX_REMAPS ];
static int remapto[ MAX_REMAPS ];

static int quiet = 0;
static int fakeFont = 0;
static int half = 0;

static struct {
  signed char offset;
  signed char width;
} *owTable,*owTable2;

static struct fonttype {
  Word resolution;
  Byte type2;
  Byte afnx;
  Byte text;
  Byte infoOnly;
  Byte includelores;
} in,out;

static FontTypeV2 header;

#define SWAPBYTES(x,y) { unsigned char __store; __store=(y); (y)=(x); (x)=__store; }

enum labels { LfontType=0, LmaxWidth, LfRectWidth, LfRectHeight,
     Lascent, Ldescent, LleftKerning, Ladvance };

static struct {
   char *fieldname;
   Word *location;
   Byte have;
} values[] = {
               {"fontType",&header.fontType,0},
               {"maxWidth",&header.maxWidth,0},
               {"fRectWidth",&header.fRectWidth,0},
               {"fRectHeight",&header.fRectHeight,0},
               {"ascent", &header.ascent, 0},
               {"descent", &header.descent, 0},
               {"leftKerning", &ignore, 0},
               {"advance", &ignore, 0}
};
#define NUM_VALUES (sizeof values / sizeof *values)

static int widths[256];
static int heights[256];
static int ydelta[256];
static char **chardata[256];
static int  numLines[256];

static int doreverse=0;
static int doubleputpixel=0;

static void cleanup( void )
{
    int i;

    for ( i = 0 ; i < numMemoryPointers ; i++ ) {
         free( memoryPointers[ i ] );
    }

    numMemoryPointers = 0;

    for ( i = 0 ; i < MAX_OPEN_FILES ; i++ ) {
         if ( NULL != openFiles[ i ] ) {
             fclose( openFiles[ i ] );
             openFiles[ i ] = NULL;
         }
    }
}

static void initialize( void )
{
    int i;
    cleanup();
    for ( i = 0 ; i < NUM_VALUES; i++ ) {
         values[ i ].have = 0;
         openFiles[ i ]   = NULL;
    }
    grayHeader[ 0 ] = 0;
}

static void err(char *s, ...)
{
    va_list argptr;
    va_start(argptr,s);
    vfprintf(stderr,s,argptr);
    va_end(argptr);
    fputc('\n',stderr);
    cleanup();
    longjmp(exitJump,1);
}

static void reverse_word_array(void *p,unsigned n)
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

static Word reverse_word(Word w)
{
    return doreverse?(w<<8) | (w>>8):w;
}

static DWord reverse_dword(DWord w)
{
    return doreverse?(w>>24)|(((w>>16)&0xff)<<8)|(((w>>8)&0xff)<<16)|(w<<24):w;
}

static int endiancheck(int c)
{
    /* on LSB first systems, return c.  Else return !c */
    Word w=0;
    *(char*)&w=1;
    if(w==1) return c; else return !c;
}

static FILE *myfopen(char *s,char *t)
{
    FILE *f;
    int   i;
    
    for ( i = 0 ; i < MAX_OPEN_FILES ; i++ )
         if ( openFiles[ i ] == NULL )
             break;
    if ( i == MAX_OPEN_FILES )
        err("Too many open files.\n");
    f=fopen(s,t);
    if(NULL==f)
        err("Error opening %s.\n",s);
    openFiles[ i ] = f;
    return f;
}

static void myfclose( FILE *f )
{
    int i;

    fclose( f );
    for ( i = 0 ; i < MAX_OPEN_FILES ; i++ ) {
         if ( openFiles[ i ] == f ) {
             openFiles[ i ] = NULL;
         }
    }
}

static char* skipspace(char* s)
{
    while(isspace(*s) && *s) s++;
    return s;
}

static void handleremap(char* s)
{
    int from;
    int to;

    if ( MAX_REMAPS <= numRemaps ) {
        err("Too many remap commands.\n");
    }

    s=skipspace(s);
    if(!*s) return;
    if(*s=='\'') {
        from = ( unsigned char )*++s;
        s++;
    }
    else if (isdigit(*s)) {
        from = atoi( s );
        while(isdigit(*s))
            s++;
    }
    else {
        return;
    }
    s = skipspace(s);
    if(*s=='\'') {
        to = ( unsigned char )*++s;
    }
    else if (isdigit(*s)) {
        to = atoi( s );
    }
    else {
        return;
    }
    
    remapfrom[ numRemaps ] = from;
    remapto[ numRemaps ]   = to;
    numRemaps++;
}

static int parseline(char *s)
/* returns 1 if it's a glyph or grayheader line */
{
    int i;
    int v;
    s = skipspace(s);
    while(isspace(*s) && *s) s++;
    if(*s=='/') return 0;
    for(i=0;i<NUM_VALUES;i++)
    {
        if(!STRNICMP(values[i].fieldname,s,strlen(values[i].fieldname)))
        {
            sscanf(s+strlen(values[i].fieldname),"%d",&v);
            *(values[i].location)=v;
            values[i].have=1;
            return 0;
        }
    }
    if(!STRNICMP(s,"remap",5) && isspace(s[5])) {
        handleremap(skipspace(s+5));
    }
    if( ( !STRNICMP(s,"glyph",5) && s[5] ) || !STRNICMP(s,"grayHeader",10) )
        return 1;
#if 0
    if(!STRNICMP(s,"grayMap",7)) {
        if ( ! quiet )
            err("Grayscale not supported.\n");
        else
            longjmp( exitJump, 1 );
    }
#endif

    return 0;
}


static void dodefault(int valuenum,Word value)
{
    if(!values[valuenum].have) *(values[valuenum].location)=value;
}

static void parse(char *s,struct fonttype *f)
{
    f->includelores=f->text=f->infoOnly=0;
    switch(s[0])
    {
        case 't':
            f->text=1;
            f->type2=0;
            f->resolution=LORES;
            break;
        case 'i':
            f->text=1;
            f->type2=0;
            f->resolution=LORES;
            f->infoOnly=1;
        case 'H':
            f->resolution=HIRES;
            f->type2=1;
            break;
        case 'L':
            f->resolution=LORES;
            f->type2=1;
            break;
        case 'l':
            f->resolution=LORES;
            f->type2=0;
            break;
        case '2':
            f->resolution=HIRES;
            f->type2=1;
            f->includelores=1;
            break;
        case '1':
            f->resolution=LORES;
            f->type2=1;
            f->includelores=0;
            break;
        default:
            err("Font type must be H or L or l rather than %s.\n",s);
    }
    if(s[1]=='a') f->afnx=1; else f->afnx=0;
}

static void detect(FILE *f,char *s,struct fonttype *t,char *mods)
{
    int c,c2;
    if(f == stdin)
    {
        t->afnx=t->type2=t->includelores=0;
        t->resolution=LORES;
        t->text=1;
        return;
    }
    c=fgetc(f);
    if(c==0x90)
    {
        t->afnx=0;
        t->resolution=LORES;
        t->type2=0;
        t->text=t->includelores=0;
    }
    else if(c==0x92)
    {
        t->afnx=0;
        t->resolution=(tolower(mods[0])=='l')?LORES:HIRES;
        t->type2=1;
        t->text=t->includelores=0;
    }
    else
    {
        c2=fgetc(f);
        if(c2==0x92)
        {
            t->afnx=1;
            t->resolution=(tolower(mods[0])=='l')?LORES:HIRES;
            t->type2=1;
            t->text=t->includelores=0;
        }
        else
        {
            int text;
            if(isspace(c))
            {
                if(!isspace(c2) && !isalpha(c2) && c2!='/') text=0;
                else text=1;
            }
            else if(c=='/')
            	text=(c2=='/');
            else if(isalpha(c))
            	text=isalpha(c2);
            else
                text=0;
            if(text)
            {
                t->afnx=t->type2=t->includelores=0;
                t->resolution=LORES;
                t->text=1;
            }
            else
                err("Error in font file!");
        }
    }
    if ( ! quiet ) {
        printf("// %s: ",s);
        if(t->afnx) printf("afnx ");
        if(t->resolution==HIRES) printf("hires ");
        if(t->resolution==LORES) printf("lores ");
        if(t->text) printf("text ");
        if(t->type2) printf("FontTypeV2");
        putchar('\n');
    }    
    rewind(f);
}

static void myread(void *p,int length,FILE *f)
{
    int l=fread(p,1,length,f);
    if(l!=length) err("Error reading input file.\n");
    return;
}

static void fixalign2out(FILE *f)
{
    Word w=0;
    if(out.afnx)
        fwrite(&w,2,1,f);
}

static void *mymalloc(unsigned length)
{
    void *p;
    if ( MAX_MYMALLOCS <= numMemoryPointers )
        err("Too many allocations!\n");
    p=malloc(length);
    if(NULL==p) {
        err("Out of memory!\n");
    }
    else {
        Byte* q = p;
        
        while( length-- )
            *q++ = 0;
    }
    memoryPointers[ numMemoryPointers++ ] = p;
    return p;
}

static void myseek(FILE *f,int position)
{
    if(fseek(f,position,SEEK_SET)) err("Error seeking to %d.\n",position);
}

static int getpixel(int x, int y)
{
    Word col,mask;
    col=x/8;
    mask=1<<(7-(x%8));
    return !!(mask&glyph[y*header.rowWords*2+col]);
}

static int getcharwidth(int c)
{
    int i;
    unsigned l;
    if(chardata[c]==NULL) return -1;
    if(widths[c]>=0) return widths[c];
    for(l=i=0;i<(int)header.fRectHeight;i++)
        if(strlen(chardata[c][i])>l) l=strlen(chardata[c][i]);
    return l;
}

static void putpixel(int x, int y, int c)
{
    Word col,mask;
    col=x/8;
    mask=1<<(7-(x%8));
    if(c)
    {
        glyph2[y*rowWords2*2+col]|=mask;
        if(doubleputpixel)
        {
            x/=2;
            y/=2;
            col=x/8;
            mask=1<<(7-(x%8));
            glyph3[y*rowWords2+col]|=mask;
        }
    }
}

static int getglyphnumber( char *s, int* isGrayHeader )
{
    int glyph;
    int i;

    *isGrayHeader = 0;

    s=skipspace(s);
    if ( !STRNICMP( s, "grayHeader", 10 ) ) {
        char* p;
        int   value;
        int   i;


        s = skipspace( s + 10 );

        if ( strlen( s ) < 4 ) {
            return -1;
        }

        p = grayHeader;

        for ( i = 0 ; i < GRAY_HEADER_LENGTH ; i++ )
             p[ i ] = 0;

        memcpy( p, "Gray", 4 );

        p += 4;
        
        memcpy( p, s, 4 );

        s += 4;
        p += 4;

        s = skipspace( s );

        if ( strlen( s ) < 4 ) {
            return -1;
        }

        sscanf( s, "%x", &value );

        s += 4;

        *p++ = value >> 8;
        *p++ = value & 0xFF;

        s = skipspace( s );

        if ( ! *s ) {
            return -1;
        }

        if ( 32 <= strlen( s ) ) {
            return -1;
        }

        strncpy( p, s, 32 );
        
        if ( 32 <= strlen( s ) )
            p[ 31 ] = 0;

        *isGrayHeader = 1;

        return 0;
    }
    if ( STRNICMP( s,"glyph",5 ) )
        return -1;
    s+=5;
    s=skipspace(s);
    if(!*s) return -1;
    if(*s=='\'')
        glyph = ( unsigned char )s[1];
    else
        glyph = atoi(s);
    for ( i = 0 ; i < numRemaps ; i++ ) {
        if ( glyph == remapfrom[ i ] ) {
            glyph = remapto[ i ];
            break;
        }
    }
/*
    if ( i == numRemaps ) {
        for ( i = 0 ; i < numRemaps ; i++ ) {
            if ( glyph == remapto[ i ] ) {
                return SKIP_GLYPH;
            }
        }
    }
 */

    return glyph;
}

static char *trim(char *s)
{
    char *t;
    t=strstr(s,"//");
    if(t!=NULL) *t=0;
    while(*s&&isspace(*s)) s++;
    if(!*s) return "";
    t=s+strlen(s)-1;
    while(*t&&isspace(*t)&&t>=s) *t--=0;
    if(t<s) return "";
    return s;
}

static void usage(void)
{
        err("fontconv [-l] inputname [-i lowresfont] H|Ha|2|2a|1|1a|l|t|i outputname\n"
   	      "This converts a Palm font into the specified output format.\n"
   	      " -q: Quiet mode\n"
   	      " -l: Take the low-res (default: hi-res) part of the FontTypeV2 font\n"
   	      " -i lowresfont: Use this low-res font to fill out the low-res portion of\n"
   	      "     output.  Must be used in conjunction with output options 2 or 2a.\n"
          "  H: hi-res portion of FontTypeV2 font\n"
          "  L: lo-res portion of FontTypeV2 font\n"
          " Ha: hi-res portion of FontTypeV2 font in afnx format\n"
          " La: lo-res portion of FontTypeV2 font in afnx format\n"
          "  l: version 1 type font\n"
          "2/2a: put the font into the hi-res portion of FontTypeV2 font (afnx for 2a)\n"
          "     and fill out the low-res portion with scaled or -i specified font\n"
          "1/1a: put the font into lo-res portion of FontTypeV2 font (afnx for 1a)\n"
          "  t: text-editable/readable format\n"
          "  i: output font header and other font information\n"
          "The inputname/outputname can be - for stdin/stdout for text format only.\n"
          "To take a lo-res font and convert it into an OS5 hi-res font, you can run:\n"
          "   fontconv input.bin H output.bin\n"
          "If you want the OS5 font to have a lo-res bitmap (a very ugly and\n"
          "unreadable one) you can do:\n"
          "   fontconv input.bin 2 output.bin.");
}

static DWord findresolution(FILE *f,Word resolution,int afnx)
{
    int olddoreverse;
    int i;
    DWord pos;
    Word densityCount,r;

    olddoreverse=doreverse;
    doreverse=endiancheck(!afnx);

    myseek(f,(Byte*)&header.densityCount-(Byte*)&header);
    myread(&densityCount,2,f);
    densityCount=reverse_word(densityCount);

    if(afnx){ fgetc(f); fgetc(f); }
    
    for(i=0;i<densityCount;i++)
    {
        myread(&r,2,f);
        r=reverse_word(r);
        if(afnx){ fgetc(f); fgetc(f); }
        myread(&pos,4,f);
        pos=reverse_dword(pos);
        if(r==resolution) break;
    }
    if(i==densityCount) err("Cannot find desired resolution.");

    doreverse=olddoreverse;
    return pos;
}
 
#ifdef FONTCONV_LIB
int DoFontConv( int argc, char** argv )
#else
int main(int argc, char **argv)
#endif
{
    FILE *f,*g;
    char *inopt;
    char *lorespart=NULL;
    int i,j,k,delta,maxwidth;
    int x,x2;
    int argpos;
    int c;
    Word w;
    Word position;
    char linebuf[1024];
    char *s;
    int  fromStdin;
    int  value;

    initialize();

    value = setjmp( exitJump );
    if ( value ) {
        cleanup();
        return value;
    }

    if(argc<4 || argc>9) usage();
    if(argv[1][0]=='-' && argv[1][1])
    {
        inopt=argv[1]+1;
        argpos=2;
        if(argc<5) usage();
    }
    else
    {
        argpos=1;
        inopt="";
    }
    if ( !strcmp( argv[argpos], "-" ) ) {
        fromStdin = 1;
        f         = stdin;
    }
    else {
        fromStdin = 0;
        f         = myfopen(argv[argpos],"rb");
    }
    detect(f,argv[argpos],&in,inopt);
    if ( ! in.text && fromStdin )
        err("Cannot feed non-text file via stdin.\n");
    if( ! fromStdin ) {
        myfclose(f);
        f=myfopen(argv[argpos],in.text?"r":"rb");
    }
    argpos++;
    while(argv[argpos][0]=='-')
    {
        if(argv[argpos][1]=='i')
        {
            if(argv[argpos][2])
                lorespart=argv[argpos]+2;
            else
                lorespart=argv[++argpos];
            argpos++;
        }
        else if ( argv[argpos][1]=='m' )
        {
        	char*  p;

        	if(argv[argpos][2]) {
        	    p = argv[argpos]+2;
        	}
        	else {
        	    p = argv[++argpos];
        	}
        	argpos++;
        }
        else if ( argv[argpos][1]=='f' )
        {
            fakeFont = 1;
            argpos++;
        }
        else if ( argv[argpos][1]=='h' )
        {
            half = 1;
            argpos++;
        }
        if(argpos+1>=argc) usage();
    }
    parse(argv[argpos++],&out);
    doreverse=endiancheck(!in.afnx);
    if(in.text)
    {
        while(NULL!=fgets(linebuf,1024,f) && !parseline(linebuf));
        if(feof(f)) err("No font data!");
        if(!values[LfRectHeight].have) err("Need fRectHeight in font.");
        for(i=0;i<256;i++) { widths[i]=-1; }
        do
        {
        	int isGrayHeader;
        	s=trim(linebuf);
            c=getglyphnumber(s,&isGrayHeader);
            if(c<0) continue;
            if(c>255) err("Only 8-bit fonts are supported.");

            if ( isGrayHeader ) {
                int byteWidth;
                unsigned char* in;
                int height;
                int doDouble;

                doDouble = ( out.resolution == LORES && out.type2 && ! fakeFont && half );

                height = doDouble ? header.fRectHeight * 2 : header.fRectHeight;

                if ( out.resolution == LORES && fakeFont && height % 2 ) {
                    height++;
                }

                byteWidth = ( GRAY_HEADER_LENGTH + height - 1 ) / height;

                if ( 127 < byteWidth ) {
                    err("No room for gray data header.  Need font with greater height.");
                }

                chardata[c] = mymalloc( header.fRectHeight * sizeof( char* ) );

                in = grayHeader;

                widths[c] = byteWidth * ( doDouble ? 4 : 8 );

                for ( j = 0 ; j < header.fRectHeight ; j++ ) {
                     char* q;
                     char* q0;
                     int   k;

                     q0 = q = chardata[c][j] = mymalloc( byteWidth * 8 + 1 );

                     for ( k = 0 ; k < byteWidth ; k++ ) {
                          int bit;

                          if ( in < grayHeader + GRAY_HEADER_LENGTH ) {
                              for ( bit = 7 ; 0 <= bit ; bit-- ) {
                                  if ( q - q0 < widths[ c ] )
                                      *q++ = ( *in & ( 1 << bit ) ) ? '#' : '-';
                              }
                              in++;
                          }
                          else {
                              for ( bit = 0 ; bit < 8 ; bit++ ) {
                                  if ( q - q0 < widths[ c ] )
                                      *q++ = '-';
                              }
                          }
                     }
                     *q = 0;
                }

                continue;
            }

            chardata[c]=mymalloc( header.fRectHeight * sizeof (char*) );
            for(j=0;j<header.fRectHeight;j++)
            {
                if(NULL==fgets(linebuf,1024,f)) err("Premature file end in character %d.",c);
                s=trim(linebuf);
                if(!*s)
                    j--;
                else if(!STRNICMP(s,"width",5) && strlen(s)>6)
                {
                    widths[c]=atoi(s+6);
                    j--;
                }
                else if(*s)
                {
                    chardata[c][j]=mymalloc( strlen(s)+1 );
                    strcpy(chardata[c][j],s);
                }
            }
	   } while(NULL!=fgets(linebuf,1024,f));

	   if ( ! fromStdin )
           myfclose(f);

       for(i=0;i<256;i++) {
           widths[i]  = getcharwidth(i);
           heights[i] = header.fRectHeight;
           ydelta[i]  = 0;
       }

       for(i=0;i<256;i++) {
           if(0<=widths[i])
               break;
       }
	   if(i==256) err("Empty file!");
	   header.firstChar=i;
	   for(i=255;i>header.firstChar;i--)
	       if(widths[i]>=0) break;
	    header.lastChar=i;
//        if ( header.firstChar < 22 )
//            header.firstChar = 22;
        numChars=header.lastChar-header.firstChar+1;
        position=0;
        maxwidth=0;
        owTable=mymalloc(2+2*numChars);
        locationTable=mymalloc(4+2*numChars);
        dodefault(Ldescent,0);
        dodefault(Lascent,header.fRectHeight-header.descent);
        for(i=0;i<numChars;i++)
        {
            locationTable[i]=position;
            owTable[i].width=widths[i+header.firstChar];

            if ( owTable[i].width == -1 ) {
                int height;
                int width;
                int numLines;
                char** icon;

                width  = 0;

                icon   = FindIcon( i + header.firstChar, &width, header.ascent, &numLines,
                             out.type2 && ( out.resolution == LORES && ! fakeFont ) && half,
                             out.type2 && ( out.resolution == HIRES || fakeFont ) );

                if ( icon != NULL ) {
                    owTable[i].width = width;
                    widths[i]        = width;
                    heights[i]       = numLines;
                    ydelta[i]        = header.ascent - numLines;
                    chardata[i + header.firstChar]      = icon;
                }
            }

            if ( owTable[i].width != -1 )
            {
        	     position+=owTable[i].width;
        	     owTable[i].offset=0;
        	     if(owTable[i].width>maxwidth) maxwidth=owTable[i].width;
            }
            else {
                 owTable[i].offset = -1;
            }
        }
        locationTable[i]=position;
        dodefault(LfRectWidth,maxwidth);
        dodefault(LmaxWidth,maxwidth);
        dodefault(LfontType,0x9000);
        header.kernMax=header.nDescent=header.leading=0;
        rowWords2=header.rowWords=(position+15)/16;
        glyphLength=glyphLength2=header.rowWords*2*header.fRectHeight;
        fRectHeight2=header.fRectHeight;
        glyph2=glyph=mymalloc(glyphLength);
        for(i=0;i<(int)glyphLength;i++) glyph[i]=0;
        for(i=0;i<numChars;i++)
        {
//fprintf(stderr,"%d %d - %d %d\n", i + header.firstChar, header.fRectHeight, heights[ i ],ydelta[ i ] );
            if(locationTable[i]==locationTable[i+1]) continue;
            for(j=0;j<(int)header.fRectHeight && j<heights[i];j++)
            {
             	 for(x=0;x<(int)header.fRectWidth && chardata[i+header.firstChar][j][x] ; x++)
              	 {
                	 	putpixel(locationTable[i]+x,j+ydelta[i],chardata[i+header.firstChar][j][x]=='#');
              	 }
            }
        }
    }
    else
    {
        myread(&header,SHORTHEADER*2,f);
        if(in.type2) myread(&header.version,sizeof header-(SHORTHEADER*2),f);
        reverse_word_array(&header,sizeof header);
        if((!!(header.fontType & 0x200)) != in.type2)
            err("Actual input file type does not match specified file type.");
        glyphLength=header.rowWords*2*header.fRectHeight;
        if(!in.type2)
        {
            glyphBitsOffset=SHORTHEADER*2;
            locationTableOffset=glyphBitsOffset+glyphLength;
        }
        else
        {
            if ( in.resolution == HIRES )
                glyphLength*=4;
            if(header.version!=1) err("Error in font.");
            glyphBitsOffset=findresolution(f,in.resolution,in.afnx);
            if(in.afnx)
                locationTableOffset=sizeof header + 2 + header.densityCount*8;
            else
                locationTableOffset=sizeof header + header.densityCount*6;
        }
        myseek(f,glyphBitsOffset);
        glyph=mymalloc(glyphLength);
        myread(glyph,glyphLength,f);
        myseek(f,locationTableOffset);
        numChars=header.lastChar-header.firstChar+1;
        locationTable=mymalloc(2*(numChars+2));
        myread(locationTable,2*(numChars+2),f);
        reverse_word_array(locationTable,2*(numChars+2));
        myseek(f,2*header.owTLoc+OWT_DELTA);
        owTable=mymalloc(numChars*2+2);
        myread(owTable,2*numChars+2,f);
        
//        for ( i = 0 ; i < numChars + 1 ; i++ ) {
//             printf( "// %d : %d <%d,%d> %d\n", i, locationTable[ i ], owTable[ i ].offset, owTable[ i ].width, locationTable[ i + 1 ] );
//        }

        if ( ! fromStdin )
            myfclose(f);
        if(in.resolution==HIRES)
        {
            for(i=0;i<numChars;i++)
            {
                locationTable[i]*=2;
                if(owTable[i].width!=-1) owTable[i].width*=2;
            }
            locationTable[i]*=2;
            header.fRectHeight*=2;
            header.fRectWidth*=2;
            header.maxWidth*=2;
            header.descent*=2;
            header.ascent*=2;
            header.rowWords*=2;
            header.fontType&=~0x200;
        }
    }

    /* input complete, time for output */
    if ( !strcmp( argv[argpos], "-" ) ) {
        if ( ! out.text )
            err( "Can only send text data to stdout." );
        g = stdout;
    }
    else {
        g=fopen(argv[argpos],out.text?"w":"wb");
    }
    doreverse=endiancheck(!out.afnx);
    if(!out.text && out.type2)
    {
    	if ( out.resolution == LORES && out.includelores ) {
    	    err("Invalid output option combination.");
    	}

        header.fontType|=0x200;

        locationTable2=mymalloc(2*numChars+4);
        owTable2=mymalloc(2+2*numChars);
        if ( out.resolution == HIRES ) {
            position=0;
            fRectHeight2   = (header.fRectHeight+1)/2*2;
            for(i=0;i<numChars;i++)
            {
                locationTable2[i]=position;
                owTable2[i].offset=owTable[i].offset;
                if(owTable[i].width>=0) {
                    owTable2[i].width=(owTable[i].width+1)/2*2;
                    position += owTable2[ i ].width;
                }
                else {
                    owTable2[i].width=-1;
                    owTable2[i].offset=-1;
                }
//                position+=(locationTable[i+1]-locationTable[i]+1)/2*2;
            }
            locationTable2[i]=position;
            position += ( fRectHeight2 / 4 + 1 ) * 2;
            locationTable2[i+1]=position;
//printf("//%d %d\n",locationTable2[i ],locationTable2[i+1 ] );
            owTable2[i].offset=0;
            owTable2[i].width=locationTable2[i+1]-locationTable2[i];
            rowWords2    = (position+31)/32*2;
            glyph2=mymalloc(glyphLength2=rowWords2*2*fRectHeight2);
        }
        else {
            for ( i = 0 ; i < numChars ; i++ ) {
                locationTable2[i] = locationTable[i];
                owTable2[i]       = owTable[i];
            }
            owTable2[numChars].offset = 0;
            if ( fakeFont && header.fRectHeight % 2 ) {
                header.fRectHeight++;
            }
            owTable2[numChars].width = header.fRectHeight / 2 + 1;
            if ( fakeFont ) {
                header.fRectHeight = 1;
                header.ascent = 1;
                header.descent = 0;
            }
            locationTable2[numChars]     = locationTable2[numChars - 1] + owTable2[numChars-1].width;
            locationTable2[numChars + 1] = locationTable2[numChars] +
                                               owTable2[numChars].width;
            rowWords2    = (locationTable2[numChars+1]+15)/16;
            fRectHeight2 = header.fRectHeight;
            glyphLength2 = rowWords2 * 2 * fRectHeight2;
            glyph2       = mymalloc(glyphLength2);
        }
        if(out.includelores)
        {
            glyph3=mymalloc(glyphLength2/4);
            if(lorespart==NULL) doubleputpixel=1;
            else
            {
                FILE *h;
                struct fonttype t;
                DWord pos;
                doubleputpixel=0;
                h=myfopen(lorespart,"rb");
                detect(h,lorespart,&t,"l");
                if(t.resolution != LORES || t.text) err("Wrong included low-res font type.");
                if(t.type2)
                    pos=findresolution(h,t.resolution,t.afnx);
                else
                    pos=SHORTHEADER*2;
                myseek(h,pos);
                myread(glyph3,glyphLength2/4,h);
                myfclose(h);
            }
        }
        else
            doubleputpixel=0;
        for(i=0;i<(int)glyphLength2;i++) glyph2[i]=0;
        if(doubleputpixel)
            for(i=0;i<(int)glyphLength2/4;i++) glyph3[i]=0;
        for(i=0;i<numChars;i++)
        {
            x2=locationTable2[i];
            for(x=locationTable[i];x<locationTable[i+1];x++,x2++)
            {
                for(k=0;k<header.fRectHeight;k++) {
                    putpixel(x2,k,getpixel(x,k));
                }
            }
        }
        if ( out.resolution==HIRES ) {
            header.maxWidth=(header.maxWidth+1)/2;
            for(i=0;i<=numChars;i++)
            {
             locationTable2[i]/=2;
             if ( 0 <= owTable2[ i ].width )
                 owTable2[ i ].width/=2;
            }
            locationTable2[i]/=2;
            header.rowWords=rowWords2/2;
            if(header.fRectHeight % 2)
              header.descent++;
            if(header.descent % 2)
            {
                header.ascent++;
                header.descent--;
            }
            header.maxWidth=(header.maxWidth+1)/2*2;
            header.fRectHeight=fRectHeight2/2;
            header.fRectWidth=(header.fRectWidth+1)/2;
            header.ascent/=2;
            header.descent/=2;
            header.version=1;
            header.densityCount=1+!!out.includelores;
        }
        else {
            header.version=1;
            header.densityCount=1;
            header.rowWords=rowWords2;
        }
        glyphBitsOffset=sizeof header+(out.afnx?8:6)*header.densityCount+4*numChars+2+2+2+(out.afnx?2:0);
        header.owTLoc=((sizeof header+(out.afnx?8:6)*header.densityCount+2*numChars+2+2+(out.afnx?2:0)) - OWT_DELTA)/2;
        reverse_word_array(&header,sizeof header);
        fwrite(&header,1,sizeof header,g);
        fixalign2out(g);
        if(out.includelores)
        {
            w=reverse_word(LORES);
            fwrite(&w,1,2,g);
            fixalign2out(g);
            glyphBitsOffset=reverse_dword(glyphBitsOffset);
            fwrite(&glyphBitsOffset,1,4,g);
            glyphBitsOffset=reverse_dword(glyphBitsOffset)+glyphLength2/4;
        }
        w=reverse_word( out.resolution );
        fwrite(&w,1,2,g);
        fixalign2out(g);
        glyphBitsOffset=reverse_dword(glyphBitsOffset);
        fwrite(&glyphBitsOffset,1,4,g);
        reverse_word_array(locationTable2,(numChars+2)*2);
        fwrite(locationTable2,numChars+2,2,g);
        fwrite(owTable2,numChars+1,2,g);
        if(out.includelores)
            fwrite(glyph3,glyphLength2/4,1,g);
        fwrite(glyph2,glyphLength2,1,g);
    }
    else if(out.text)
    {
        long avgWidth;
        long totalWidth;
        int  charCount;
        header.fontType &= ~0x200;
        totalWidth = 0;
        charCount  = 0;
        for ( i = 0 ; i < numChars ; i++ ) {
            if ( 0 < owTable[ i ].width ) {
                totalWidth += owTable[ i ].width;
                charCount++;
            }
        }
        if ( charCount == 0 )
            avgWidth = 0;
        else
            avgWidth = 100 * totalWidth / charCount;
        if ( ! out.infoOnly )
            fprintf(g,"// generated by fontconv\n");
        fprintf(g,
              "fontType %d\n"
              "firstChar %d\n"
              "lastChar %d\n"
              "maxWidth %d\n"
              "kernMax %d\n"
              "nDescent %d\n"
              "fRectWidth %d\n"
              "fRectHeight %d\n"
              "ascent %d\n"
              "descent %d\n"
              "leading %d\n"
              "averageWidth %ld.%02ld\n",
                header.fontType,header.firstChar,
                header.lastChar,header.maxWidth,header.kernMax,
                header.nDescent,header.fRectWidth,header.fRectHeight,
                header.ascent,header.descent,header.leading,
                avgWidth / 100,avgWidth % 100);
        if ( out.infoOnly ) {
            cleanup();
            return 0;
        }
        fputc('\n',g);

        for(i=0;i<numChars;i++)
        {
            fprintf( g, "// WIDTH %d (OFFSET %d, DELTA %d)\n", owTable[ i ].width, owTable[ i ].offset,locationTable[i+1]-locationTable[i] );
            if(owTable[i].width<=0 || locationTable[i+1]==locationTable[i]) continue;
            c=i+header.firstChar;
            if(c>' ' && c<127) fprintf(g,"GLYPH '%c'\n",c);
            else fprintf(g,"GLYPH %u\n",(unsigned char)c);
            for(j=0;j<header.fRectHeight;j++)
            {
                for(k=0;k<owTable[i].width-(locationTable[i+1]-locationTable[i]);k++)
                    fputc('-',g);
                delta=k;
                for(;k<owTable[i].width;k++)
                    fputc(getpixel(locationTable[i]+k-delta,j)?'#':'-',g);
                fputc('\n',g);
            }
            fputc('\n',g);
        }
    }
    else
    {
        header.fontType &= ~0x200;
        if ( fakeFont ) {
            header.fRectHeight = 1;
            glyphLength = header.rowWords * 2;
            header.ascent = 1;
            header.descent = 0;
        }
        reverse_word_array(locationTable,2*(numChars+1));
        header.owTLoc = (2*SHORTHEADER+glyphLength+2+2*(numChars+1)-OWT_DELTA)/2;
        reverse_word_array(&header,2*SHORTHEADER);
        fwrite(&header,2,SHORTHEADER,g);
        fwrite(glyph,1,glyphLength,g);
        fwrite(locationTable,numChars+1,2,g);
        fwrite(locationTable+numChars,1,2,g);
        fwrite(owTable,numChars,2,g);
        fwrite("\0\0",1,2,g);
    }
    cleanup();
    return 0;
}

#ifdef FONTCONV_LIB
int FontConv( char* arg1, char* arg2, char* arg3, char* arg4, char* arg5, char* arg6,
              char* arg7, char* arg8 )
{
    char* argv[ 9 ];
    int   argc;

    argv[ 0 ] = "FontConv_library_call";
    argv[ 1 ] = arg1;
    argv[ 2 ] = arg2;
    argv[ 3 ] = arg3;
    argv[ 4 ] = arg4;
    argv[ 5 ] = arg5;
    argv[ 6 ] = arg6;
    argv[ 7 ] = arg7;
    argv[ 8 ] = arg8;
    
    quiet = 1;

    for ( argc = 0 ; argc < 9 ; argc++ ) {
         if ( argv[ argc ] == NULL )
             break;
    }
    return DoFontConv( argc, argv );
}
#endif
