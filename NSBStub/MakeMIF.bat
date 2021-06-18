rem MakeMIF

rem This takes the Tiny .svg file and makes the MIF file
rem Run this after svg2svgt.exe
rem Run this before MakeSISX

tools\mifconv resources\nsbIcon.mif /X /S. /c32 nsbIcon_1.svg
del nsbIcon_1.svg