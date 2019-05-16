Introduction
==============
Littlevgl is an Open-source Embedded GUI Library. We definde an UI APP, which can easily run on Native or on WASM VM. There are 3 binaries for test two scenarios.
1. Native Linux. The App code built into Linux executables.
2. WASM VM for Different platform. WASM VM and native extension being built into Linux and Zephyr platforms. With WASM VM inside, many WASM APP can run on top of it.
3. WASM APP. This kind of binary can be run on WASM VM.

Directory structure
--------------------------------
<pre>
├── build.sh
├── LICENCE.txt
├── README.md
├── UI.JPG
├── vgl-native-ui-app
│   ├── CMakeLists.txt
│   ├── lv_drivers
│   │   ├── display_indev.h
│   │   ├── indev
│   │   │   ├── mouse.c
│   │   │   └── mouse.h
│   │   ├── linux_display_indev.c
│   │   ├── lv_conf.h
│   │   └── system_header.h
│   └── main.c
├── vgl-wasm-runtime
│   ├── CMakeLists.txt
│   ├── src
│   │   ├── display_indev.h
│   │   ├── ext-lib-export.c
│   │   └── platform
│   │       ├── linux
│   │       │   ├── display_indev.c
│   │       │   ├── iwasm_main.c
│   │       │   ├── main.c
│   │       │   └── mouse.c
│   │       └── zephyr
│   │           ├── board_config.h
│   │           ├── display.h
│   │           ├── display_ili9340_adafruit_1480.c
│   │           ├── display_ili9340.c
│   │           ├── display_ili9340.h
│   │           ├── display_indev.c
│   │           ├── iwasm_main.c
│   │           ├── LICENSE
│   │           ├── main.c
│   │           ├── pin_config_jlf.h
│   │           ├── pin_config_stm32.h
│   │           ├── XPT2046.c
│   │           └── XPT2046.h
│   └── zephyr-build
│       ├── CMakeLists.txt
│       └── prj.conf
└── wasm-apps
    ├── build_wasm_app.sh
    ├── Makefile_wasm_app
    ├── src
    │   ├── display_indev.h
    │   ├── lv_conf.h
    │   ├── main.c
    │   └── system_header.h
    └── ui_app.wasm

- build.sh
  This build sto build and binaries.
- LICENCE.txt
- UI.JPG
- user_guide.md
- vgl-native-ui-app
  Littlevgl graphics app has being built into Linux application named "vgl_native_ui_app", which can directly run on Linux.
- vgl-wasm-runtime
  Wasm micro-runtime and littlevgl native interface built into Linux application named "littlevgl", where wasm apps can run on it.
- wasm-apps
  A wasm app with littlevgl graphics.
</pre>

Install required SDK and libraries
==============
- 32 bit SDL(simple directmedia layer) 
Use apt-get
    sudo apt-get install libsdl2-dev:i386<br>
Or, install from source   
<pre>
   Download source from www.libsdl.org
    `./configure C_FLAGS=-m32 CXX_FLAGS=-m32 LD_FLAGS=-m32`
     ` make`
    `sudo make install`
</pre>
- Install EMSDK
    https://emscripten.org/docs/tools_reference/emsdk.html
- Cmake
     CMAKE version above 3.13.1.

Build & Run
==============

Build and run on Linux
--------------------------------
- Build
`./build.sh`
    All binaries are in "out", which contains "host_tool", "vgl_native_ui_app", "TestApplet1.wasm" and "vgl_wasm_runtime".
- Run native Linux application
<pre>
`./vgl_native_ui_app`
<img src="./UI.JPG">
The number on top will plus one each second, and the number on the bottom will plus one when clicked.
</pre>
- Run WASM VM Linux applicaton & install WASM APP
<pre>
 First start vgl_wasm_runtime in server mode.
`./vgl_wasm_runtime -s`
 Then install wasm APP use host tool.
`./host_tool -i TestApplet1 -f TestApplet1.wasm`
</pre>

Build and run on zephyr
--------------------------------
WASM VM and native extension method can be built into zephyr, Then we can install wasm app into STM32.
- Build wasm into Zephyr system
<pre>
 a. clone zephyr source code
`git clone https://github.com/zephyrproject-rtos/zephyr.git`
 b. copy samples
    `cd zephyr/samples/`
    `cp -a <iwasm_root_dir>samples/littlevgl/vgl-wasm-runtime vgl-wasm-runtime`
    `cd vgl-wasm-runtime/zephyr_build`
 c. create a link to wamr core
   ` ln -s <iwasm_root_dir>/core core`
 d. build source code
    Since ui_app incorporated littlevgl source code, so it need more RAM on device to install it.
    It is recommended that RAM SIZE greater than 512KB.
    In our test use nucleo_f767zi, which are not supported by zephyr.
    However nucleo_f767zi is almost the same as nucleo_f746zg, except FLASH and SRAM size.
    So we changed the DTS setting of nucleo_f746zg boards for workaround.

    `Modify zephyr/dts/arm/st/f7/stm32f746xg.dtsi, change DT_SIZE_K(320) to DT_SIZE_K(512)`
    `mkdir build && cd build`
    `source ../../../../zephyr-env.sh`
    `cmake -GNinja -DBOARD=nucleo_f746zg ..`
   ` ninja flash`
</pre>
- Test on STM32 NUCLEO_F767ZI with ILI9341 Display with XPT2046 touch.
Hardware Connetions
<pre>
+-------------------+-+------------------+
|NUCLEO-F767ZI || ILI9341  Display |
+-------------------+-+------------------+
| CN7.10               |         CLK     |
+-------------------+-+------------------+
| CN7.12               |         MISO    |
+-------------------+-+------------------+
| CN7.14               |         MOSI    |
+-------------------+-+------------------+
| CN11.1               | CS1 for ILI9341 |
+-------------------+-+------------------+
| CN11.2               |         D/C     |
+-------------------+-+------------------+
| CN11.3               |         RESET   |
+-------------------+-+------------------+
| CN9.25               |    PEN interrupt|
+-------------------+-+------------------+
| CN9.27               | CS2 for XPT2046 |
+-------------------+-+------------------+
| CN10.14             | PC UART RX       |
+-------------------+-+------------------+
| CN11.16             | PC UART RX       |
+-------------------+-+------------------+
</pre>
- Install wasm app to zephyr using host_tool
First connect PC and STM32 with UART. Then install use host_tool.
`./host_tool -D /dev/ttyUSBXXX -i ui_app -f ui_app.wasm`

