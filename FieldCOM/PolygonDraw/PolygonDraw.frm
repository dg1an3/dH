VERSION 5.00
Object = "{5E9E78A0-531B-11CF-91F6-C2863C385E30}#1.0#0"; "msflxgrd.ocx"
Begin VB.Form PolygonDrawForm 
   Caption         =   "PolygonDraw"
   ClientHeight    =   6810
   ClientLeft      =   60
   ClientTop       =   450
   ClientWidth     =   6990
   LinkTopic       =   "Form1"
   ScaleHeight     =   6810
   ScaleWidth      =   6990
   StartUpPosition =   3  'Windows Default
   Begin VB.CommandButton Command1 
      Caption         =   "Command1"
      Height          =   495
      Left            =   4800
      TabIndex        =   5
      Top             =   6120
      Width           =   1935
   End
   Begin VB.CommandButton btnSave 
      Caption         =   "Save Polygon"
      Height          =   495
      Left            =   2400
      TabIndex        =   4
      Top             =   6120
      Width           =   1815
   End
   Begin VB.CommandButton btnLoad 
      Caption         =   "Load Polygon"
      Height          =   495
      Left            =   360
      TabIndex        =   3
      Top             =   6120
      Width           =   1815
   End
   Begin MSFlexGridLib.MSFlexGrid MSFlexGrid1 
      Height          =   855
      Left            =   240
      TabIndex        =   1
      Top             =   840
      Width           =   6495
      _ExtentX        =   11456
      _ExtentY        =   1508
      _Version        =   393216
      Cols            =   0
      FixedRows       =   0
      FixedCols       =   0
   End
   Begin VB.PictureBox Picture1 
      Height          =   3735
      Left            =   240
      ScaleHeight     =   3675
      ScaleWidth      =   6435
      TabIndex        =   0
      Top             =   2040
      Width           =   6495
   End
   Begin VB.Label Label1 
      Caption         =   "Vertices:"
      Height          =   255
      Left            =   240
      TabIndex        =   2
      Top             =   480
      Width           =   2535
   End
End
Attribute VB_Name = "PolygonDrawForm"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Dim poly As New Polygon3D
Dim vert As Matrix
Dim bDrawing As Boolean

Private Sub AddVertex(X As Single, Y As Single)

    ' add another column to the matrix, setting 3 for the third vector element
    poly.AddVertex X, Y
    
    ' add another column to the grid view
    MSFlexGrid1.Cols = vert.Cols
    MSFlexGrid1.Col = vert.Cols - 1
    
    MSFlexGrid1.Row = 0
    MSFlexGrid1 = vert.Element(vert.Cols - 1, 0)
    
    MSFlexGrid1.Row = 1
    MSFlexGrid1 = vert.Element(vert.Cols - 1, 1)
    
    ' refresh the picture
    Picture1.Refresh
    
End Sub

Private Sub Command1_Click()

    For AtVertex = 0 To vert.Cols - 2
                
        MsgBox "Vertex Columns = " & vert.Cols
        
        ' draw a line
        MsgBox "Vertex " & AtVertex & " : " & vert(AtVertex, 0) & "," & vert(AtVertex, 1) & "-" _
            & vert(AtVertex + 1, 0) & "," & vert(AtVertex + 1, 1)
            
    Next AtVertex
    
End Sub

Private Sub Form_Load()

    ' helper accessor for vertices matrix
    Set vert = poly.Vertices
    
    ' setting drawing mode to False
    bDrawing = False
    
End Sub

Private Sub Picture1_DblClick()

    ' close the polygon
    AddVertex vert(1, 0), vert(1, 1)
    
End Sub

Private Sub Picture1_MouseDown(Button As Integer, Shift As Integer, X As Single, Y As Single)

    ' add the new vertex
    AddVertex X, Y
    
    ' setting drawing mode to True
    bDrawing = True
    
End Sub

Private Sub Picture1_MouseMove(Button As Integer, Shift As Integer, X As Single, Y As Single)

    ' don't process if we're not drawing
    If Not bDrawing Then Exit Sub
    
    ' add the new vertex
    AddVertex X, Y

End Sub

Private Sub Picture1_MouseUp(Button As Integer, Shift As Integer, X As Single, Y As Single)

    ' setting drawing mode to False
    bDrawing = False
    
End Sub

Private Sub Picture1_Paint()
    
    For AtVertex = 0 To vert.Cols - 2
                
        ' draw a line
        Picture1.Line (vert(AtVertex, 0), vert(AtVertex, 1)) _
            -(vert(AtVertex + 1, 0), vert(AtVertex + 1, 1))
            
    Next AtVertex
    
End Sub

Private Sub btnLoad_Click()

    ' load the polygon
    Dim MyStorage As New FileStorage
    MyStorage.Pathname = "C:\Polygon.dat"
    MyStorage.Load poly
    
    ' reshape the grid
    MSFlexGrid1.Cols = vert.Cols
    
    ' Populate the grid
    For nCol = 0 To vert.Cols - 1
    
        ' set the column
        MSFlexGrid1.Col = nCol
        
        ' add another column to the grid view
        MSFlexGrid1.Row = 0
        MSFlexGrid1 = vert.Element(nCol, 0)
        
        MSFlexGrid1.Row = 1
        MSFlexGrid1 = vert.Element(nCol, 1)
        
    Next nCol
    
    ' refresh the picture
    Picture1.Refresh
    
End Sub


Private Sub btnSave_Click()

    ' save the polygon
    Dim MyStorage As New FileStorage
    MyStorage.Pathname = "C:\Polygon.dat"
    MyStorage.Save poly
    
End Sub


