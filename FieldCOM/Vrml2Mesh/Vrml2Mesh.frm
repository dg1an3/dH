VERSION 5.00
Begin VB.Form Form1 
   Caption         =   "Form1"
   ClientHeight    =   3195
   ClientLeft      =   60
   ClientTop       =   345
   ClientWidth     =   4680
   LinkTopic       =   "Form1"
   ScaleHeight     =   3195
   ScaleWidth      =   4680
   StartUpPosition =   3  'Windows Default
   Begin VB.TextBox Text1 
      Height          =   375
      Left            =   840
      TabIndex        =   1
      Text            =   "Text1"
      Top             =   2280
      Width           =   3135
   End
   Begin VB.CommandButton Command1 
      Caption         =   "Command1"
      Height          =   615
      Left            =   960
      TabIndex        =   0
      Top             =   480
      Width           =   2655
   End
End
Attribute VB_Name = "Form1"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Private Sub Command1_Click()

    ' convert the file to a Mesh
    Dim msh As New Mesh
    msh.Label = "patient_nuages"
    
    Dim fsys As New FileSystemObject
    Dim f As File
    Set f = fsys.GetFile("./patient.wrl")
    
    Dim ts As TextStream
    Set ts = f.OpenAsTextStream
    
    ' find the first line with "Coordinate3"
    Dim line As String
    Do While InStr(line, "Coordinate3") = 0
        line = ts.ReadLine
    Loop

    line = ts.ReadLine
    
    Dim vert As Matrix
    Set vert = msh.Vertices
    vert.Reshape 40000, 3
    
    lineCount = 0
    Do While InStr(line, "]") = 0
    
        ' extract the coordinate
        vert(lineCount, 0) = Val(line)
        line = Mid(line, InStr(line, " ") + 1)
        vert(lineCount, 2) = Val(line)
        line = Mid(line, InStr(line, " ") + 1)
        vert(lineCount, 1) = Val(line)
        
        line = ts.ReadLine
        lineCount = lineCount + 1
        
        If lineCount Mod 100 = 0 Then
            Text1.Text = "Coordinates: " & lineCount
            Text1.Refresh
        End If
    Loop
    
    vert.Reshape lineCount, 3
    
    
    Do While InStr(line, "Normal") = 0
        line = ts.ReadLine
    Loop

    line = ts.ReadLine
    
    Dim norm As Matrix
    Set norm = msh.Normals
    norm.Reshape 40000, 3
    
    lineCount = 0
    Do While InStr(line, "]") = 0
        
        ' extract the coordinate
        norm(lineCount, 0) = Val(line)
        line = Mid(line, InStr(line, " ") + 1)
        norm(lineCount, 2) = Val(line)
        line = Mid(line, InStr(line, " ") + 1)
        norm(lineCount, 1) = Val(line)
        If norm(lineCount, 1) = 0# And norm(lineCount, 2) = 0# And norm(lineCount, 3) = 0# Then
            ' Problem with normals
            norm(lineCount, 1) = 1#
        End If
        
        line = ts.ReadLine
        lineCount = lineCount + 1
        
        If lineCount Mod 100 = 0 Then
            Text1.Text = "Normals: " & lineCount
            Text1.Refresh
        End If
    Loop
    
    norm.Reshape lineCount, 3


    Do While InStr(line, "IndexedFaceSet") = 0
        line = ts.ReadLine
    Loop

    line = ts.ReadLine
    
    Dim index() As Long
    ReDim index(100000) As Long
    
    lineCount = 0
    Do While InStr(line, "]") = 0
        
        ' extract the coordinate
        index(lineCount) = Val(line)
        line = Mid(line, InStr(line, ",") + 1)
        If index(lineCount) < 0 Or index(lineCount) >= vert.Cols() Then
            MsgBox "Problem with indices"
        End If
        lineCount = lineCount + 1
        
        index(lineCount) = Val(line)
        line = Mid(line, InStr(line, ",") + 1)
        If index(lineCount) < 0 Or index(lineCount) >= vert.Cols() Then
            MsgBox "Problem with indices"
        End If
        lineCount = lineCount + 1
        
        index(lineCount) = Val(line)
        If index(lineCount) < 0 Or index(lineCount) >= vert.Cols() Then
            MsgBox "Problem with indices"
        End If
        lineCount = lineCount + 1
        
        line = ts.ReadLine
        
        If lineCount Mod 100 = 0 Then
            Text1.Text = "Index: " & lineCount
            Text1.Refresh
        End If
    Loop
    ReDim Preserve index(lineCount) As Long
    
    ' now check indices against normals
    lineCount = 0
    Do While InStr(line, "]") = 0
        
        ' extract the coordinate
        NewIndex = Val(line)
        line = Mid(line, InStr(line, ",") + 1)
        If index(lineCount) <> NewIndex Or index(lineCount) < 0 Or index(lineCount) >= vert.Cols() Then
            MsgBox "Problem with indices"
        End If
        lineCount = lineCount + 1
        
        NewIndex = Val(line)
        line = Mid(line, InStr(line, ",") + 1)
        If index(lineCount) <> NewIndex Or index(lineCount) < 0 Or index(lineCount) >= vert.Cols() Then
            MsgBox "Problem with indices"
        End If
        lineCount = lineCount + 1
        
        NewIndex = Val(line)
        If index(lineCount) <> NewIndex Or index(lineCount) < 0 Or index(lineCount) >= vert.Cols() Then
            MsgBox "Problem with indices"
        End If
        lineCount = lineCount + 1
        
        line = ts.ReadLine
        
        If lineCount Mod 100 = 0 Then
            Text1.Text = "Index: " & lineCount
            Text1.Refresh
        End If
    Loop
    
    ' set the indices
    msh.Indices = index
        
    ' save the mesh
    Dim fs As New FileStorage
    fs.Pathname = "./mesh.dat"
    fs.Save msh
    
End Sub
