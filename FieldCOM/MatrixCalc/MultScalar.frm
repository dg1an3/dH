VERSION 5.00
Begin VB.Form MultScalar 
   Caption         =   "Scalar Multiply"
   ClientHeight    =   1950
   ClientLeft      =   60
   ClientTop       =   450
   ClientWidth     =   4680
   LinkTopic       =   "Form2"
   ScaleHeight     =   1950
   ScaleWidth      =   4680
   StartUpPosition =   3  'Windows Default
   Begin VB.TextBox txtScalar 
      Height          =   375
      Left            =   2400
      TabIndex        =   3
      Text            =   "1.0"
      Top             =   360
      Width           =   1935
   End
   Begin VB.CommandButton btnCancel 
      Caption         =   "Cancel"
      Height          =   495
      Left            =   2520
      TabIndex        =   1
      Top             =   1080
      Width           =   1695
   End
   Begin VB.CommandButton btnOK 
      Caption         =   "OK"
      Default         =   -1  'True
      Height          =   495
      Left            =   360
      TabIndex        =   0
      Top             =   1080
      Width           =   1695
   End
   Begin VB.Label Label1 
      Alignment       =   1  'Right Justify
      Caption         =   "Scalar:"
      Height          =   375
      Left            =   480
      TabIndex        =   2
      Top             =   360
      Width           =   1575
   End
End
Attribute VB_Name = "MultScalar"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Dim m As Matrix

Public Sub SetMatrix(mNew As Matrix)

    Set m = mNew
    
End Sub

Private Sub btnOK_Click()

    If IsNumeric(txtScalar) Then
    
        ' do the multiply
        m.MultScalar txtScalar
        
    End If
    
    ' done so hide the form
    Me.Hide
    
End Sub

Private Sub btnCancel_Click()
    
    ' hide the form
    Me.Hide

End Sub


