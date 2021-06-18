Attribute VB_Name = "CRC32"
Option Explicit

Public Function CalcCRC32(ByteArray() As Byte) As Long
Dim i As Long
Dim J As Long
Dim Limit As Long
Dim CRC As Long
Dim Temp1 As Long
Dim Temp2 As Long
Dim CRCTable(0 To 255) As Long
Dim s As String
Dim s1 As Variant
  
  Limit = &HEDB88320
  For i = 0 To 255
    CRC = i
    For J = 8 To 1 Step -1
      If CRC < 0 Then
        Temp1 = CRC And &H7FFFFFFF
        Temp1 = Temp1 \ 2
        Temp1 = Temp1 Or &H40000000
      Else
        Temp1 = CRC \ 2
      End If
      If CRC And 1 Then
        CRC = Temp1 Xor Limit
      Else
        CRC = Temp1
      End If
    Next J
    CRCTable(i) = CRC
  Next i
  
s = s & "&h0000,&h1021,&h2042,&h3063,&h4084,&h50a5,&h60c6,&h70e7,&h8108,&h9129,&ha14a,"
s = s & "&hb16b,&hc18c,&hd1ad,&he1ce,&hf1ef,&h1231,&h0210,&h3273,&h2252,&h52b5,&h4294,"
s = s & "&h72f7,&h62d6,&h9339,&h8318,&hb37b,&ha35a,&hd3bd,&hc39c,&hf3ff,&he3de,&h2462,"
s = s & "&h3443,&h0420,&h1401,&h64e6,&h74c7,&h44a4,&h5485,&ha56a,&hb54b,&h8528,&h9509,"
s = s & "&he5ee,&hf5cf,&hc5ac,&hd58d,&h3653,&h2672,&h1611,&h0630,&h76d7,&h66f6,&h5695,"
s = s & "&h46b4,&hb75b,&ha77a,&h9719,&h8738,&hf7df,&he7fe,&hd79d,&hc7bc,&h48c4,&h58e5,"
s = s & "&h6886,&h78a7,&h0840,&h1861,&h2802,&h3823,&hc9cc,&hd9ed,&he98e,&hf9af,&h8948,"
s = s & "&h9969,&ha90a,&hb92b,&h5af5,&h4ad4,&h7ab7,&h6a96,&h1a71,&h0a50,&h3a33,&h2a12,"
s = s & "&hdbfd,&hcbdc,&hfbbf,&heb9e,&h9b79,&h8b58,&hbb3b,&hab1a,&h6ca6,&h7c87,&h4ce4,"
s = s & "&h5cc5,&h2c22,&h3c03,&h0c60,&h1c41,&hedae,&hfd8f,&hcdec,&hddcd,&had2a,&hbd0b,"
s = s & "&h8d68,&h9d49,&h7e97,&h6eb6,&h5ed5,&h4ef4,&h3e13,&h2e32,&h1e51,&h0e70,&hff9f,"
s = s & "&hefbe,&hdfdd,&hcffc,&hbf1b,&haf3a,&h9f59,&h8f78,&h9188,&h81a9,&hb1ca,&ha1eb,"
s = s & "&hd10c,&hc12d,&hf14e,&he16f,&h1080,&h00a1,&h30c2,&h20e3,&h5004,&h4025,&h7046,"
s = s & "&h6067,&h83b9,&h9398,&ha3fb,&hb3da,&hc33d,&hd31c,&he37f,&hf35e,&h02b1,&h1290,"
s = s & "&h22f3,&h32d2,&h4235,&h5214,&h6277,&h7256,&hb5ea,&ha5cb,&h95a8,&h8589,&hf56e,"
s = s & "&he54f,&hd52c,&hc50d,&h34e2,&h24c3,&h14a0,&h0481,&h7466,&h6447,&h5424,&h4405,"
s = s & "&ha7db,&hb7fa,&h8799,&h97b8,&he75f,&hf77e,&hc71d,&hd73c,&h26d3,&h36f2,&h0691,"
s = s & "&h16b0,&h6657,&h7676,&h4615,&h5634,&hd94c,&hc96d,&hf90e,&he92f,&h99c8,&h89e9,"
s = s & "&hb98a,&ha9ab,&h5844,&h4865,&h7806,&h6827,&h18c0,&h08e1,&h3882,&h28a3,&hcb7d,"
s = s & "&hdb5c,&heb3f,&hfb1e,&h8bf9,&h9bd8,&habbb,&hbb9a,&h4a75,&h5a54,&h6a37,&h7a16,"
s = s & "&h0af1,&h1ad0,&h2ab3,&h3a92,&hfd2e,&hed0f,&hdd6c,&hcd4d,&hbdaa,&had8b,&h9de8,"
s = s & "&h8dc9,&h7c26,&h6c07,&h5c64,&h4c45,&h3ca2,&h2c83,&h1ce0,&h0cc1,&hef1f,&hff3e,"
s = s & "&hcf5d,&hdf7c,&haf9b,&hbfba,&h8fd9,&h9ff8,&h6e17,&h7e36,&h4e55,&h5e74,&h2e93,&h3eb2,&h0ed1,&h1ef0"
s1 = Split(s, ",")
For i = 0 To 255
   CRCTable(i) = s1(i)
   If i > 250 Then MsgBox Hex(CRCTable(i))
Next

  Limit = UBound(ByteArray)
  CRC = -1
  For i = 0 To Limit
    If CRC < 0 Then
      Temp1 = CRC And &H7FFFFFFF
      Temp1 = Temp1 \ 256
      Temp1 = (Temp1 Or &H800000) And &HFFFFFF
    Else
      Temp1 = (CRC \ 256) And &HFFFFFF
    End If
    Temp2 = ByteArray(i)   ' get the byte
    Temp2 = CRCTable((CRC Xor Temp2) And &HFF)
    CRC = Temp1 Xor Temp2
  Next i
  CRC = CRC Xor &HFFFFFFFF
  CalcCRC32 = CRC
End Function

