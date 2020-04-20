DMC_PATH=~/compi/dmc
OUTPUT_EXE_FILENAME="bin\rend.exe"
BUILD_OPTIONS="-a4 -f -mld -o+speed -wx -4 -o $OUTPUT_EXE_FILENAME"

SOURCE_FILES="
src/renderer.c
src/main.c
src/polygon.c
src/file.c
src/texture.c
src/generic_stack.c
src/mesh.c
"

wine "$DMC_PATH/bin/dmc.exe" $SOURCE_FILES $BUILD_OPTIONS -I$DMC_PATH\\include
rm *.obj
