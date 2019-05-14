#!/bin/bash

CURR_DIR=$PWD
ROOT_DIR=${PWD}/../../..
OUT_DIR=${PWD}/out
BUILD_DIR=${PWD}/build

IWASM_ROOT=${PWD}/../../core/iwasm
APP_LIBS=${IWASM_ROOT}/lib/app-libs
NATIVE_LIBS=${IWASM_ROOT}/lib/native-interface
APP_LIB_SRC="${APP_LIBS}/base/*.c ${APP_LIBS}/extension/sensor/*.c ${NATIVE_LIBS}/*.c"
WASM_APPS=${PWD}/wasm-apps

rm -rf ${OUT_DIR}
mkdir ${OUT_DIR}
mkdir ${OUT_DIR}/wasm-apps

cd ${ROOT_DIR}/wamr/core/shared-lib/mem-alloc
if [ ! -d "tlsf" ]; then
    git clone https://github.com/mattconte/tlsf
fi

echo "#####################build simple project"
cd ${CURR_DIR}
mkdir -p cmake_build
cd cmake_build
cmake ..
make
if [ $? != 0 ];then
    echo "BUILD_FAIL simple exit as $?\n"
    exit 2
fi
cp -a simple ${OUT_DIR}
echo "#####################build simple project success"

echo "#####################build host-tool"
cd ${ROOT_DIR}/wamr/core/app-mgr/host-tool
mkdir -p bin
cd bin
cmake ..
make
if [ $? != 0 ];then
        echo "BUILD_FAIL host tool exit as $?\n"
        exit 2
fi
cp host_tool ${OUT_DIR}
echo "#####################build host-tool success"


echo "#####################build wasm apps"

cd ${CURR_DIR}

echo "#####################build wasm app timer"
APP_SRC="${WASM_APPS}/timer/main.c ${APP_LIB_SRC}"
emcc -O3 -I${APP_LIBS}/base -I${APP_LIBS}/extension/sensor -I${NATIVE_LIBS} \
     -s WASM=1 -s SIDE_MODULE=1 -s ASSERTIONS=1 -s STACK_OVERFLOW_CHECK=2 \
     -s TOTAL_MEMORY=65536 -s TOTAL_STACK=4096 \
     -s "EXPORTED_FUNCTIONS=['_on_init', '_on_destroy', '_on_request', '_on_response', \
                             '_on_sensor_event', '_on_timer_callback']" \
     -o ${OUT_DIR}/wasm-apps/timer.wasm ${APP_SRC}

echo "#####################build wasm app request_response/inter-host-wasm"
APP_SRC="${WASM_APPS}/request_response/inter-host-wasm/main.c ${APP_LIB_SRC}"
emcc -O3 -I${APP_LIBS}/base -I${APP_LIBS}/extension/sensor -I${NATIVE_LIBS} \
     -s WASM=1 -s SIDE_MODULE=1 -s ASSERTIONS=1 -s STACK_OVERFLOW_CHECK=2 \
     -s TOTAL_MEMORY=65536 -s TOTAL_STACK=4096 \
     -s "EXPORTED_FUNCTIONS=['_on_init', '_on_destroy', '_on_request', '_on_response', \
                             '_on_sensor_event', '_on_timer_callback']" \
     -o ${OUT_DIR}/wasm-apps/req-resp-inter-host-wasm.wasm ${APP_SRC}

echo "#####################build wasm app request_response/inter-wasm-apps/req"
APP_SRC="${WASM_APPS}/request_response/inter-wasm-apps/req/main.c ${APP_LIB_SRC}"
emcc -O3 -I${APP_LIBS}/base -I${APP_LIBS}/extension/sensor -I${NATIVE_LIBS} \
     -s WASM=1 -s SIDE_MODULE=1 -s ASSERTIONS=1 -s STACK_OVERFLOW_CHECK=2 \
     -s TOTAL_MEMORY=65536 -s TOTAL_STACK=4096 \
     -s "EXPORTED_FUNCTIONS=['_on_init', '_on_destroy', '_on_request', '_on_response', \
                             '_on_sensor_event', '_on_timer_callback']" \
     -o ${OUT_DIR}/wasm-apps/req-resp-inter-wasm-apps-req.wasm ${APP_SRC}

