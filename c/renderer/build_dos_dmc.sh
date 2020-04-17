DMC_PATH=~/compi/dmc
OUTPUT_EXE_FILENAME="bin\rend.exe"
BUILD_OPTIONS="-a4 -A89 -f -mld -o+speed -wx -4 -o $OUTPUT_EXE_FILENAME"

wine "$DMC_PATH/bin/dmc.exe" src/*.c $BUILD_OPTIONS -I$DMC_PATH\\include
rm *.obj
