#!/bin/bash

# emscripten environment, install instruction: https://emscripten.org/docs/getting_started/downloads.html
source ~/emsdk/emsdk_env.sh

# compile
EM_CC="em++"
EM_CFLAGS="-s WASM=1 -O3"
EM_LDFLAGS=$"--bind -s USE_SDL=2"
echo $EM_LDFALGS
${EM_CC} main.cpp ${EM_CFLAGS} ${EM_LDFLAGS} -o index.js -s EXTRA_EXPORTED_RUNTIME_METHODS='["ccall", "cwrap"]'

# start local webserver for testing
python3 -m http.server 8000
