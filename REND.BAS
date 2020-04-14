'
' 2020 Tarpeeksi Hyvae Soft
'
' Prototyping Rally-Sport's terrain and object rendering.
'
'

DECLARE SUB LoadVerticesFromDisk (numVertices%, dst() AS ANY)
DECLARE SUB Transform (numVertices%, vertices() AS ANY, transformedVertices() AS ANY, cameraPos AS ANY)
DECLARE SUB Render (numVertices%, vertices() AS ANY)
DECLARE SUB TransformVector (src AS ANY, dst AS ANY, cameraPos AS ANY)

SCREEN 13

CONST screenWidth = 320
CONST screenHeight = 200
CONST screenWidthHalf = (screenWidth / 2)
CONST screenHeightHalf = (screenHeight / 2)

TYPE vertex
    x AS SINGLE
    y AS SINGLE
    z AS SINGLE
END TYPE

' Load the 3d model's vertex data.
OPEN "gate.vtx" FOR INPUT AS #1
INPUT #1, numVertices%
DIM vertices(numVertices%) AS vertex
DIM transformedVertices(numVertices%) AS vertex
CALL LoadVerticesFromDisk(numVertices%, vertices())

DIM cameraPos AS vertex
cameraPos.x = 200
cameraPos.y = -700
cameraPos.z = -5

' Loop until the user presses 'q'.
DO
    ' Camera movement.
    SELECT CASE INKEY$
        CASE "q": EXIT DO
        CASE "e": cameraPos.z = (cameraPos.z + (20 / 256))
        CASE "d": cameraPos.z = (cameraPos.z - (20 / 256))
        CASE "s": cameraPos.x = (cameraPos.x - 20)
        CASE "f": cameraPos.x = (cameraPos.x + 20)
        CASE "w": cameraPos.y = (cameraPos.y + 20)
        CASE "r": cameraPos.y = (cameraPos.y - 20)
    END SELECT

    CALL Transform(numVertices%, vertices(), transformedVertices(), cameraPos)
    CALL Render(numVertices%, transformedVertices())

    COLOR 7: LOCATE 25, 1: PRINT "ESDF=Camera WR=Zoom Q=Quit";
LOOP

SUB LoadVerticesFromDisk (numVertices%, dst() AS vertex)
    FOR v = 0 TO (numVertices% - 1)
        INPUT #1, dst(v).x
        INPUT #1, dst(v).y
        INPUT #1, dst(v).z

        dst(v).x = -dst(v).x
        dst(v).y = -dst(v).y
        dst(v).z = (dst(v).z / 512)
    NEXT v
END SUB

SUB Render (numVertices%, vertices() AS vertex)
    ' Synchronize with screen refresh.
    WAIT &H3DA, 8
    WAIT &H3DA, 8, 8

    CLS

    FOR v = 0 TO (numVertices% - 1) STEP 4
        LINE (vertices(v + 0).x, vertices(v + 0).y)-(vertices(v + 1).x, vertices(v + 1).y), 4
        LINE (vertices(v + 1).x, vertices(v + 1).y)-(vertices(v + 2).x, vertices(v + 2).y), 4
        LINE (vertices(v + 2).x, vertices(v + 2).y)-(vertices(v + 3).x, vertices(v + 3).y), 4
        LINE (vertices(v + 3).x, vertices(v + 3).y)-(vertices(v + 0).x, vertices(v + 0).y), 4
    NEXT v
END SUB

SUB Transform (numVertices%, vertices() AS vertex, transformedVertices() AS vertex, cameraPos AS vertex)
    FOR v = 0 TO (numVertices% - 1)
        CALL TransformVector(vertices(v), transformedVertices(v), cameraPos)
    NEXT v
END SUB

SUB TransformVector (src AS vertex, dst AS vertex, cameraPos AS vertex)
    ' Perspective division to a vanishing point at the top center of the screen
    ' (at x=160, y=0 in VGA mode 13h).
    dst.x = (screenWidthHalf + ((cameraPos.x + src.x - screenWidthHalf) / (cameraPos.z + src.z)))
    dst.y = ((cameraPos.y + src.y) / (cameraPos.z + src.z))
END SUB

