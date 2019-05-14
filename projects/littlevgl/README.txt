There are 3 sub-folders for 3 binaries.
    native-ui-app   : littlevgl graphics app built into Linux application named "ui_app", which can directly run on Linux.
    vgl-wasm-runtime: wasm micro-runtime and littlevgl native interface built into Linux application named "littlevgl", where wasm apps run on it.
    wasm-apps       : A wasm app with littlevgl graphics.

Install required SDK and libraries.
=======================
1. 32 bit SDL(simple directmedia layer) Library is required
    a. sudo apt-get install libsdl2-dev:i386
    b. Alternatively it can be download from www.libsdl.org, then 
        ./configure C_FLAGS=-m32 CXX_FLAGS=-m32 LD_FLAGS=-m32
        ./make
        ./sudo make install
2. Install EMSDK
    https://emscripten.org/docs/tools_reference/emsdk.html

Build out binaries
=======================
./build.sh
All binaries are in ./out, which contains "host_tool", "native_ui_app", "TestApplet1.wasm" and "vgl_wasm_runtime".


Run
=======================
1. Run UI app directly on Linux.
    ./native_ui_app will show UI app.
2. Run wasm vm then install the UI app.
    ./vgl_wasm_runtime -s
    ./host_tool -i TestApplet1.wasm -f TestApplet1.wasm
