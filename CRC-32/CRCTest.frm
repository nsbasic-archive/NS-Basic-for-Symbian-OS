VERSION 5.00
Begin VB.Form Form1 
   Caption         =   "Form1"
   ClientHeight    =   8235
   ClientLeft      =   1650
   ClientTop       =   1545
   ClientWidth     =   6585
   LinkTopic       =   "Form1"
   ScaleHeight     =   8235
   ScaleWidth      =   6585
End
Attribute VB_Name = "Form1"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit

Private Sub Form_Load()
Dim s(19200) As Byte
Dim header(155) As Byte
Dim i As Integer
Dim inputfile As String
   inputfile = "c:\nsbasic\download\my.exe"
   i = readInputFile(inputfile, s)
   s(20) = &HA2
   s(21) = &HDA
   s(22) = &HF
   s(23) = &HC9
   For i = 0 To s(155)
      header(i) = s(i)
   Next
   MsgBox Hex(CalcCRC32(header))
   
End Sub

Function readInputFile(inputfile As String, s() As Byte) As Integer
   Dim i As Integer
   Dim FileNumber As Integer
   FileNumber = FreeFile
   i = -1
   Open inputfile For Binary As #FileNumber
   While EOF(FileNumber) = False
      i = i + 1
      Get #FileNumber, , s(i)
   Wend
   Close #FileNumber
   readInputFile = i - 1
End Function
