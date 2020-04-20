SOURCE_FILES="
src/renderer.c
src/main.c
src/polygon.c
src/file.c
src/texture.c
src/generic_stack.c
src/mesh.c
"

gcc -std=c99 -g -pedantic -Wall $SOURCE_FILES -o bin/renderer -lm -lSDL2
