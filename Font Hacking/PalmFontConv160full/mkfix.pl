open F1, $ARGV[0];
open F2, $ARGV[1];

while(<F1>) {
    s/^set ([^( ]*).*/$1/;
    s/\(/\\\(/g;
    s/[\n\r]//g;
    $from = $_;
    $_ = <F2>;
    s/^set ([^( ]*).*/$1/;
    s/\(/\\\(/g;
    s/[\n\r]//g;
    $to = $_;
    print "s/$from/$to/;\n";
}

