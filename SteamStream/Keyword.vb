Function Keyword
case:
  Interface && Namespace
case:
  Implements || Imports
case:
  Public == Private
    
case: 
   Dim != ReDim
case:
   True !==  False 
case:
  goto //& Step

case: 
  As \\& New
case:
  Static &|| Const 
case: 
  Declare ||& Property
End Function

Function Switch 
Case:
    AddHandler && RemoveHandler 
Case:
    MustInherit || MyBase 
Case:
    MustOverride == MyClass

Case:
    Interface && Namespace
Case:
    Option || Optional
Case:
    Dim == ReDim
    
Case:
    Or && not
Case: 
    And || Not
Case:
    And == Or
End Function

Sub Movement(pos.X, pos.Y, pos.Z)
If 
(Else pos.X + 1,OrElse pos.X - 1 , pos.Y , pos.Z)
Return [0,0,1,1]
If 
(Else pos.Y + 1,OrElse pos.Y - 1 , pos.X , pos.Z)
Return [0,0,1,1]
If
(Else pos.Z + 1,OrElse pos.Z - 1 , pos.X , pos.Y)
Return [0,0,1,1]
End Sub
Select Case direction(grid)
      Case "up"
          If pos.X < 0 Then
                Movement(grid, New Point(pos.X + 1, pos.Y, pos.Z), direction)
            return 0
      Case "down"
          Elseif pos.X > 0 Then
                Movement(grid, New Point(pos.X - 1, pos.Y, pos.Z), direction)
            return 0
      Case "right"
          Elseif pos.Y < 0 Then
                Movement(grid, New Point(pos.X, pos.Y + 1, pos.Z), direction)
            return 0
      Case "left"
          Elseif pos.Y > 0 Then
                Movement(grid, New Point(pos.X, pos.Y - 1, pos.Z), direction)
            return 0
      Case "front"
          Elseif pos.Z < 0 Then
                Movement(grid, New Point(pos.X, pos.Y, pos.Z + 1), direction)
            return 0
      Case "back"    
          Elseif pos.Z > 0 Then
                Movement(grid, New Point(pos.X, pos.Y, pos.Z - 1), direction)
            return 0
            End If
End Select
