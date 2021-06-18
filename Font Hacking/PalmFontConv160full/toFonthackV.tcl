#!/usr/bin/wish
# $Id: toFonthackV.tcl,v 1.5 2004/02/26 22:15:25 arpruss Exp $
#
source combobox.tcl
package require combobox 2.3
catch {namespace import combobox::*}

proc mainline {} {
       create_widgets
}

proc go { } {
    global bitmapVersions
    global theOutFileName
    global theDBName
    global theSize
    global styleId
    global fontConvTypes
    global fontList
    global theFileNames
    global argv0
    global forceNBS
    global forceEven
    global encoding

    set progDir [file dirname $argv0]
    set workingDirExt "_tempDir"
    set toText "$progDir/topalmtext"
    set fontConv "$progDir/fontconv"
    set toPrc "$progDir/toprc"
    set workingDir "$theOutFileName$workingDirExt"

    set activeFontList ""

    foreach font $fontList {
        if {$theFileNames($font) != ""} {
            lappend activeFontList $font
        }
    }
    if {$activeFontList == ""} {
        tk_messageBox -title "Error" -message "You need to specify at least one font."
        return
    }
    if {$theOutFileName == ""} {
        tk_messageBox -title "Error" -message "You need to specify the output filename."
        return
    }
    if {$theDBName == ""} {
        tk_messageBox -title "Error" -message "You need to specify the Palm database name."
        return
    }

    file mkdir $workingDir
    set toDelete [glob -nocomplain -path "$workingDir/" *.bin]
    if {$toDelete != ""} {
        eval file delete $toDelete
    }

    if {$forceNBS} {
       set opt1 "-f"
    } {
       set opt1 "-f-"
    }
    if {$forceEven} {
       set opt2 "-e"
    } {
       set opt2 "-e-"
    }
    foreach font $activeFontList {
             exec $toText -$encoding $opt1 $opt2 -n $theFileNames($font) $theSize($font) > "$workingDir/$font.fonttext"
             exec $fontConv "$workingDir/$font.fonttext" 2a "$workingDir/$styleId($font).bin"
    }

    set commandLine [list "$theOutFileName" "$theDBName" Font fHKv]
    set commandLine [concat $commandLine [glob -path "$workingDir/" *.bin]]

    eval exec $toPrc $commandLine
    set toDelete [glob -nocomplain -path "$workingDir/" *.bin *.fonttext]
    if {$toDelete != ""} {
        eval file delete $toDelete
    }
    eval file delete "$workingDir"
    tk_messageBox -title "Done!" -message "Produced $theOutFileName."
}

proc getOutFileName { } {
    global theOutFileName
    global theDBName
    set types {
        {{Palm Resource Database}   {.prc}        }
    }
    set theOutFileName [tk_getSaveFile -defaultextension prc -filetypes $types]
    if {$theDBName == ""} then {
        set theDBName [file rootname [file tail $theOutFileName]]
    }
}

proc insertFileName { font } {
    global theFileNames
    global biggerVersion
    global smallerVersion
    set theFileNames($font) [tk_getOpenFile]
    if {$biggerVersion($font) != "" && $theFileNames($biggerVersion($font)) == ""} then {
        set theFileNames($biggerVersion($font)) $theFileNames($font)
    }
    if {$smallerVersion($font) != "" && $theFileNames($smallerVersion($font)) == ""} then {
        set theFileNames($smallerVersion($font)) $theFileNames($font)
    }
}


proc add_font_record { font name size id bigger smaller } {
    global theFileNames
    global theSize
    global styleId
    global biggerVersion
    global smallerVersion
    global fontList
    lappend fontList $font
    set styleId($font) $id
    set biggerVersion($font) $bigger
    set smallerVersion($font) $smaller
    set theSize($font) $size
    frame .frm($font)
    label .frm($font).name -text "$name: " -width 12
    entry .frm($font).fileName -width 45 -textvariable theFileNames($font)
    button .frm($font).browse -text "Browse..." -command "insertFileName $font"
    label .frm($font).sizeLabel -text "Size"
    entry .frm($font).size -width 5 -textvariable theSize($font)
    pack .frm($font)
    grid .frm($font).name .frm($font).fileName .frm($font).browse .frm($font).sizeLabel .frm($font).size
}

proc create_widgets {} {
    global forceEven
    global theOutFileName
    global theDBName
    global forceNBS
    global encoding
    global .frmEncoding.box
    add_font_record theFont        "Font"    18 "afnx22b8" "" ""
    frame .frmOutput
    label .frmOutput.fileNameLabel -text "Output filename: "
    entry .frmOutput.outFileName -width 45 -textvariable theOutFileName
    label .frmOutput.dbNameLabel -text "Palm name: "
    button .frmOutput.browse -text "Browse..." -command "getOutFileName"
    entry .frmOutput.dbName -width 30 -textvariable theDBName
    pack .frmOutput
    grid .frmOutput.fileNameLabel .frmOutput.outFileName .frmOutput.browse
    grid .frmOutput.dbNameLabel .frmOutput.dbName

    frame .frmGenerate
    label .frmGenerate.label -text "Font generation: "
    checkbutton .frmGenerate.forceNBS -text "Force some Latin 1 characters" -variable forceNBS
    checkbutton .frmGenerate.forceEven -text "Force even width" -variable forceEven
    set forceNBS 0
    set forceEven 1
    pack .frmGenerate
    grid .frmGenerate.label .frmGenerate.forceNBS .frmGenerate.forceEven

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
    button .frmCommands.go -text "Go!" -command go
    button .frmCommands.cancel -text "Exit" -command exit
    pack .frmCommands
    grid .frmCommands.go .frmCommands.cancel
}

mainline
