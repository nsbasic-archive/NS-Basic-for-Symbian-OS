#!/usr/bin/wish
# $Id: toFontsmoother.tcl,v 1.10 2005/05/02 20:52:51 arpruss Exp $
#
source combobox.tcl
package require combobox 2.3
catch {namespace import combobox::*}

proc mainline {} {
       create_widgets
}

proc setResFonts4OS5 { } {
    global styleId
    global fakeStyleId
    global theAARes
    global theAAStart

    set styleId(stdFont) "afnx7d00"
    set fakeStyleId(stdFont) "GrFf7d00"
    set theAARes(stdFont) "GrFn7d00"
    set theAAStart(stdFont) "7d00"

    set styleId(boldFont) "afnx7d01"
    set fakeStyleId(boldFont) "GrFf7d01"
    set theAARes(boldFont) "GrFn7d01"
    set theAAStart(boldFont) "7d10"

    set styleId(largeFont) "afnx7d02"
    set fakeStyleId(largeFont) "GrFf7d02"
    set theAARes(largeFont) "GrFn7d02"
    set theAAStart(largeFont) "7d20"

    set styleId(largeBoldFont) "afnx7d03"
    set fakeStyleId(largeBoldFont) "GrFf7d03"
    set theAARes(largeBoldFont) "GrFn7d03"
    set theAAStart(largeBoldFont) "7d30"
}

proc setResFontSubst { } {
    global styleId
    global fakeStyleId
    global theAARes
    global theAAStart

    set styleId(stdFont) "afnx0000"
    set fakeStyleId(stdFont) "GrFf7d00"
    set theAARes(stdFont) "GrFn7d00"
    set theAAStart(stdFont) "7d00"

    set styleId(boldFont) "afnx0001"
    set fakeStyleId(boldFont) "GrFf7d01"
    set theAARes(boldFont) "GrFn7d01"
    set theAAStart(boldFont) "7d10"

    set styleId(largeFont) "afnx0002"
    set fakeStyleId(largeFont) "GrFf7d02"
    set theAARes(largeFont) "GrFn7d02"
    set theAAStart(largeFont) "7d20"

    set styleId(largeBoldFont) "afnx0007"
    set fakeStyleId(largeBoldFont) "GrFf7d07"
    set theAARes(largeBoldFont) "GrFn7d07"
    set theAAStart(largeBoldFont) "7d70"
}

proc setResFontHackV { } {
    global styleId
    global fakeStyleId
    global theAARes
    global fontList
    global theAAStart

    foreach font $fontList {
        set styleId($font) "afnx22b8"
        set fakeStyleId($font) "GrFf22b8"
        set theAARes($font) "GrFn22b8"
        set theAAStart($font) "7d00"
    }
}

