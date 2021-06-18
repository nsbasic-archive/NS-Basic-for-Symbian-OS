$Id: README.TXT,v 1.61 2005/02/05 16:10:07 arpruss Exp $

SEE LICENSE IN FILE: COPYING
AN ADDITIONAL LICENSE GRANT IS AVAILABLE IN: GRANT
MOREOVER, THE GRAYSCALE SUPPORT PalmOS CODE IS COVERED BY A BSD LICENSE THAT
ALLOWS USE IN COMMERCIAL PROJECTS.  SEE: grayfont.[ch] and palmbitmap.h

IMPORTANT NOTES:
    There is a "force even width" option.  Use this option only if you are
    generating non-anti-aliased fonts (otherwise the font will look uglier
    than it should).  You need to use this if you're generating fonts for
    OS5 use and are having spacing problems in your fonts.  It is a good
    option to keep active for hi-res non-anti-aliased fonts for OS5,
    though it should be turned off for Sony and Palm OS4 fonts.

    In toplucker.tcl, you can force, for instance, bold and italic fonts
    to match the non-bold Roman fonts in terms of vertical metrics which is
    useful if generating fonts for a program like Plucker that ignores
    baseline positions.  Some glyphs may be cut off then.

    Plucker ignores the Custom, Greek, Hebrew and Interlinear fonts in
    a package, so if you're using Plucker, don't bother to fill these in.
    PalmBible+ uses at most the Normal, Bold, Thin, Custom, Greek, Hebrew and
    Interlinear fonts, so you can leave the others blank.  In general, you
    can leave blank any fonts you don't want, and they'll default to built-in
    Plucker or PalmBible+ fonts or system fonts, as available.

    See also the KNOWN ISSUES section at the end.

    The binaries for this work are based on the FreeType 2 library.
    See www.freetype.org for more information, and download source code
    from http://sourceforge.net/project/showfiles.php?group_id=3157

    Thank you to Bryan Oakley for his ComboBox widget!



0. RUNNING ON WINDOWS AND OTHER GUI SYSTEMS

You need to istall tcl/tk on your system to run the Plucker, PalmBible+,
Fonts4OS/FontSubst, FonthackV and standardAA font generators.  Get it from
          http://www.activestate.com/Products/Download/Register.plex?id=ActiveTcl

Then you can just double-click on "toplucker", "toFonts4OS5", "toFontsmoother",
or "toFonthackV" to run the appropriate generator.

toFontsmoother has four output modes: "Fonts4OS5", "FontHackV" "Mobipocket" and
"Standard". The "Standard" mode is for applications that take FontHack format
fonts, e.g., eReader.  All these require Alexander Pruss's shareware
FontSmoother antialiased font hack.  In Mobipocket and Standard mode, two fonts
are generated, one with the specified name and the other with "-resource.prc"
appended.  You may want to fill out just one slot in Mobipocket and Standard
mode.  Note that in these modes, the first 26 characters of the Palm name must
be unique: you cannot have fonts whose names differ only in characters 27-31.
Note that in FontHackV mode it doesn't matter what goes in which slot, as only
the first filled slot matters.

PalmBible+ can read Plucker OS5 font packages, so you can just generate a
Plucker OS5 font package (extended by allowing for Hebrew and Greek) with 
"toplucker", and this is the recommended procedure.  But if you want to make a
package that works only with PalmBible+ and not Plucker, you can check the
appropriate checkbox.

