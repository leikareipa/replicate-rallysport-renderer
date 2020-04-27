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

gcc -std=c99 -g -pedantic -Wall -Isrc/ $SOURCE_FILES -o bin/renderer -lm -lSDL2