proc go { quiet } {
    global bitmapVersions
    global theAAStart
    global theAARes
    global styleId
    global fakeStyleId
    global fontConvTypes
    global fontList
    global argv0
    global hires

    global inFontFileName
    global fontSize
    global outFileName
    global outDBName
    global kerning
    global forceExtraLatin1Characters
    global forceEven
    global exactSpacing
    global encoding
    global extraOptions
    global output

    set progDir [file dirname $argv0]
    set workingDirExt "_tempDir"
    set toText "$progDir/topalmtext"
    set fontConv "$progDir/fontconv"
    set toGray "$progDir/togray"
    set toPrc "$progDir/toprc"
    set workingDir "$outFileName$workingDirExt"
    set fsConvert "$progDir/fsconvert"

    set activeFontList ""

    foreach font $fontList {
        if {$inFontFileName($font) != ""} {
            if {$activeFontList != "" && $output == "FontHackV"} {
                tk_messageBox -title "Warning" -message "Only the first listed font will be included in FontHackV mode."
                break
            }
            lappend activeFontList $font
        }
    }
    if {$activeFontList == ""} {
        tk_messageBox -title "Error" -message "You need to specify at least one font."
        return
    }
    if {$outFileName == ""} {
        tk_messageBox -title "Error" -message "You need to specify the output filename."
        return
    }
    if {$outDBName == ""} {
        tk_messageBox -title "Error" -message "You need to specify the Palm database name."
        return
    }

    file mkdir $workingDir
    set toDelete [glob -nocomplain -path "$workingDir/" *.bin]
    if {$toDelete != ""} {
        eval file delete $toDelete
    }

    if {$forceExtraLatin1Characters} {
       set opt1 "-f"
    } {
       set opt1 "-f-"
    }
    if {$forceEven} {
       set opt2 "-e"
    } {
       set opt2 "-e-"
    }
    set opt3 "-B"
    set opt4 "-E"
    if {$kerning} {
       set opt5 "-n-"
    } {
       set opt5 "-n"
    }
    if { $output == "FontHackV" } {
        setResFontHackV
    }
    if { $output == "Fonts4OS5" || $output == "Standard" || $output == "Mobipocket" } {
        setResFonts4OS5
    }
    if { $output == "FontSubst" } {
        setResFontSubst
    }
    foreach font $activeFontList {
             set commandLine [list "$toText" -$encoding $opt1 $opt2 $opt3 $opt4 $opt5]
             if { $output == "Standard" || ! $hires } {
                 lappend commandLine "-E-" "-e-"
             }
             if { $extraOptions($font) != "" } {
                 set commandLine [concat $commandLine $extraOptions($font)]
             }
             set commandLine0 $commandLine
             lappend commandLine "$inFontFileName($font)" $fontSize($font) > "$workingDir/$font.fonttext"
             eval exec $commandLine
             if { $output == "Fonts4OS5" || $output == "FontHackV" || $output == "FontSubst" } {
                 set grayHeader "grayHeader $theAARes($font) $outDBName"
             } else {
                 set grayHeader "grayHeader $theAARes($font) GrFn-$outDBName"
             }
             set f [open "$workingDir/$font.fonttext" "a"]
             puts $f $grayHeader
             close $f
             if { $output == "Fonts4OS5" || $output == "FontHackV" || $output == "FontSubst" } {
                 set ARM "a"
             } {
                 set ARM ""
             }
             if { $output != "Standard" && $hires } {
                 set commandLine $commandLine0
                 lappend commandLine -H "$inFontFileName($font)" $fontSize($font) > "$workingDir/$font.half"
                 eval exec $commandLine
                 set f [open "$workingDir/$font.half" "a"]
                 puts $f $grayHeader
                 close $f
                 exec "$fontConv" "$workingDir/$font.half" -h L "$workingDir/$styleId($font).b"
                 exec "$fontConv" "$workingDir/$font.fonttext" -i "$workingDir/$styleId($font).b" 2$ARM "$workingDir/$styleId($font).binf"
             } {
                 if { $output == "Standard" } {
                     exec "$fontConv" "$workingDir/$font.fonttext" l "$workingDir/$styleId($font).binf"
                 } {
                     exec "$fontConv" "$workingDir/$font.fonttext" l$ARM "$workingDir/$styleId($font).binf"
                 }
             }

             if {$exactSpacing} {
                 set grayOpt4 "-E-"
             } {
                 set grayOpt4 $opt4
             }
             set commandLine [list "$toText" -$encoding $opt1 $opt2 $grayOpt4 $opt5]
             if { $output == "Standard" || ! $hires } {
                 lappend commandLine "-E-" "-e-"
             }
             if { $extraOptions($font) != "" } {
                 set commandLine [concat $commandLine $extraOptions($font)]
             }
             set commandLine0 $commandLine
             lappend commandLine -g "$inFontFileName($font)" $fontSize($font) > "$workingDir/$font.gray"
             eval exec $commandLine
             exec "$toGray" -a -3 "-W$workingDir/" "-#$theAAStart($font)" "$theAARes($font)" "$workingDir/$font.gray" GU34
             if { $exactSpacing && $output != "Standard" && $hires } {
                 set commandLine $commandLine0
                 lappend commandLine -B "$inFontFileName($font)" $fontSize($font) > "$workingDir/$font.fake"
                 eval exec $commandLine
                 set f [open "$workingDir/$font.fake" "a"]
                 puts $f $grayHeader
                 close $f
                 exec "$fontConv" "$workingDir/$font.fake" -f L$ARM "$workingDir/$fakeStyleId($font).bin"
             }
    }

    if {$output == "Fonts4OS5" || $output == "FontHackV" || $output == "FontSubst" } {

        if {$output == "FontHackV" } {
            set f [open "$workingDir/tSTR2328.bin" "w"]
            puts $f "$outDBName generated by toFontsmoother.tcl"
            close $f
            set commandLine [list "$outFileName" "$outDBName" Font fHKv]
        } 
        if { $output == "Fonts4OS5" } {
            set commandLine [list "$outFileName" "$outDBName" Font FOS5]
        }
        if {$output == "FontSubst" } {
            set f [open "$workingDir/tAIN0000.bin" "w"]
            puts $f "$outDBName\0"
            close $f
            set commandLine [list "$outFileName" "$outDBName" fmod FSub]
        }

        set commandLine [concat $commandLine [glob -path "$workingDir/" *.bin *.binf]]

        eval exec "{$toPrc}" $commandLine
    } {
        set commandLine [list "$outFileName-resources.prc" "GrFn-$outDBName" Font GrFn]
        set commandLine [concat $commandLine [glob -path "$workingDir/" *.bin]]

        eval exec "{$toPrc}" $commandLine

        set commandLine [list -d "$outFileName" "$outDBName" Font Font]
        set commandLine [concat $commandLine [glob -path "$workingDir/" *.binf]]

        eval exec "{$toPrc}" $commandLine
    }
    set toDelete [glob -nocomplain -path "$workingDir/" *]
    if {$toDelete != ""} {
        eval file delete $toDelete
    }
    eval file delete "$workingDir"

    if {! $quiet} {
       tk_messageBox -title "Done!" -message "Produced $outFileName."
    }
}

