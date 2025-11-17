# Copyright (C) 2019 Intel Corporation.  All rights reserved.
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

FROM gcc:12.2.0@sha256:a3e091325c0af43bc9c1c576ddd155351d5b16438124421188bb4b4fcacc1452 AS BASE

## set work directory
WORKDIR /root/
COPY resource /root/

# hadolint ignore=DL3008
RUN apt-get update  \
    && apt-get -y install make cmake --no-install-recommends

##  -clone wamr-repo and build iwasm
RUN git clone -b main --depth=1 https://github.com/bytecodealliance/wasm-micro-runtime.git \
    && mkdir -p /root/wasm-micro-runtime/product-mini/platforms/linux/build

WORKDIR /root/wasm-micro-runtime/product-mini/platforms/linux/build
RUN cmake .. -DWAMR_BUILD_DEBUG_INTERP=1 \
    && make \
    && cp /root/wasm-micro-runtime/product-mini/platforms/linux/build/iwasm /root/iwasm \
    && rm -fr /root/wasm-micro-runtime

FROM ubuntu:22.04@sha256:104ae83764a5119017b8e8d6218fa0832b09df65aae7d5a6de29a85d813da2fb
# COPY files from BASE image
COPY --from=BASE /root/iwasm /root
COPY --from=BASE /root/debug.sh /root
COPY --from=BASE /root/run.sh /root

WORKDIR /root/