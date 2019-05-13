Build the Zephyr image
=======================


Build the linux version runtime with littlevgl native interface support
========================================================================


Build the UI application into the linux execuable with SDL:
===========================================================
0. goto build/native-ui-app
1. mkdir build && cd build
2. cmake ..
3. make
lvgl will be fetched in cmake.



wasm-apps
=========
The folder for wasm applications

build procedure:
goto wasm-apps
./build_wasm.sh
If no "lvgl" folder, will git clone it.