proc getOutFileName { } {
    global outFileName
    global outDBName
    global output
    if { "$output" != "Fonts4OS5" } {
        set ext "pdb"
        set types {
            {{Palm Database}   {".pdb"}        }
        }
    } {
        set ext "prc"
        set types {
            {{Palm Resource Database}   {".prc"}  }
        }
    }
    set outFileName [tk_getSaveFile -defaultextension $ext -filetypes $types]
    if {$outDBName == ""} then {
        set outDBName [file rootname [file tail $outFileName]]
    }
}

proc insertFileName { font } {
    global inFontFileName
    global biggerVersion
    global smallerVersion
    global output

    set inFontFileName($font) [tk_getOpenFile]
    if {$output != "FontHackV" } {
        if {$biggerVersion($font) != "" && $inFontFileName($biggerVersion($font)) == ""} then {
            set inFontFileName($biggerVersion($font)) $inFontFileName($font)
        }
        if {$smallerVersion($font) != "" && $inFontFileName($smallerVersion($font)) == ""} then {
            set inFontFileName($smallerVersion($font)) $inFontFileName($font)
        }
    }
}


proc saveConfiguration { } {
    global inFontFileName
    global fontSize
    global outFileName
    global outDBName
    global kerning
    global forceExtraLatin1Characters
    global forceEven
    global exactSpacing
    global encoding
    global extraOptions
    global output
    global hires

    global fontList

    set  fileName [tk_getSaveFile -defaultextension pfj -filetypes { { {PalmFontConv job} {pfj} } { {All files} * } } ]
    if { $fileName == "" } {
        return
    }
    set  config [open $fileName "w"]
    puts $config "# PalmFontConv configuration file for toFontSmoother."
    puts $config "# This is a tcl/tk script."
    puts $config "set outFileName      \"$outFileName\""
    puts $config "set outDBName        \"$outDBName\""
    puts $config "set kerning          $kerning"
    puts $config "set exactSpacing     $exactSpacing"
    puts $config "set forceEven        $forceEven"
    puts $config "set encoding         \"$encoding\""
    puts $config "set output           \"$output\""
    puts $config "set hires            $hires"
    puts $config "set forceExtraLatin1Characters         $forceExtraLatin1Characters"

    foreach font $fontList {
        puts $config "set inFontFileName($font)     \"$inFontFileName($font)\""
        puts $config "set fontSize($font)           $fontSize($font)"
        puts $config "set extraOptions($font)       [list $extraOptions($font)]"
    }

    close $config
    tk_messageBox -title "Done!" -message "Saved configuration in $fileName."
}


