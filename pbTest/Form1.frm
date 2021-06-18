VERSION 5.00
Begin VB.Form Form1 
   Caption         =   "Form1"
   ClientHeight    =   5505
   ClientLeft      =   60
   ClientTop       =   450
   ClientWidth     =   5370
   LinkTopic       =   "Form1"
   ScaleHeight     =   5505
   ScaleWidth      =   5370
   StartUpPosition =   3  'Windows Default
   Begin VB.CommandButton Command2 
      Caption         =   "Command2"
      Height          =   375
      Left            =   600
      TabIndex        =   2
      Top             =   240
      Width           =   1215
   End
   Begin VB.PictureBox Picture1 
      Height          =   3615
      Left            =   240
      Picture         =   "Form1.frx":0000
      ScaleHeight     =   237
      ScaleMode       =   3  'Pixel
      ScaleWidth      =   277
      TabIndex        =   1
      Top             =   960
      Width           =   4215
   End
   Begin VB.CommandButton Command1 
      Caption         =   "Command1"
      Height          =   375
      Left            =   4080
      TabIndex        =   0
      Top             =   240
      Width           =   1095
   End
End
Attribute VB_Name = "Form1"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False

Private Sub Command1_Click()
Dim pb

Picture1.AutoSize = True
Picture1.Picture = LoadPicture("e:\alexandra.jpg")

If 1 = 0 Then
Set pb = Me.Controls.Add("NSCEPictureBox.NSCEPictureBoxCtrl.1", "x")
pb.object.ScaleMode = 3
pb.object.Top = 10
pb.object.Left = 10
pb.object.AutoSize = True
pb.object.Picture = "e:\alexandra.jpg"
pb.Visible = True
pb.object.Refresh
MsgBox pb.object.Width & " " & pb.object.Height
Controls.Remove (pb)
Set pb = Nothing
End If

End Sub

Function pb_Click()
   MsgBox "pb click"
End Function

Private Sub Command2_Click()
Picture1.AutoRedraw = True
Picture1.Picture = LoadPicture("e:\alexandra.jpg")
Picture1.PaintPicture Picture1.Picture, 0, 0, Picture1.ScaleWidth, Picture1.ScaleHeight
Picture1.Picture = Picture1.Image
End Sub
