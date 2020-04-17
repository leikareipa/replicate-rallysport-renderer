SOURCE_FILES="
src/renderer.c
src/main.c
src/polygon.c
"

gcc -ansi -pedantic -Wall $SOURCE_FILES -o bin/renderer
