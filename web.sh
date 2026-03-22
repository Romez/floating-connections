emcc main.cpp \
  -o docs/index.html \
  -Os -std=c++23 -Wall \
  -I../raylib/src \
  ./lib/libraylib.web.a \
  -lm \
  -s USE_GLFW=3 \
  -s ASYNCIFY \
  -s ASSERTIONS=2 \
  --preload-file resources \
  --shell-file minshell.html \
  -DPLATFORM_WEB
