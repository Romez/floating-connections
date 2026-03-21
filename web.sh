emcc main.cpp \
  -o docs/index.html \
  -Os -std=c++20 -Wall \
  -I../raylib/src \
  ./lib/libraylib.web.a \
  -lm \
  -s USE_GLFW=3 \
  -s ASYNCIFY \
  --preload-file resources \
  --shell-file minshell.html \
  -DPLATFORM_WEB