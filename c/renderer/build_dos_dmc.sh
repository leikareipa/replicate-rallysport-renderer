DMC_PATH=~/compi/dmc
OUTPUT_EXE_FILENAME="bin\rend.exe"
BUILD_OPTIONS="-a4 -f -mld -o+speed -wx -4 -o $OUTPUT_EXE_FILENAME"

SOURCE_FILES="
src/main.c
src/renderer/renderer.c
src/renderer/polygon.c
src/common/file.c
src/common/genstack.c
src/assets/mesh.c
src/assets/texture.c
src/assets/ground.c
"

wine "$DMC_PATH/bin/dmc.exe" $SOURCE_FILES $BUILD_OPTIONS -Isrc/ -I$DMC_PATH/include
rm *.obj
