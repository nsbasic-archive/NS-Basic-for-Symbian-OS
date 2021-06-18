#!/usr/bin/wish
# $Id: ripfonts.tcl,v 1.4 2004/06/17 13:06:43 arpruss Exp $
#

proc mainline {} {
       create_widgets
}

proc go { } {
    global inFileName
    global argv0

    set progDir [file dirname $argv0]
    set extractFont "$progDir/extractfont"

    set fontsFound [exec "$extractFont" "$inFileName"]
    if { $fontsFound == "" } {
        tk_messageBox -title "Done!" -message "No PalmOS fonts found."
    } {
        tk_messageBox -title "Done!" -message "Extracted:\n$fontsFound."
    }
}

proc getInFileName { } {
    global inFileName
    set inFileName [tk_getOpenFile -defaultextension prc -filetypes { { {Palm databases} {*.pdb *.prc} } { {All files} * } } ]
}

proc create_widgets {} {
    global inFileName

    frame .frmInput
    label .frmInput.fileNameLabel -text "Filename: "
    entry .frmInput.inFileName -width 45 -textvariable inFileName
    button .frmInput.browse -text "Browse..." -command "getInFileName"
    pack .frmInput
    grid .frmInput.fileNameLabel .frmInput.inFileName .frmInput.browse

    frame .frmCommands
    button .frmCommands.go -text "Go!" -font bold -command "go"
    button .frmCommands.cancel -text "Exit" -command exit
    pack .frmCommands
    grid .frmCommands.go .frmCommands.cancel
}

mainline