For PalmBible+, you can either use Unicode fonts for the Hebrew and Greek, as long
as they are rich enough to include the characters, or you can use a non-Unicode
font that already has the right encoding (e.g., a GreekKeys font--but, note,
Athenian.ttf doesn't seem to work).  If you use a non-Unicode font that has the
right encoding, check the "Keep original encoding" box to keep PalmFontConv
from trying to convert from Unicode.  


NOTE: Microsoft core fonts Times New Roman, Courier and Arial include a Hebrew
range.  If you use fonts other than Microsoft's core fonts Times New Roman,
Arial or Courier for this, you will have to manually tweak the output to
include some precombined characters.

To get accented Greek, you may want to use Gentium:

      http://www.sil.org/~gaultney/gentium/


You don't need to know anything about the stuff below if this is all you want
to do.  However, you can also do advanced tweaking by specifying options to
topalmtext in the "options" box.  This is only for advanced users.  Do not use
if you don't know what you're doing.

As of PalmFontConv 1.30, all utilities accept not just TrueType and Type 1 and
other FreeType 2 supported fonts, but they also read Palm font resources,
either in pilrc-compatible text format or in binary format.  If the utility
does not allow specification of density, double density is used.  This means that
you can use Palm font resources from other sources as inputs to toplucker.tcl.

Moreover, you can run ripfonts.tcl on any Palm pdb or prc file in order to extract
all font bitmaps from that file (double density only in the case of FontTypeV2 fonts).
If the input file is dir/file.pdb, output fonts will be of the form:
   dir/file-N-*.txt
where N is the height in pixels and * is a whole bunch of unique identifying
information.  These text readable files are pilrc-compatible and can be edited
manually for tweaking before using toplucker on them.

This supports FontBucket font databases.


Make sure you abide by all relevant copyright and licensing rules when ripping
fonts.  In the U.S., bitmapped fonts (which is all that this extracts) are
currently not copyrightable (but I am not a lawyer).  Elsewhere, check your
local laws.  Moreover, even in the U.S., rights you might have under copyright
law can be curtailed by licensing provisions.


The toplucker.tcl program allows you to save and load job configurations.  You
can also load a configuration from the commandline via:
    ./toplucker.tcl filename.pfj
You can even run a bunch of configurations (assuming all the necessary fields
are filled out) via:
    ./toplucker.tcl -run filename1.pfj ...



The pfj file format has changed between versions 1.04 and 1.10.  If you have
perl installed, the upgradepfj.pl script lets you migrate old jobs to the
new format via:
    perl upgradepfj.pl < oldJob.pfj > newJob.pfj



Security Note: A job configuration contains tcl/tk code.  Thus, using a job
      configuration produced by an unscrupulous third party could compromise your
      system security.  Only use job configurations produced by trusted
      sources.


1. BUILDING

The Makefile is for Cygwin and gcc, but the code should compile on other systems
with small changes to the Makefile only.  You need the FreeType 2 library for
topalmtext.  Get it from www.freetype.org


2. FONTCONV

fontconv [-l] inputname [-i lowresfont] H|Ha|2|2a|l|t|i outputname

This converts a Palm font from the input format into the output format.
You need to specify both the input and the output formats.
 -l: Take the low-res (default: hi-res) part of the FontTypeV2 font
 -i lowresfont: Use this low-res font to fill out the low-res portion of
     output.  Must be used in conjunction with output options 2 or 2a.
  H: hi-res portion of FontTypeV2 font
  L: lo-res portion of FontTypeV2 font
 Ha: hi-res portion of FontTypeV2 font in afnx format
 La: lo-res portion of FontTypeV2 font in afnx format
  l: version 1 type font
  2: put the font into the hi-res portion of FontTypeV2 font and
     fill out the low-res portion with scaled or -i specified font
 2a: same as 2, except for afnx format
1/1a: make a FontTypeV2 font with only a lo-res portion (afnx format
      for 1a)
  t: text-editable/readable format
  i: dump header and font information only
The inputname/outputname can be - for stdin/stdout for text format only.
To take a lo-res font and convert it into an OS5 hi-res font, you can
run:
   fontconv input.bin H output.bin
If you want the OS5 font to have a lo-res bitmap (a very ugly and
unreadable one) you can do:
   fontconv input.bin 2 output.bin

The text format is fairly straightfoward.  Just convert some Palm fonts
into it and see what it looks like.


3. TOPALMTEXT

This takes a font in any format supported by the FreeType 2 library (e.g.,
type 1, truetype, bdf, pcf, etc.) and produces a text file compatible with fontconv.
Thus, to convert the type 1 font CHRONOS.PFB to an OS5 hi-res font using a
pixel size of 22, do:
    topalmtext CHRONOS.PFB 22 | fontconv - H chronos.bin

The options for topalmtext are subject to change.  Currently, they are:
    topalmtext [-n] [-f] [-e] [-g] [-h] [-encoding] [-bN] [-ax:D] [-kx:D] [-vx:D]
        [-mfilename2] filename1 [pixelSize [gray]]

The pixelSize parameter is ignored for bitmap fonts.  For other fonts it's
a measurement of the pixel size of the font.  This is generally less than
the total height of the generated font.  I think it reflects the height of a
capital X or something like that.  The "gray" marker is for generating
gray-scale fonts, but the gray-scale converter is not finished.  It only
works with outline fonts.  The -f option forces character 0xA0 to be a
non-breaking space and 0x85 to be ..., overriding whatever the font's character
map says.  The -n option disables kerning, as needed for non-grayscale fonts
or older versions of Plucker.

The -e option makes topalmtext try (though not that hard) to make sure that
each output glyph has even width.  This tends to produce much nicer looking
OS5 fonts.

The -h option makes topalmtext try to guess where pre-combined Hebrew characters
might be stored in a Microsoft OpenType Times New Roman, Arial or Courier font
for the purposes of encoding pbplus_hebrew.  Do not use this with any other
font.

The -g option has the same effect as the "gray" parameter.

The -bN option spreads characters horizontally by N extra pixels, thus
creating a fake bold.

The -ax:D, -kx:D and -vx:D options tweak the advance width, kerning and
vertical position, respectively, for the character(s) specified by x.  The
tweak adds D (decimal) to the normal value.  Thus, -k65:-1 shifts the capital 
A glyph one pixel to the left.  Probably if you do that, you also want to decrease its 
advance width by the corresponding amount, since otherwise there will be an extra
pixel to the right.  The tweaks are useful for fine-tuning the font spacing.

The x parameter in the -a, -k and -v options can be:
  a     : decimal number
  a-b   : where a and b are decimal numbers, to specify a range
  /c/   : where c is a literal character
  /c-d/ : where c and d are literal characters, to specify a range

Kerning is always off for non-gray-scale fonts, and even width forcing is
always off for gray-scale fonts.

The -mfilename2 option makes the filename1 font fit within the vertical
metrics of the filename2 font.  This can be used to force, for instance,
bold and italic fonts to match the non-bold Roman fonts and is important if
generating fonts for a program like Plucker that ignores baseline positions.
Some glyphs may be cut off then.

The -encoding option is ignored unless the input font is a Unicode font, in
which case it sets the output encoding.  The use of -f with -encoding, unless
the encoding is palmLatin, leads to unpredictable results.  The list of
possible encodings is:
              none palmLatin CP1250 CP1251 CP1252 CP1253 CP1254 CP1255
              CP1256 CP1257 CP1258 CP874 ISO8859_1 ISO8859_10 ISO8859_11
              ISO8859_13 ISO8859_14 ISO8859_15 ISO8859_16 ISO8859_2 ISO8859_3
              ISO8859_4 ISO8859_5 ISO8859_6 ISO8859_7 ISO8859_8 ISO8859_9 KOI8R
              GreekKeys PBPlus_Hebrew PBPlus_Interlinear

Note that GreekKeys includes an extension of GreekKeys for PalmBible+ in order
to include all capital Latin glyphs.

The -n, -f, -e, and -g parameters can be negated via: -n-, -f-, -e- and -g-.
Parameters are read left-to-right, except that the "gray" parameter if available
counts by courtesy as the first parameter on the line.  Parameters further to the 
right override those on the left.  Note that because of the way the "gray"
parameter works, "-g- ... gray" has the "gray" parameter overridden by the -g-.

Extra options specified in toplucker.tcl go at the end of the parameters and hence
override everything else.

topalmtext can accept Palm font resources (including in text format) as input, in 
which case all parameters and options are ignored (the font size must be specified
but is ignored).



4. TOGRAY

This takes as input gray-scale fonts in TOPALMTEXT output format, and converts
them to .bin resources in the format described in GrayFormat.txt.  Usage:
     togray [-Wworkingdir] [ResourceType]HexResourceNum filename.txt ResourceTypeList [- ...]

-WworkingDir:
     Directory to put resources in.
ResourceType:
     Four letter identifier of the type of resource (no space between it and the
     the HexResourceNum), if different from GrFn (Plucker requires NFNT for
     Lo-Res and Sony and nfnt for OS5).
HexResourceNum:
     Resource number of the main (GrFn) resource for the font.
filename.txt:
     Input grayscale font.
ResourceTypeList:
     List of entries taken from GU14 (pre-OS5 upright), GL14 (pre-OS5 rotated-left),
     GR14 (pre-OS5 rotated-right), GU34 (OS5 upright), GL34 (OS5 rotated-left),
     GR34 (OS5 rotated-right)

This generates a whole bunch of G*.bin files which you can put into a .prc with
par.  You can simultaneously convert multiple files for putting into a single .prc
by separating commands with a "-" surrounded by spaces.  For scripting convenience,
a "-" surrounded by spaces at the very end of the argument list is completely
ignored.



5. TOPLUCKERGRAY

This shell script takes as input one or two outline fonts (TTF or Type 1) and sizes, and creates
an anti-aliased font package suitable for Plucker.  It also clobbers any G*.bin and nfnt*.bin and NFNT*.bin files
in your current directory as well as some topluckergray-*.txt files.  Usage:
   topluckergray lo|palmhi|sonyhi[.right|.left] dbname inputfilename pixelsize [inputfilename boldpixelsize]

lo|palmhi|sonyhi:
     type of font

.right: (no spaces between the type and the rotation)
     include clock-wise rotated versions

.left:
     include counterclock-wise rotates versions

.all:
     include all rotated versions

dbname:
     name of font db (will show up in font dialog in Plucker)

inputfilename:
     path of outline font

pixelsize:
     approximate size in pixels (not exact!)

If you include a second inputfilename and boldpixelsize, this will be used for bold fonts.



6. TOPLUCKER.TCL

If you have tcl/tk and par installed, and make sure that toplucker.tcl and the binaries for
par, togray and topalmText are all in the same directory, then toPlucker is a more user friendly
(and more powerful) interface to producing gray-scale font packages for Plucker.


7. TOPRC

This simple tool creates a .pdb or .prc database.  A .prc database is created from binary files
with filenames of the form:
    rsrcXXXX.ext
where rsrc is the resource type and XXXX is the resource id.  The .ext has no special
meaning, and neither do any characters in the filename after character 8.  This can
be used if on Windows you need to have both, say, files for nfnt0338 and NFNT0338.
Just name one of these nfnt0338x.bin and the other NFNT0338y.bin.

Usage:
  toprc outname.prc dataBaseName TypeID CreatorID rsrcXXXX.ext ...
or:
  toprc -d outname.pdb databaseName TypeID CreatorID filename1.ext ...

dataBaseName:
      Palm database name:  Maximum of 31 characters.
      TypeID:              Type of resource database (e.g., "Font" for Plucker font databases).
      CreatorID:           Creator ID.
      rsrcXXXX.ext ...     List of resource data files.
      filename1.ext ...    List of records.

You can also automatically create a grayfont-compatible font remap resource
for Plucker.  To do that, when creating a .prc file, instead of rsrcXXXX.ext, put:
    -f rsrcXXXX fontID
This creates a new resource of type rsrc and number XXXX (hex) and sets it to be
a remapping resource that remaps the font to fontID.  The fontID can be either a hex
number or one
of the following special identifiers:

  Sony fonts:
        hrTinyFont
        hrTinyBoldFont
        hrSmallFont
        hrSmallSymbolFont
        hrSmallSymbol11Font
        hrSmallSymbol7Font
        hrSmallLedFont
        hrSmallBoldFont
        hrStdFont
        hrBoldFont
        hrLargeFont
        hrSymbolFont
        hrSymbol11Font
        hrSymbol7Font
        hrLedFont
        hrLargeBoldFont

    PalmOS fonts:
        stdFont
        boldFont
        largeFont
        symbolFont
        symbol11Font
        symbol7Font
        ledFont
        largeBoldFont

    Plucker built-in fonts:
        stdFixedFont_sony
        stdFixedFont_palm
        narrowFixedFont
        narrowFont_sony

If you do this, you don't need TOREMAP.


8. USING WITH FONTHACK (for OS <5) and FONTHACK OS 5

To create a bunch of fonthack fonts for OS <5, first convert them to binary files with
fontconv (and, if applicable, topalmtext), and then use:
      toprc -d outname.pdb "Out Name" Font Font filename1.bin filename2.bin ...

For instance, to create a font with two different sizes of the Sample.pfb font,
you can do:
      topalmtext Sample.pfb 9 | fontconv - l Sample9.bin
      toplamtext Sample.pfb 13 | fontconv - l Sample13.bin
      toprc -d Sample.pdb "Sample Font" Font Font Sample9.bin Sample13.bin

To create a FonthackV font for OS 5, convert it with fontconv to afnx format (with
low-res component included, please) with filename afnx22b8.bin, and then use:
    toprc outname.prc "Out Name" Font fHKv afnx22b8.bin

For instance, to create a 22 point version of Sample.pfb, do:
       topalmtext Sample.pfb 22 | fontconv - 2a afnx22b8.bin
       toprc Sample22.prc "Sample 22" Font fHKv afnx22b8.bin

9. USING WITH FONTS4OS5 OR FONTSUBST

To create a package of Fonts4OS5 fonts for OS 5, first convert them to afnx format
binary files with fontconv (and, if applicable, topalmtext), including low-res
components if possible, and using the following filenames:
       afnx7d00.bin: standard font
       afnx7d01.bin: bold font
       afnx7d02.bin: large font
       afnx7d03.bin: large bold font
       
For a mysterious reason, FontSubst uses a format that differs in two bits from
standard afnx format (each resource has second byte 0x02 instead of 0x92) and
requires a tAIN0000.bin resource file containing the font name.  So, if you're 
creating FontSubst fonts, after creating the above files, do:
       fsconvert "Font name"
This deletes the afnx7d??.bin files and creates afnx0000.bin, afnx0001.bin,
afnx0002.bin and afnx0007.bin (yes, "0007") files.  It also makes the tAIN0000.bin
resource.

Now, to create the actual font package, for Fonts4OS5 do:
        toprc filename.prc "Font name" Font FOS5 afnx7d00.bin afnx7d01.bin afnx7d02.bin afnx7d03.bin
and for FontSubst do:
        toprc filename.prc "Font name" fmod FSub afnx0000.bin afnx0001.bin afnx0002.bin afnx0007.bin tAIN0000.bin

In fact, fsconvert lets you generally convert back and forth.  To convert 
Fonts4OS5 afnx7d0?.bin to FontSubst afnx000?.bin, do:
        fsconvert "Font name"
In the reverse direction, do:
        fsconvert -



10. grayfont.[ch]

An example set of routines for displaying gray-scale fonts on Palm units.  This
one is taken from Plucker, but can be easily modified.


11. Known issues

A. Under WinXP (and others perhaps) browsing to the system fonts
   directory with the tcl/tk browser doesn't let you select the
   fonts.

   Workaround:  Before you browse to the system fonts directory,
   set the window to display details.  Then you will see the filenames
   of the font files and be able to type them in directly.

B. TrueType fonts don't look that great.  This is because TrueType hinting
   is replaced by auto-hinting for patent reasons.

C. The last directory browsed to is not remembered in the tcl/tk GUI.


12. TOREMAP

A renderer for grayscale fonts is supposed to have a capability for remapping a font
to a built-in PalmOS font.  This requires a special fake nfnt or NFNT resource with
remapping information.  You can generate such a resource with:
        toremap nfntxxxx.bin [fontID]
or:
        toremap nfntxxxx.bin [fontID]
If fontID is omitted, it defaults to 0 (standard system font).  fontID may be a
hexadecimal number indicating the fontID to remap the font to, or it can be one
of the following special identifiers:

  Sony fonts:
        hrTinyFont
        hrTinyBoldFont
        hrSmallFont
        hrSmallSymbolFont
        hrSmallSymbol11Font
        hrSmallSymbol7Font
        hrSmallLedFont
        hrSmallBoldFont
        hrStdFont
        hrBoldFont
        hrLargeFont
        hrSymbolFont
        hrSymbol11Font
        hrSymbol7Font
        hrLedFont
        hrLargeBoldFont

    PalmOS fonts:
        stdFont
        boldFont
        largeFont
        symbolFont
        symbol11Font
        symbol7Font
        ledFont
        largeBoldFont

    Plucker built-in fonts:
        stdFixedFont_sony
        stdFixedFont_palm
        narrowFixedFont
        narrowFont_sony

13. EXTRACTFONT

Usage:
    extractfont filename.pdb | filename.prc

This extracts all fonts from that file (double density only in the case of FontTypeV2
fonts) in text format.  If the input file is dir/filename.pdb, output fonts will be
of the form:
   dir/filename-N-*.txt
   
This supports FontBucket font databases.

Make sure you abide by all relevant copyright and licensing rules when ripping
fonts.  In the U.S., bitmapped fonts (which is all that this extracts) are
currently not copyrightable (but I am not a lawyer).  Elsewhere, check your
local laws.  Moreover, even in the U.S., rights you might have under copyright
law can be curtailed by licensing provisions.