proc sourceConfiguration { fileName } {
    global inFontFileName
    global fontSize
    global outFileName
    global outDBName
    global kerning
    global forceExtraLatin1Characters
    global forceEven
    global exactSpacing
    global encoding
    global extraOptions
    global output                             
    global hires

    source $fileName
}

proc loadConfiguration { } {
     set fileName [tk_getOpenFile -defaultextension pfj -filetypes { { {PalmFontConv job} {pfj} } { {All files} * } } ]
     if { $fileName == "" } {
         return
     }
     sourceConfiguration $fileName
}

proc add_font_record { font name size id fakeId bigger smaller aaStart aaRes } {
    global inFontFileName
    global fontSize
    global styleId
    global fakeStyleId
    global biggerVersion
    global smallerVersion
    global fontList
    global kerning
    global theAAStart
    global theAARes
    global row
    global extraOptions

    lappend fontList $font
    set styleId($font) $id
    set fakeStyleId($font) $fakeId
    set biggerVersion($font) $bigger
    set smallerVersion($font) $smaller
    set fontSize($font) $size
    set theAAStart($font) $aaStart
    set theAARes($font) $aaRes
    label .frmFonts._($font)name -text "$name"
    entry .frmFonts._($font)fileName -width 45 -textvariable inFontFileName($font)
    button .frmFonts._($font)browse -text "Browse..." -command "insertFileName $font"
    entry .frmFonts._($font)size -width 5 -textvariable fontSize($font)
    entry .frmFonts._($font)extraOptions -width 30 -textvariable extraOptions($font)

    grid config .frmFonts._($font)name -column 0 -row $row -sticky "e"
    grid config .frmFonts._($font)fileName -column 1 -row $row
    grid config .frmFonts._($font)browse -column 2 -row $row
    grid config .frmFonts._($font)size -column 3 -row $row
    grid config .frmFonts._($font)extraOptions -column 4 -row $row

    set row [expr 1 + $row]
}

proc change_output { a } {
    global output
    if { $output == "FontHackV" } {
        tk_messageBox -title "FontHackV" -message "Leave all entries except the first blank."
    }
}

