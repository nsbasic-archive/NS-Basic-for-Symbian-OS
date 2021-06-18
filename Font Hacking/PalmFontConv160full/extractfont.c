#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "fontconv.h"

#define SHORT_HEADER 13

typedef unsigned char byte;

#ifdef __GNUCC__
#define STRICMP strcasecmp
#else
#define STRICMP stricmp
#endif

#define resourceAttribute  0x0001

int prc;

unsigned long rdatapos;
unsigned long rmappos;
unsigned long rdatalet;
unsigned long rmaplen;
unsigned long rtypepos;
unsigned long rnamepos;
byte numtypes;
unsigned int myresnum;
unsigned long myreflistpos;
unsigned long mynamepos;
unsigned long mydatapos;
unsigned long rdatalen;
char thename[1000];
FILE *in = NULL;
FILE *out = NULL;

#define MAX_NAME_BASES 1000

char nameBases[ MAX_NAME_BASES ][ 1100 ];
int  numNameBases = 0;
int  counts[ MAX_NAME_BASES ];

void err(char *s, ...)
{
#ifndef DLL
    va_list argptr;
    va_start(argptr,s);
    vfprintf(stderr,s,argptr);
    va_end(argptr);
#endif
#ifdef DLL
    longjmp(ExitJump,2);
#else
    if ( in != NULL )
        fclose( in );
    if ( out != NULL )
        fclose( out );
    exit(1);
#endif
}

FILE *myfopen(char *s,char *m)
{
    FILE *f=fopen(s,m);
    if(f==NULL) err("Cannot open %s.\n",s);
    return f;
}

int myfgetc(FILE *f)
{
    int c=fgetc(f);
    if(c==-1) err("Error reading font!");
    return c;
}

void myseek(FILE *f, unsigned long x)
{
    if(fseek(f,x,SEEK_SET)<0 ||
        ftell(f)!=(long)x) err("Error seeking!");
    return;
}

unsigned char getbyte(FILE *f)
{
    int c;
    if(-1==(c=fgetc(f))) err("Error reading file.\n");
    return c;
}

unsigned short getword(FILE *f)
{
    unsigned short x;
    x=getbyte(f);
    return x*256+getbyte(f);
}

unsigned long getdword(FILE *f)
{
    unsigned short x;
    x=getword(f);
    return x*65536l+getword(f);
}

unsigned long get3bytes(FILE *f)
{
    unsigned short x;
    x=getbyte(f);
    return x*65536l+getword(f);
}

void read4bytes(FILE *f,char *s)
{
    int i;
    for(i=0;i<4;i++) s[i]=getbyte(f);
    s[4]=0;
}

