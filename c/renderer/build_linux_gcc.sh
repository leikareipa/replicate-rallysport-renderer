SOURCE_FILES="
src/renderer.c
src/main.c
src/polygon.c
"

gcc -std=c99 -pedantic -Wall $SOURCE_FILES -o bin/renderer -lm
