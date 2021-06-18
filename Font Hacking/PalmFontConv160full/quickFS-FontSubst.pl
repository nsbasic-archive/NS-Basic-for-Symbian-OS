#/usr/bin/perl
# fontstyle inNormalFName inBoldFName size1 size
# 0         1             2           3     4
unless ( @ARGV >= 5 ) { die "quickFS fontstyle stdFileName boldFileName size1 size2 ..."; }
$i = 3;
while ( $i < @ARGV ) {
    $size1 = $ARGV[ $i ];
    $size2 = $ARGV[ $i + 1 ];
    open O, ">temp.pfj";
    print O "set outFileName      \"FontSubst-$ARGV[0]-$size1-ker.prc\"\n";
    print O "set outDBName        \"$ARGV[0] $size1 aa ker\"\n";
    print O "set kerning          1\n";
    print O "set exactSpacing     1\n";
    print O "set forceEven        0\n";
    print O "set encoding         \"palmLatin\"\n";
    print O "set output           \"FontSubst\"\n";
    print O "set forceExtraLatin1Characters         0\n";
    print O "set inFontFileName(stdFont)     \"$ARGV[1]\"\n";
    print O "set fontSize(stdFont)           $size1\n";
    print O "set extraOptions(stdFont)       {}\n";
    print O "set inFontFileName(boldFont)     \"$ARGV[2]\"\n";
    print O "set fontSize(boldFont)           $size1\n";
    print O "set extraOptions(boldFont)       {}\n";
    print O "set inFontFileName(largeFont)     \"$ARGV[1]\"\n";
    print O "set fontSize(largeFont)           $size2\n";
    print O "set extraOptions(largeFont)       {}\n";
    print O "set inFontFileName(largeBoldFont)     \"$ARGV[2]\"\n";
    print O "set fontSize(largeBoldFont)           $size2\n";
    print O "set extraOptions(largeBoldFont)       {}\n";
    close O;
    print "$ARGV[0] $size1 $size2\n";
    system "./toFontSmoother.tcl -run temp.pfj\n";
    $i = $i + 2;
    open O, ">temp.pfj";
    print O "set outFileName      \"FontSubst-$ARGV[0]-$size1-ker.prc\"\n";
    print O "set outDBName        \"$ARGV[0] $size1 FS aa ker\"\n";
    print O "set kerning          1\n";
    print O "set exactSpacing     1\n";
    print O "set forceEven        0\n";
    print O "set encoding         \"palmLatin\"\n";
    print O "set output           \"FontSubst\"\n";
    print O "set forceExtraLatin1Characters         0\n";
    print O "set inFontFileName(stdFont)     \"$ARGV[1]\"\n";
    print O "set fontSize(stdFont)           $size1\n";
    print O "set extraOptions(stdFont)       {}\n";
    print O "set inFontFileName(boldFont)     \"$ARGV[2]\"\n";
    print O "set fontSize(boldFont)           $size1\n";
    print O "set extraOptions(boldFont)       {}\n";
    print O "set inFontFileName(largeFont)     \"$ARGV[1]\"\n";
    print O "set fontSize(largeFont)           $size2\n";
    print O "set extraOptions(largeFont)       {}\n";
    print O "set inFontFileName(largeBoldFont)     \"$ARGV[2]\"\n";
    print O "set fontSize(largeBoldFont)           $size2\n";
    print O "set extraOptions(largeBoldFont)       {}\n";
    close O;
    print "$ARGV[0] $size1 $size2\n";
    system "./toFontSmoother.tcl -run temp.pfj\n";
    $i = $i + 2;
};