void process(FILE *f,long len,unsigned long datapos,char* rsrc,unsigned short rnum)
{
    char nameBase[ 1000 ];
    char name[1040];
    char name2[1040];
    byte header[ SHORT_HEADER * 2 ];

    if ( *rsrc && strcmp( rsrc, "nfnt" ) && strcmp( rsrc, "afnx" ) && strcmp( rsrc, "NFNT" ) &&
         strcmp( rsrc, "xFnt" ) )
        return;

    myseek(f,datapos);
    if ( len < SHORT_HEADER * 2 )
        return;
    fread( header, 1, SHORT_HEADER * 2, f );
    len -= SHORT_HEADER;
//printf(" %d %x %x %x %x \n",!*rsrc, header[ 0 ], header[ 1] , header[ 10 ] & ~2, header[ 11 ] );
    if ( !*rsrc && ( header[ 0 ] & ~4 ) == 0 && header[ 1 ] == 0 && ( header[ 10 ] & ~2 ) == 0x90 && header[ 11 ] ==0 ) {
       /* could be fontbucket font */
        int i;

        for ( i = 0 ; i < SHORT_HEADER * 2 - 10; i++ ) {
             header[ i ] = header[ i + 10 ];
        }
        if ( len < 10 )
            return;
        fread( header + SHORT_HEADER * 2 - 10, 1, 10, f );
    }

    if ( ( header[ 0 ] == 0x90 && header[ 1 ] == 0x00 ) || ( header[ 0 ] == 0x92 && header[ 1 ] == 0x00 ) ||
         ( header[ 0 ] == 0x00 && header[ 1 ] == 0x92 ) ) {
        int afnx;
        int height;
        int errvalue;
        int i;
        int v2 = ! ( header[ 0 ] == 0x90 );

        int newType;

        afnx = ( header[ 0 ] == 0 );

        newType = ( afnx || header[ 0 ] == 0x92 );

        if ( ! afnx ) {
            height = header[ 0xE ] * 256 + header[ 0xF ];
        }
        else {
            height = header[ 0xF ] * 256 + header[ 0xE ];
        }

        if ( newType )
            height *= 2;

        if ( *rsrc != 0 ) {
            sprintf(nameBase,"%s-%d-%s%04x",thename,height,rsrc,rnum);
        }
        else {
            sprintf(nameBase,"%s-%d-%s",thename,height,header[ 0 ]==0x90 ? "v1" : "v2");
        }
        for ( i = 0 ; i < numNameBases ; i ++ ) {
             if ( ! STRICMP( nameBases[ i ], nameBase ) ) {
                 break;
             }
        }
        if ( i == numNameBases ) {
            if ( MAX_NAME_BASES <= numNameBases ) {
                err("Too many fonts!\n");
            }
            strcpy( nameBases[ numNameBases ], nameBase );
            counts[ numNameBases ] = 1;
            numNameBases++;
        }
        else {
            counts[ i ]++;
        }

        if ( counts[ i ] != 1 ) {
            sprintf( nameBase + strlen( nameBase ), "-%d", counts[ i ] );
        }

        sprintf( name, "%s.bin_efTemp",nameBase );

        out=myfopen(name,"wb");
        fwrite( header, 1, SHORT_HEADER * 2, out );
        while( len-- && !feof(f) )
        {
            int c = fgetc(f);
            if ( c == -1 )
                break;
            fputc(c,out);
        }
        fclose(out);
        out = NULL;

        if ( v2 ) {
            sprintf( name2, "%s-hires.txt", nameBase );
            errvalue = FontConv( name, "t", name2, NULL,  NULL, NULL, NULL, NULL );
            if ( ! errvalue )
                puts( name2 );
            sprintf( name2, "%s-lores.txt", nameBase );
            errvalue = FontConv( "-l", name, "t", name2, NULL, NULL, NULL, NULL );
            if ( ! errvalue )
                puts( name2 );
        }
        else {
            sprintf( name2, "%s.txt",nameBase );
            errvalue = FontConv( name, "t", name2, NULL,  NULL, NULL, NULL, NULL );
            if ( ! errvalue )
                puts(name2);
        }

        unlink( name );
    }
}

int main(int argc, char **argv)
{
    int i;
    char name[1024];
    char s[5] = "";
    char *p;
    unsigned long pos,pos2,pos3;
    if(argc!=2)
        err("extractfont filename\n"
            "  This extracts all the bitmapped font (NFNT/nfnt) resources\n"
            "  from the specified .prc or .pdb file.\n"
            "  Each resource found gets saved into a separate file with\n"
            "  filename ending in .bin.\n");
    strcpy(name,argv[1]);
    in=myfopen(name,"rb");
    strcpy(thename,name);
    if((p=strrchr(thename,'.'))!=NULL ) {
        char *q;
        q = strrchr(thename,':');
        if ( q == NULL || q < p ) {
            q = strrchr(thename,'/');
            if ( q == NULL || q<p ) {
                q= strrchr(thename,'\\');
                if ( q==NULL || q<p )
                    *p=0;
            }
        }
    }
    myseek(in,32);
    prc = !! ( resourceAttribute & getword(in) );
    myseek(in,0x4C);
    numtypes=getword(in);
    for(i=0;i<numtypes;i++)
    {
        unsigned short rnum;
        if ( prc ) {
            read4bytes(in,s);
            rnum = getword(in);
        }
        pos2=getdword(in);
        if ( ! prc ) {
            getword( in );
            getword( in );
        }
        pos=ftell(in);
        if(i<numtypes-1)
        {
        	if ( prc ) {
                getdword(in);
                getword(in);
            }
            pos3=getdword(in);
        }
        else
        {
            pos3=0x7FFFFFFF;
        }
        process(in,pos3-pos2,pos2,s,rnum);
        myseek(in,pos);
    }
    fclose(in);
    in = NULL;
    return 0;
}