echo "#####################build wasm app request_response/inter-wasm-apps/resp"
APP_SRC="${WASM_APPS}/request_response/inter-wasm-apps/resp/main.c ${APP_LIB_SRC}"
emcc -O3 -I${APP_LIBS}/base -I${APP_LIBS}/extension/sensor -I${NATIVE_LIBS} \
     -s WASM=1 -s SIDE_MODULE=1 -s ASSERTIONS=1 -s STACK_OVERFLOW_CHECK=2 \
     -s TOTAL_MEMORY=65536 -s TOTAL_STACK=4096 \
     -s "EXPORTED_FUNCTIONS=['_on_init', '_on_destroy', '_on_request', '_on_response', \
                             '_on_sensor_event', '_on_timer_callback']" \
     -o ${OUT_DIR}/wasm-apps/req-resp-inter-wasm-apps-resp.wasm ${APP_SRC}

echo "#####################build wasm app event/inter-host-wasm"
APP_SRC="${WASM_APPS}/event/inter-host-wasm/main.c ${APP_LIB_SRC}"
emcc -O3 -I${APP_LIBS}/base -I${APP_LIBS}/extension/sensor -I${NATIVE_LIBS} \
     -s WASM=1 -s SIDE_MODULE=1 -s ASSERTIONS=1 -s STACK_OVERFLOW_CHECK=2 \
     -s TOTAL_MEMORY=65536 -s TOTAL_STACK=4096 \
     -s "EXPORTED_FUNCTIONS=['_on_init', '_on_destroy', '_on_request', '_on_response', \
                             '_on_sensor_event', '_on_timer_callback']" \
     -o ${OUT_DIR}/wasm-apps/event-inter-host-wasm.wasm ${APP_SRC}

echo "#####################build wasm app event/inter-wasm-apps/providers"
APP_SRC="${WASM_APPS}/event/inter-wasm-apps/providers/main.c ${APP_LIB_SRC}"
emcc -O3 -I${APP_LIBS}/base -I${APP_LIBS}/extension/sensor -I${NATIVE_LIBS} \
     -s WASM=1 -s SIDE_MODULE=1 -s ASSERTIONS=1 -s STACK_OVERFLOW_CHECK=2 \
     -s TOTAL_MEMORY=65536 -s TOTAL_STACK=4096 \
     -s "EXPORTED_FUNCTIONS=['_on_init', '_on_destroy', '_on_request', '_on_response', \
                             '_on_sensor_event', '_on_timer_callback']" \
     -o ${OUT_DIR}/wasm-apps/event-inter-wasm-apps-pub.wasm ${APP_SRC}

echo "#####################build wasm app event/inter-wasm-apps/subscriber"
APP_SRC="${WASM_APPS}/event/inter-wasm-apps/subscriber/main.c ${APP_LIB_SRC}"
emcc -O3 -I${APP_LIBS}/base -I${APP_LIBS}/extension/sensor -I${NATIVE_LIBS} \
     -s WASM=1 -s SIDE_MODULE=1 -s ASSERTIONS=1 -s STACK_OVERFLOW_CHECK=2 \
     -s TOTAL_MEMORY=65536 -s TOTAL_STACK=4096 \
     -s "EXPORTED_FUNCTIONS=['_on_init', '_on_destroy', '_on_request', '_on_response', \
                             '_on_sensor_event', '_on_timer_callback']" \
     -o ${OUT_DIR}/wasm-apps/event-inter-wasm-apps-sub.wasm ${APP_SRC}

echo "#####################build wasm app sensor"
APP_SRC="${WASM_APPS}/sensor/main.c ${APP_LIB_SRC}"
emcc -O3 -I${APP_LIBS}/base -I${APP_LIBS}/extension/sensor -I${NATIVE_LIBS} \
     -s WASM=1 -s SIDE_MODULE=1 -s ASSERTIONS=1 -s STACK_OVERFLOW_CHECK=2 \
     -s TOTAL_MEMORY=65536 -s TOTAL_STACK=4096 \
     -s "EXPORTED_FUNCTIONS=['_on_init', '_on_destroy', '_on_request', '_on_response', \
                             '_on_sensor_event', '_on_timer_callback']" \
     -o ${OUT_DIR}/wasm-apps/sensor.wasm ${APP_SRC}

echo "#####################build wasm apps success"
