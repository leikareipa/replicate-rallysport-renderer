'
' 2020 Tarpeeksi Hyvae Soft
'
' Prototyping n-sided polygon filling.
'
'

DECLARE FUNCTION lerp! (a!, b!, weight!)
DECLARE SUB Render (numVerts%, verts() AS ANY)
DECLARE SUB SortVerticesCCW (numVerts%, verts() AS ANY)
DECLARE SUB InitLerpValues (x!, clr!, vertexIdx%, verts() AS ANY)
DECLARE SUB InitLerpDeltas (deltaEndX, deltaRightColor, dir%, vertexIdx%, verts() AS ANY)

SCREEN 13

CONST screenWidth = 320
CONST screenHeight = 200

TYPE vertex
    x AS SINGLE
    y AS SINGLE
    z AS SINGLE
    clr AS SINGLE ' Color.
END TYPE

CONST numVerts = 6
DIM verts(numVerts) AS vertex

verts(1).x = 160
verts(1).y = 50
verts(1).clr = 22
verts(0).x = 100
verts(0).y = 100
verts(0).clr = 22
verts(2).x = 120
verts(2).y = 130
verts(2).clr = 22
verts(3).x = 200
verts(3).y = 115
verts(3).clr = 29
verts(4).x = 180
verts(4).y = 80
verts(4).clr = 29
verts(5) = verts(0)

' Main loop.
DO
    SELECT CASE INKEY$
        CASE "q": EXIT DO
        CASE "e"
            verts(0).y = verts(0).y - 1
            CALL Render(numVerts, verts())
        CASE "d"
            verts(0).y = verts(0).y + 1
            CALL Render(numVerts, verts())
        CASE "s"
            verts(0).x = verts(0).x - 1
            CALL Render(numVerts, verts())
        CASE "f"
            verts(0).x = verts(0).x + 1
            CALL Render(numVerts, verts())
    END SELECT
LOOP

SUB InitLerpDeltas (deltaX, deltaColor, dir%, vertexIdx%, verts() AS vertex)
    IF verts(vertexIdx%).y = verts(vertexIdx% + dir%).y THEN
        deltaX = 0
        deltaColor = 0
    ELSE
        height! = (verts(vertexIdx% + dir%).y - verts(vertexIdx%).y)

        deltaX = (verts(vertexIdx% + dir%).x - verts(vertexIdx%).x) / height!
        deltaColor = (verts(vertexIdx% + dir%).clr - verts(vertexIdx%).clr) / height!
    END IF
END SUB

SUB InitLerpValues (x, clr, vertexIdx%, verts() AS vertex)
    x = verts(vertexIdx%).x
    clr = verts(vertexIdx%).clr
END SUB

FUNCTION lerp! (a!, b!, weight!)
   lerp! = (a! + (weight! * (b! - a!)))
END FUNCTION

SUB Render (numVerts%, verts() AS vertex)
    CALL SortVerticesCCW(numVerts% - 1, verts())
    verts(numVerts% - 1) = verts(0)

    yPos% = verts(0).y
    leftVertIdx% = 0
    rightVertIdx% = (numVerts - 1)

    CALL InitLerpDeltas(deltaStartX, deltaLeftColor, 1, leftVertIdx%, verts())
    CALL InitLerpDeltas(deltaEndX, deltaRightColor, -1, rightVertIdx%, verts())
    CALL InitLerpValues(startX, leftColor, leftVertIdx%, verts())
    CALL InitLerpValues(endX, rightColor, rightVertIdx%, verts())

    ' Render the polygon.
    WAIT &H3DA, 8
    WAIT &H3DA, 8, 8
    CLS
    DO
        IF yPos% >= screenHeight THEN
            EXIT DO
        END IF

        IF yPos% = verts(leftVertIdx% + 1).y THEN
            leftVertIdx% = leftVertIdx% + 1

            CALL InitLerpDeltas(deltaStartX, deltaLeftColor, 1, leftVertIdx%, verts())
            CALL InitLerpValues(startX, leftColor, leftVertIdx%, verts())
        END IF

        IF yPos% = verts(rightVertIdx% - 1).y THEN
            rightVertIdx% = rightVertIdx% - 1

            CALL InitLerpDeltas(deltaEndX, deltaRightColor, -1, rightVertIdx%, verts())
            CALL InitLerpValues(endX, rightColor, rightVertIdx%, verts())
        END IF

        ' When we reach the bottom of the polygon, we're done.
        IF leftVertIdx% = rightVertIdx% THEN
            EXIT DO
        END IF

        currentColor = leftColor
        deltaCurrentColor = (rightColor - leftColor) / (endX - startX)
        FOR x = startX TO (endX - 1)
            PSET (x, yPos%), currentColor

            ' Increment horizontal deltas.
            currentColor = currentColor + deltaCurrentColor
        NEXT x

        ' Increment vertical deltas.
        startX = startX + deltaStartX
        endX = endX + deltaEndX
        leftColor = leftColor + deltaLeftColor
        rightColor = rightColor + deltaRightColor

        yPos% = yPos% + 1
    LOOP

    ' Draw a dot to mark each vertex.
    FOR v = 0 TO (numVerts - 2)
        PSET (verts(v).x, verts(v).y), v + 10
    NEXT v

    ' Print the corresponding vertex index next to each vertex.
    FOR v = 0 TO (numVerts - 2)
       ' LOCATE verts(v).y / 8, verts(v).x / 8: PRINT v;
    NEXT v
END SUB

SUB SortVerticesCCW (numVerts%, verts() AS vertex)
    ' Sort by Y coordinate, from top to bottom on screen.
    DO
        finished% = 1
        FOR i% = 1 TO (numVerts% - 1)
            IF verts(i% - 1).y > verts(i%).y THEN
                SWAP verts(i%), verts(i% - 1)
                finished% = 0
            END IF
        NEXT i%
    LOOP WHILE finished% = 0

    DIM topVert AS vertex
    DIM bottomVert AS vertex
    DIM leftVerts(numVerts%) AS vertex
    DIM rightVerts(numVerts%) AS vertex

    topVert = verts(0)
    bottomVert = verts(numVerts% - 1)

    leftVerts(0) = topVert

    numLeftVerts% = 1
    numRightVerts% = 0

    FOR i% = 1 TO (numVerts% - 2)
        lr = lerp(topVert.x, bottomVert.x, ((verts(i%).y - topVert.y) / (bottomVert.y - topVert.y)))

        IF verts(i%).x < lr THEN
            leftVerts(numLeftVerts%) = verts(i%)
            numLeftVerts% = numLeftVerts% + 1
        ELSE
            rightVerts(numRightVerts%) = verts(i%)
            numRightVerts% = numRightVerts% + 1
        END IF
    NEXT i%

    leftVerts(numLeftVerts%) = bottomVert
    numLeftVerts% = numLeftVerts% + 1
    
    FOR i% = 0 TO (numLeftVerts% - 1)
        verts(i%) = leftVerts(i%)
    NEXT i%

    FOR i% = 0 TO (numRightVerts% - 1)
        verts(i% + numLeftVerts%) = rightVerts(numRightVerts% - i% - 1)
    NEXT i%
END SUB

