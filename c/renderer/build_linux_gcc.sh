SOURCE_FILES="
src/renderer.c
src/main.c
src/polygon.c
src/file.c
src/texture.c
"

gcc -std=c99 -pedantic -Wall $SOURCE_FILES -o bin/renderer -lm -lSDL2
