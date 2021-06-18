#!/usr/bin/wish
# $Id: toplucker.tcl,v 1.40 2005/05/07 18:22:52 arpruss Exp $
#
source combobox.tcl
package require combobox 2.3
catch {namespace import combobox::*}

proc mainline {} {
       create_widgets
}

proc go { quiet } {
    global extraOptions
    global bitmapVersions
    global outFileName
    global palmBibleOnly
    global outDBName
    global fontSize
    global styleId
    global orientId
    global fontPrefixes
    global generateFontType
    global systemList
    global fontConvTypes
    global fontList
    global inFontFileName
    global fakeBold
    global orientList
    global generateFontOrientation
    global argv0
    global antiAliasing
    global forceExtraLatin1Characters
    global disableKerning
    global forceEven
    global encodings
    global keepSourceEncoding
    global offsets
    global encoding
    global matchFontMetrics
    global matchMetricsSource

    set progDir [file dirname $argv0]
    set workingDirExt "_tempDir"
    set toText "$progDir/topalmtext"
    set toGray "$progDir/togray"
    set fontConv "$progDir/fontconv"
    set toPrc "$progDir/toprc"
    set workingDir "$outFileName$workingDirExt"
    set bitDepth 4

    set activeFontList ""

    foreach font $fontList {
        if {$inFontFileName($font) != ""} {
            lappend activeFontList $font
        }
        if {$keepSourceEncoding($font)} {
            set finalEncodings($font) "none"
        } {
            if {$encodings($font) != "" } {
                set finalEncodings($font) $encodings($font)
            } {
                set finalEncodings($font) $encoding
            }
        }
    }
    if {$activeFontList == ""} {
        tk_messageBox -title "Error" -message "You need to specify at least one font."
        return
    }
    set activeSystemList ""
    foreach system $systemList {
        if {$generateFontType($system)} {
            lappend activeSystemList $system
        }
    }
    if {$activeSystemList == ""} {
        tk_messageBox -title "Error" -message "You need to specify at least one system."
        return
    }
    set activeOrientList ""
    foreach orient $orientList {
        if {$generateFontOrientation($orient)} {
            lappend activeOrientList $orient
        }
    }
    if {$activeOrientList == ""} {
        tk_messageBox -title "Error" -message "You need to specify at least one orientation."
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

    set commandLine0 "$toText"
    if {$forceExtraLatin1Characters} {
       lappend commandLine0 "-f"
    }
    if {$disableKerning} {
       lappend commandLine0 "-n"
    }
    if {$forceEven} {
       lappend commandLine0 "-e"
    }
    if {$antiAliasing} {
        set gray "gray"
    } {
        set gray ""
    }
    foreach font $activeFontList {
         set commandLine $commandLine0
         if { $matchFontMetrics($font) } {
             lappend commandLine "-m$inFontFileName($matchMetricsSource($font))"
         }
         lappend commandLine "-b$fakeBold($font)"
         lappend commandLine "-$finalEncodings($font)"

         set fnameFull [string tolower $inFontFileName($font)]
         set fname [file tail $fnameFull]
         if { $fname == "arial.ttf" || $fname == "ariali.ttf" || $fname == "arialb.ttf" || $fname == "arialbi.ttf" ||
                $fname == "cour.ttf"  || $fname == "couri.ttf"  || $fname == "courb.ttf"  || $fname == "courbi.ttf"  ||
                $fname == "times.ttf" || $fname == "timesi.ttf" || $fname == "timesb.ttf" || $fname == "timesbi.ttf" } {
             lappend commandLine "-h"
         }
         if { $extraOptions($font) != "" } {
             set commandLine [concat $commandLine $extraOptions($font)]
         }
         set commandLine [concat $commandLine $inFontFileName($font)]
         lappend commandLine "$fontSize($font)"
         if { $antiAliasing } {
             lappend commandLine "gray"
         }
         lappend commandLine > "$workingDir/$font.fonttext"
         eval exec $commandLine
         if {! $antiAliasing} {
             foreach system $activeSystemList {
                 exec "$fontConv" "$workingDir/$font.fonttext" $fontConvTypes($system) "$workingDir/$fontPrefixes($system)$styleId($font).bin"
             }
         }
    }
    if {$antiAliasing} {
        foreach font $activeFontList {
            set commandLine "-W$workingDir/"
            lappend commandLine "-#$offsets($font)"
            foreach system $activeSystemList {
                lappend commandLine $fontPrefixes($system)$styleId($font)
                lappend commandLine "$workingDir/$font.fonttext"
                foreach orient $activeOrientList {
                    lappend commandLine "G$orientId($orient)$bitmapVersions($system)$bitDepth"
                }
                lappend commandLine "-"
            }
            eval exec "{$toGray}" $commandLine
        }
    }

    if { $palmBibleOnly } {
        set typeName "skin"
        set creatorName "PBLP"
    } {
        set typeName    "Font"
        set creatorName "Plkr"
    }

    set commandLine [list "$outFileName" "$outDBName" $typeName $creatorName]
    set commandLine [concat $commandLine [glob -path "$workingDir/" *.bin]]

    eval exec "{$toPrc}" $commandLine
    set toDelete [glob -nocomplain -path "$workingDir/" *.bin *.fonttext]
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
    set types {
        {{Palm Resource Database}   {.prc}        }
    }
    set outFileName [tk_getSaveFile -defaultextension prc]
    if {$outDBName == ""} then {
        set outDBName [file rootname [file tail $outFileName]]
    }
}

proc insertFileName { font } {
    global inFontFileName
    global biggerVersion
    global smallerVersion
    global palmBibleOnly
    set inFontFileName($font) [tk_getOpenFile]
    if {$palmBibleOnly} {} {
        if {$biggerVersion($font) != "" && $inFontFileName($biggerVersion($font)) == ""} then {
            set inFontFileName($biggerVersion($font)) $inFontFileName($font)
        }
        if {$smallerVersion($font) != "" && $inFontFileName($smallerVersion($font)) == ""} then {
            set inFontFileName($smallerVersion($font)) $inFontFileName($font)
        }
    }
}

proc saveConfiguration { } {
    global outFileName
    global outDBName
    global palmBibleOnly
    global forceExtraLatin1Characters
    global disableKerning
    global forceEven
    global encoding
    global antiAliasing

    global systemList
    global generateFontType

    global fontSize
    global fontList
    global inFontFileName
    global fakeBold
    global extraOptions

    global orientList
    global generateFontOrientation

    global keepSourceEncoding
    global matchFontMetrics

    set  fileName [tk_getSaveFile -defaultextension pfj -filetypes { { {PalmFontConv job} {pfj} } { {All files} * } } ]
    if { $fileName == "" } {
        return
    }
    set  config [open $fileName "w"]
    puts $config "# PalmFontConv configuration file for PalmFontConv 1.10 and up."
    puts $config "# This is a tcl/tk script."
    puts $config "set outFileName      \"$outFileName\""
    puts $config "set outDBName        \"$outDBName\""
    puts $config "set palmBibleOnly    $palmBibleOnly"
    puts $config "set disableKerning   $disableKerning"
    puts $config "set forceEven        $forceEven"
    puts $config "set encoding         \"$encoding\""
    puts $config "set antiAliasing     $antiAliasing"
    puts $config "set forceExtraLatin1Characters         $forceExtraLatin1Characters"

    foreach font $fontList {
        puts $config "set inFontFileName($font)     \"$inFontFileName($font)\""
        puts $config "set fontSize($font)           $fontSize($font)"
        puts $config "set fakeBold($font)           $fakeBold($font)"
        puts $config "set keepSourceEncoding($font) $keepSourceEncoding($font)"
        puts $config "set matchFontMetrics($font)   $matchFontMetrics($font)"
        puts $config "set extraOptions($font)       [list $extraOptions($font)]"
    }

    foreach system $systemList {
        puts $config "set generateFontType($system)  $generateFontType($system)"
    }

    foreach orient $orientList {
        puts $config "set generateFontOrientation($orient)  $generateFontOrientation($orient)"
    }
    close $config
    tk_messageBox -title "Done!" -message "Saved configuration in $fileName."
}

proc add_font_record { font name id bigger smaller encoding offset match } {
    global inFontFileName
    global fakeBold
    global styleId
    global biggerVersion
    global smallerVersion
    global fontList
    global offsets
    global keepSourceEncoding
    global encodings
    global offsets
    global row
    global matchMetricsSource
    global matchFontMetrics
    global extraOptions

    lappend fontList $font
    set styleId($font) $id
    set biggerVersion($font) $bigger
    set smallerVersion($font) $smaller
    set encodings($font) $encoding
    set offsets($font) $offset
    set fakeBold($font) "0"
    set matchMetricsSource($font) $match

    label .frmFonts.name$font -text "$name: "
    entry .frmFonts.fileName$font -width 45 -textvariable inFontFileName($font)
    button .frmFonts.browse$font -text "Browse..." -command "insertFileName $font"
    entry .frmFonts.size$font -textvariable fontSize($font) -width 3
    checkbutton .frmFonts.keepEncoding$font -variable keepSourceEncoding($font)
    set keepSourceEncoding($font) 0
    entry .frmFonts.bold$font -textvariable fakeBold($font) -width 2
    if { $matchMetricsSource($font) != "" } {
        checkbutton .frmFonts.match$font -variable matchFontMetrics($font)
        grid config .frmFonts.match$font -column 6 -row $row
    } {
        set matchFontMetrics($font) 0
    }
    entry .frmFonts.extraOptions$font -width 15 -textvariable extraOptions($font)

    grid config .frmFonts.name$font -column 0 -row $row -sticky "e"
    grid config .frmFonts.fileName$font -column 1 -row $row
    grid config .frmFonts.browse$font -column 2 -row $row
    grid config .frmFonts.size$font   -column 3 -row $row
    grid config .frmFonts.keepEncoding$font -column 4 -row $row
    grid config .frmFonts.bold$font -column 5 -row $row
    grid config .frmFonts.extraOptions$font -column 7 -row $row

    set row [expr 1 + $row]
}

proc add_system { form system name fontPrefix bitmapVersion fontConvType } {
    global fontPrefixes
    global generateFontType
    global systemList
    global bitmapVersions
    global fontConvTypes
    lappend systemList $system
    set fontPrefixes($system) $fontPrefix
    set bitmapVersions($system) $bitmapVersion
    set fontConvTypes($system) $fontConvType
    checkbutton $form -text "$name" -variable generateFontType($system)
}

proc add_orient { form orient name id default } {
    global generateFontOrientation
    global orientList
    global orientId
    lappend orientList $orient
    set orientId($orient) $id
    set generateFontOrientation($orient) $default
    checkbutton $form -text "$name" -variable generateFontOrientation($orient)
}

proc doForcePB {} {
    global palmBibleOnly
    global generateFontType
    global generateFontOrientation
    if { $palmBibleOnly } {
        set generateFontType(loRes) 0
        set generateFontType(sony)  0
        set generateFontType(os5)   1
        set generateFontOrientation(normal) 1
        set generateFontOrientation(left)   0
        set generateFontOrientation(right)  0
    }
}



proc doDefaultSizes {} {
    global fontSize

    set smallFontSize 18
    set largeFontSize 21
    set narrowFontSize 17
    set hebrewFontSize 20

    set fontSize(stdFont)        $smallFontSize
    set fontSize(boldFont)       $smallFontSize
    set fontSize(largeFont)      $largeFontSize
    set fontSize(largeBoldFont)  $largeFontSize
    set fontSize(narrow)         $narrowFontSize
    set fontSize(fixed)          $smallFontSize
    set fontSize(IstdFont)       $smallFontSize
    set fontSize(IboldFont)      $smallFontSize
    set fontSize(IlargeFont)     $largeFontSize
    set fontSize(IlargeBoldFont) $largeFontSize
    set fontSize(Inarrow)        $narrowFontSize
    set fontSize(Ifixed)         $smallFontSize
    set fontSize(greekFont)      $smallFontSize
    set fontSize(hebrewFont)     $hebrewFontSize
    set fontSize(interlinearFont) $smallFontSize
    set fontSize(customFont)     $smallFontSize
}

proc resize { delta } {
    global fontSize
    global fontList

    foreach font $fontList {
        set fontSize($font) [expr $delta + $fontSize($font)]
    }
}

proc sourceConfiguration { fileName } {
    global outFileName
    global outDBName
    global palmBibleOnly
    global forceExtraLatin1Characters
    global disableKerning
    global forceEven
    global encoding
    global antiAliasing

    global systemList
    global generateFontType

    global fontSize
    global fontList
    global inFontFileName
    global fakeBold
    global extraOptions

    global orientList
    global generateFontOrientation

    global keepSourceEncoding
    global matchFontMetrics

    source $fileName
}

proc loadConfiguration { } {
     set fileName [tk_getOpenFile -defaultextension pfj -filetypes { { {PalmFontConv job} {pfj} } { {All files} * } } ]
     if { $fileName == "" } {
         return
     }
     sourceConfiguration $fileName
}

proc create_widgets {} {
    global outFileName
    global antiAliasing
    global outDBName
    global forceExtraLatin1Characters
    global palmBibleOnly
    global forceEven
    global disableKerning
    global generateFontType
    global encoding

    global smallFontSize
    global largeFontSize
    global narrowFontSize
    global hebrewFontSize
    global row
    global argv
    global argc

    set row 0

    frame .frmFonts
    label .frmFonts.name -text "Plucker/PB+ font"
    label .frmFonts.fileName -text "Desktop filename"
    label .frmFonts.size -text "Size"
    label .frmFonts.keepEncoding -text "Keep original encoding" -wrap 60
    label .frmFonts.bold -text "Fake bold width" -wrap 50
    label .frmFonts.match -text "Force standard metrics" -wrap 60
    label .frmFonts.extraOptions  -text "Extra options" -wrap 50
    pack .frmFonts
    grid config .frmFonts.name -column 0 -row 0 -sticky "e"
    grid config .frmFonts.fileName -column 1 -row 0
    grid config .frmFonts.size   -column 3 -row 0
    grid config .frmFonts.keepEncoding -column 4 -row 0
    grid config .frmFonts.bold -column 5 -row 0
    grid config .frmFonts.match -column 6 -row 0
    grid config .frmFonts.extraOptions -column 7 -row 0

    set row [expr 1 + $row]

    add_font_record stdFont         "Standard"           "20" "largeFont" "narrow" "" 1000 ""
    add_font_record boldFont        "Bold"               "21" "largeBoldFont" "" "" 2000  "stdFont"
    add_font_record largeFont       "Large"              "22" "" "" "" 3000 ""
    add_font_record largeBoldFont   "Large bold"         "23" "" "" "" 4000 ""
    add_font_record narrow          "Narrow"              "24" "" "" "" 5000 ""
    add_font_record fixed           "Fixed width"        "26" "" "" "" 6000 "stdFont"
    add_font_record IstdFont        "Italic"             "60" "IlargeFont" "Inarrow" "" 7000 "stdFont"
    add_font_record IboldFont       "Italic bold"        "61" "IlargeBoldFont" "" "" 8000 "stdFont"
    add_font_record IlargeFont      "Italic large"       "62" "" "" "" 9000 ""
    add_font_record IlargeBoldFont  "Italic large bold"  "63" "" "" "" A000 ""
    add_font_record Inarrow         "Italic narrow"       "64" "" "" "" B000 ""
    add_font_record Ifixed          "Italic fixed width" "66" "" "" "" C000 "stdFont"
    add_font_record greekFont       "PB+ Greek"          "F0" "" "" "GreekKeys" D000 ""
    add_font_record hebrewFont      "PB+ Hebrew"           "F1" "" "" "PBPlus_Hebrew" E000 ""
    add_font_record interlinearFont "PB+ Interlinear"    "F2" "" "" "PBPlus_Interlinear" 1800 ""
    add_font_record customFont      "PB+ Custom"         "F3" "" "" "" F000 ""

    doDefaultSizes

    frame .frmResize
    label .frmResize.label -text "Font sizes: "
    button .frmResize.down -text "-" -command "resize -1"
    button .frmResize.up -text "+" -command "resize 1"
    button .frmResize.defaults -text "Default sizes" -command "doDefaultSizes"

    label .frmResize.encLabel -text "Normal encoding: "
    combobox .frmResize.encBox -textvariable encoding
    .frmResize.encBox list insert end \
                  "none" "palmLatin" "CP1250" "CP1251" "CP1252" "CP1253" "CP1254" "CP1255" \
                  "CP1256" "CP1257" "CP1258" "CP874" "ISO8859_1" "ISO8859_10" "ISO8859_11" \
                  "ISO8859_13" "ISO8859_14" "ISO8859_15" "ISO8859_16" "ISO8859_2" "ISO8859_3" \
                  "ISO8859_4" "ISO8859_5" "ISO8859_6" "ISO8859_7" "ISO8859_8" "ISO8859_9" "KOI8R" \
                  "PBPlus_Hebrew" "GreekKeys" "PBPlus_Interlinear"
    pack .frmResize
    grid .frmResize.label -row 0 -column 0
    grid .frmResize.down -row 0 -column 1
    grid .frmResize.up -row 0 -column 2
    grid .frmResize.defaults -row 0 -column 3
    grid .frmResize.encLabel -row 0 -column 4
    grid .frmResize.encBox -row 0 -column 5
    .frmResize.encBox select 1

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
    checkbutton .frmGenerate.antiAlias -text "Anti-aliasing" -variable antiAliasing
    checkbutton .frmGenerate.disableKerning -text "Disable kerning" -variable disableKerning
    checkbutton .frmGenerate.forceExtraLatin1Characters -text "Force some Latin 1 chars" -variable forceExtraLatin1Characters
    checkbutton .frmGenerate.forceEven -text "Force even width" -variable forceEven
    checkbutton .frmGenerate.pbOnly -text "PalmBible+ only" -variable palmBibleOnly -command "doForcePB"
    pack .frmGenerate
    grid .frmGenerate.antiAlias .frmGenerate.forceExtraLatin1Characters .frmGenerate.disableKerning .frmGenerate.forceEven .frmGenerate.pbOnly
    set disableKerning 0
    set forceExtraLatin1Characters       0
    set forceEven      0
    set palmBibleOnly        0
    set antiAliasing        1

    frame .frmSystems
    add_system .frmSystems.loRes loRes "Standard resolution" "NFNT02" 1 "l"
    add_system .frmSystems.sony sony "Sony high resolution (pre-OS5)" "NFNT04" 1 "l"
    add_system .frmSystems.os5 os5 "OS5 high resolution" "nfnt06" 3 "H"
    set generateFontType(os5) 1
    pack .frmSystems
    grid .frmSystems.loRes .frmSystems.sony .frmSystems.os5

    frame .frmOrient
    label .frmOrient.label -text "Orientation (only if anti-aliased): "
    add_orient .frmOrient.normal normal "Normal" "U" 1
    add_orient .frmOrient.right right "Clockwise" "R" 0
    add_orient .frmOrient.left left "Counterclockwise" "L" 0
    pack .frmOrient
    grid .frmOrient.label .frmOrient.normal .frmOrient.right .frmOrient.left

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

