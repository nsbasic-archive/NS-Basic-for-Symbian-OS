#!/set usr/set bin/set perl
while ( <> ) {
    s/^set theOutFileName/set outFileName/;
    s/^set theDBName/set outDBName/;
    s/^set forcePB/set palmBibleOnly/;
    s/^set forceNBs/set forceExtraLatin1Characters/;
    s/^set disableKerning/set disableKerning/;
    s/^set forceEven/set forceEven/;
    s/^set encoding/set encoding/;
    s/^set aaFonts/set antiAliasing/;
    s/^set theFileNames/set inFontFileName/;
    s/^set theSize/set fontSize/;
    s/^set theBold/set fakeBold/;
    s/^set keepEncodings/set keepSourceEncoding/;
    s/^set doMatches/set matchFontMetrics/;
    s/^set opts/set extraOptions/;
    s/^set systemSelection/set generateFontType/;
    s/^set systemSelection/set generateFontOrientation/;
    print;
}

