#
# $Id: Makefile,v 1.16 2004/06/02 13:02:34 arpruss Exp $
# Makefile for palmfontconv project
#

SHELL           = /bin/sh
EXE             = .exe

FONTCONV_SRC    = fontconv.c
FONTCONV        = ./fontconv$(EXE)

TOPALMTEXT_SRC  = topalmtext.c fontconv.c
TOPALMTEXT      = ./topalmtext$(EXE)

TOGRAY_SRC      = togray.c fontconv.c
TOGRAY          = ./togray$(EXE)

TOPRC_SRC	= toprc.c
TOPRC		= ./toprc$(EXE)

FSNAME_SRC	= fsconvert.c
FSNAME		= ./fsconvert$(EXE)

TOREMAP_SRC	= toremap.c
TOREMAP		= ./toremap$(EXE)

EXTRACTFONT_SRC = extractfont.c fontconv.c
EXTRACTFONT	= ./extractfont$(EXE)

CFLAGS		= -O3 -Wall 

all: $(FONTCONV) $(TOPALMTEXT) $(TOGRAY) $(TOPRC) $(TOREMAP) $(FSNAME) $(EXTRACTFONT)

$(FSNAME): $(FSNAME_SRC) 
	gcc $(CFLAGS) -o $(FSNAME) $(FSNAME_SRC)

$(FONTCONV): $(FONTCONV_SRC) palmtypes.h
	gcc $(CFLAGS) -o $(FONTCONV) $(FONTCONV_SRC)

$(TOPRC): $(TOPRC_SRC) palmtypes.h
	gcc $(CFLAGS) -o $(TOPRC) $(TOPRC_SRC)

$(TOGRAY): $(TOGRAY_SRC) palmtypes.h
	gcc -DFONTCONV_LIB $(CFLAGS) -o $(TOGRAY) $(TOGRAY_SRC)

$(EXTRACTFONT): $(EXTRACTFONT_SRC) fontconv.c
	gcc -DFONTCONV_LIB $(CFLAGS) -o $(EXTRACTFONT) $(EXTRACTFONT_SRC)

$(TOREMAP): $(TOREMAP_SRC)
	gcc $(CFLAGS) -o $(TOREMAP) $(TOREMAP_SRC)

$(TOPALMTEXT): $(TOPALMTEXT_SRC) encodings.c
	gcc $(CFLAGS) -DFONTCONV_LIB -o $(TOPALMTEXT) `freetype-config --cflags` $(TOPALMTEXT_SRC) `freetype-config --libs`

clean:
	rm *.o *.exe *.bin topluckergray-*.txt
