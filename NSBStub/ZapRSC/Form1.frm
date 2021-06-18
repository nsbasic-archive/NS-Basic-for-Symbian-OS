VERSION 5.00
Begin VB.Form Form1 
   Caption         =   "Form1"
   ClientHeight    =   5985
   ClientLeft      =   60
   ClientTop       =   450
   ClientWidth     =   5490
   LinkTopic       =   "Form1"
   ScaleHeight     =   5985
   ScaleWidth      =   5490
   StartUpPosition =   3  'Windows Default
   Begin VB.TextBox Text1 
      Height          =   5535
      Left            =   1920
      MultiLine       =   -1  'True
      ScrollBars      =   2  'Vertical
      TabIndex        =   1
      Top             =   120
      Width           =   3015
   End
   Begin VB.CommandButton Command1 
      Caption         =   "Read"
      Height          =   375
      Left            =   120
      TabIndex        =   0
      Top             =   120
      Width           =   1455
   End
End
Attribute VB_Name = "Form1"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Private Sub Command1_Click()
   Dim s(500) As Byte
   Dim i As Integer
   Dim j As Integer
   Dim reslen As Integer
   Dim outputFile As String
   Dim inputfile As String
   Dim newName As String
   
   Dim app_name As String
   Dim resource_file As String
   Dim group_name As String
   Dim short_caption As String
   Dim caption As String
   Dim icon_file As String
   
   app_name = "HelloWorldBasic"
   resource_file = "\resource\apps\HelloWorldBasic"
   group_name = "NS Basic"
   short_caption = "HEWB"
   caption = "JelloWorld"
   icon_file = "\resource\apps\Helloworldbasic_aif.mif"
   
   'do reg resource
   inputfile = "e:\nsbs1\NSBStub\resources\NSBStub_reg.rsc_template"
   outputFile = "e:\nsbs1\NSBStub\resources\NSBStub_reg.rsc"
   
   reslen = readInputFile(inputfile, s)
   
   On Error Resume Next
   Kill outputFile
   On Error GoTo 0
   
   Open outputFile For Binary As #5
   copyBytes s, 0, 29
   writeString app_name
   copyBytes s, 47, 51
   writeString resource_file
   copyBytes s, 84, 92
   writeString group_name
   copyBytes s, 103, reslen
   
   Close #5
   
   'do regular resource
   inputfile = "e:\nsbs1\NSBStub\resources\NSBStub.rsc_template"
   outputFile = "e:\nsbs1\NSBStub\resources\NSBStub.rsc"
   
   reslen = readInputFile(inputfile, s)
   
   On Error Resume Next
   Kill outputFile
   On Error GoTo 0
   
   Open outputFile For Binary As #5
   copyBytes s, 0, 28
   Put #5, , CByte(Asc(Mid(short_caption, 1, 1)))
   Put #5, , CByte(Asc(Mid(short_caption, 2, 1)))
   Put #5, , CByte(Asc(Mid(short_caption, 3, 1)))
   Put #5, , CByte(Asc(Mid(short_caption, 4, 1)))
   copyBytes s, 33, 192
   writeString caption
   copyBytes s, 205, 213
   writeString caption
   copyBytes s, 226, 228
   writeString icon_file
   copyBytes s, 269, reslen
   
   Close #5
   
End Sub
Function readInputFile(inputfile As String, s() As Byte) As Integer
   Dim i As Integer
   i = -1
   Open inputfile For Binary As #5
   While EOF(5) = False
      i = i + 1
      Get #5, , s(i)
   Wend
   Close #5
   readInputFile = i - 1
End Function
Sub copyBytes(s() As Byte, fromByte As Integer, toByte As Integer)
   Dim i As Integer
   For i = fromByte To toByte
      Put #5, , CByte(s(i))
   Next
End Sub
Sub writeString(outString As String)
   Dim i As Integer
   Put #5, , CByte(Len(outString))
   Put #5, , CByte(Len(outString))
   For i = 1 To Len(outString)
      Put #5, , CByte(Asc(Mid(outString, i, 1)))
   Next
End Sub