proc create_widgets {} {
    global outFileName
    global outDBName
    global forceExtraLatin1Characters
    global exactSpacing
    global forceEven
    global output
    global encoding
    global row
    global argv
    global argc
    global hires

    frame .frmFonts
    label .frmFonts.name -text "System font"
    label .frmFonts.fileName -text "Desktop filename"
    label .frmFonts.size -text "Size"
    label .frmFonts.extraOptions  -text "Extra options"
    pack .frmFonts
    grid config .frmFonts.name -column 0 -row 0
    grid config .frmFonts.fileName -column 1 -row 0
    grid config .frmFonts.size   -column 3 -row 0
    grid config .frmFonts.extraOptions -column 4 -row 0

    set row 1

    add_font_record stdFont        "Standard"    18 "afnx7d00" "GrFf7d00" "largeFont" "" "7d00" "GrFn7d00"
    add_font_record boldFont       "Bold"        18 "afnx7d01" "GrFf7d01" "largeBoldFont" "" "7d10" "GrFn7d01"
    add_font_record largeFont      "Large"       21 "afnx7d02" "GrFf7d02" "" "" "7d20" "GrFn7d02"
    add_font_record largeBoldFont  "Large bold"  21 "afnx7d03" "GrFf7d03" "" "" "7d30" "GrFn7d03"

    frame .frmOutput
    label .frmOutput.fileNameLabel -text "Output filename: "
    entry .frmOutput.outFileName -width 45 -textvariable outFileName
    label .frmOutput.dbNameLabel -text "Palm name: "
    button .frmOutput.browse -text "Browse..." -command "getOutFileName"
    entry .frmOutput.dbName -width 30 -textvariable outDBName
    pack .frmOutput
    grid .frmOutput.fileNameLabel .frmOutput.outFileName .frmOutput.browse
    grid .frmOutput.dbNameLabel .frmOutput.dbName

    frame .frmGenerate
    label .frmGenerate.label -text "Font generation: "
    checkbutton .frmGenerate.forceExtraLatin1Characters  -text "Force Latin 1" -variable forceExtraLatin1Characters
    checkbutton .frmGenerate.kerning   -text "Kerning" -variable kerning
    checkbutton .frmGenerate.forceEven -text "Scale to even width" -variable forceEven
    checkbutton .frmGenerate.exactSpacing     -text "exact spacing" -variable exactSpacing
    checkbutton .frmGenerate.hires     -text "hi-res" -variable hires
    combobox .frmGenerate.box -textvariable output
    .frmGenerate.box list insert end \
                  "Fonts4OS5" "FontSubst" "FontHackV" "Mobipocket" "Standard"
    set forceExtraLatin1Characters 0
    set forceEven 0
    set exactSpacing     1
    set kerning 1
    set hires   1
    .frmGenerate.box select 0
    pack .frmGenerate
    grid .frmGenerate.label .frmGenerate.forceExtraLatin1Characters .frmGenerate.forceEven .frmGenerate.kerning .frmGenerate.exactSpacing .frmGenerate.hires .frmGenerate.box

    frame .frmEncoding
    label .frmEncoding.label -text "Encoding: "

    combobox .frmEncoding.box -textvariable encoding
    .frmEncoding.box list insert end \
                  "none" "palmLatin" "CP1250" "CP1251" "CP1252" "CP1253" "CP1254" "CP1255" \
                  "CP1256" "CP1257" "CP1258" "CP874" "ISO8859_1" "ISO8859_10" "ISO8859_11" \
                  "ISO8859_13" "ISO8859_14" "ISO8859_15" "ISO8859_16" "ISO8859_2" "ISO8859_3" \
                  "ISO8859_4" "ISO8859_5" "ISO8859_6" "ISO8859_7" "ISO8859_8" "ISO8859_9" "KOI8R" \
                  "PBPlus_Hebrew" "GreekKeys" "PBPlus_Interlinear"
    pack .frmEncoding
    grid .frmEncoding.label -row 0 -column 0
    grid .frmEncoding.box -row 0 -column 1
    .frmEncoding.box select 1

    frame .frmCommands
    button .frmCommands.go -text "Go!" -font bold -command "go 0"
    button .frmCommands.save -text "Save job" -command saveConfiguration
    button .frmCommands.load -text "Load job" -command loadConfiguration
    button .frmCommands.cancel -text "Exit" -command exit
    pack .frmCommands
    grid .frmCommands.go .frmCommands.load .frmCommands.save .frmCommands.cancel

    if { [lindex $argv 1] != "" && [lindex $argv 0] == "-run" } {
        for {set i 1} {$i<$argc} {incr i} {
            sourceConfiguration [lindex $argv $i]
            go 1
        }
        exit
    }
    if { [lindex $argv 0] != "" } {
        sourceConfiguration [lindex $argv 0]
    }
}

mainline
