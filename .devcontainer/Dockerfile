# Copyright (C) 2019 Intel Corporation.  All rights reserved.
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

# See here for image contents: https://github.com/microsoft/vscode-dev-containers/tree/v0.245.2/containers/cpp/.devcontainer/base.Dockerfile
# [Choice] Debian / Ubuntu version (use Debian 12/11/9, Ubuntu 18.04/21.04 on local arm64/Apple Silicon): debian-12, debian-11, debian-10, debian-9, ubuntu-21.04, ubuntu-20.04, ubuntu-18.04
ARG VARIANT=jammy
FROM mcr.microsoft.com/devcontainers/base:${VARIANT}

ARG DEBIAN_FRONTEND=noninteractive
ENV TZ=Asian/Shanghai

# hadolint ignore=DL3008
RUN apt-get update \
  && apt-get upgrade -y \
  && apt-get install -y apt-transport-https apt-utils build-essential \
  ca-certificates ccache clang curl g++-multilib git gnupg \
  libgcc-12-dev lib32gcc-12-dev libstdc++-12-dev lsb-release llvm \
  ninja-build opam ocaml ocamlbuild \
  software-properties-common tree tzdata \
  unzip valgrind vim wget zip --no-install-recommends

#
# binaryen
ARG BINARYEN_VER=114
WORKDIR /opt
RUN wget -c --progress=dot:giga https://github.com/WebAssembly/binaryen/releases/download/version_${BINARYEN_VER}/binaryen-version_${BINARYEN_VER}-x86_64-linux.tar.gz \
  && tar xf binaryen-version_${BINARYEN_VER}-x86_64-linux.tar.gz \
  && ln -sf /opt/binaryen-version_${BINARYEN_VER} /opt/binaryen \
  && rm binaryen-version_${BINARYEN_VER}-x86_64-linux.tar.gz

#
# CMAKE (https://apt.kitware.com/)
SHELL ["/bin/bash", "-o", "pipefail", "-c"]
# hadolint ignore=DL3008
ARG CMAKE_VER=3.27.0
RUN wget https://github.com/Kitware/CMake/releases/download/v${CMAKE_VER}/cmake-${CMAKE_VER}-Linux-x86_64.sh \
  -q -O /tmp/cmake-install.sh \
  && chmod u+x /tmp/cmake-install.sh \
  && mkdir /opt/cmake-${CMAKE_VER} \
  && /tmp/cmake-install.sh --skip-license --prefix=/opt/cmake-${CMAKE_VER} \
  && rm /tmp/cmake-install.sh \
  && ln -s /opt/cmake-${CMAKE_VER}/bin/* /usr/local/bin

#
# install emsdk
WORKDIR /opt
RUN git clone https://github.com/emscripten-core/emsdk.git

ARG EMSDK_VER=3.1.43
WORKDIR /opt/emsdk
RUN  git pull \
  && ./emsdk install ${EMSDK_VER} \
  && ./emsdk activate ${EMSDK_VER} \
  && echo "source /opt/emsdk/emsdk_env.sh" >> /root/.bashrc

#
# install wasi-sdk
ARG WASI_SDK_VER=24
RUN wget -c --progress=dot:giga https://github.com/WebAssembly/wasi-sdk/releases/download/wasi-sdk-${WASI_SDK_VER}/wasi-sdk-${WASI_SDK_VER}.0-x86_64-linux.tar.gz -P /opt \
  && tar xf /opt/wasi-sdk-${WASI_SDK_VER}.0-x86_64-linux.tar.gz -C /opt \
  && ln -sf /opt/wasi-sdk-${WASI_SDK_VER}.0-x86_64-linux /opt/wasi-sdk \
  && rm /opt/wasi-sdk-${WASI_SDK_VER}.0-x86_64-linux.tar.gz

#
#install wabt
ARG WABT_VER=1.0.34
RUN wget -c --progress=dot:giga https://github.com/WebAssembly/wabt/releases/download/${WABT_VER}/wabt-${WABT_VER}-ubuntu-20.04.tar.gz -P /opt \
  && tar xf /opt/wabt-${WABT_VER}-ubuntu-20.04.tar.gz -C /opt \
  && ln -sf /opt/wabt-${WABT_VER} /opt/wabt \
  && rm /opt/wabt-${WABT_VER}-ubuntu-20.04.tar.gz

#
# install bazelisk
ARG BAZELISK_VER=1.17.0
RUN mkdir /opt/bazelisk \
  && wget -c --progress=dot:giga https://github.com/bazelbuild/bazelisk/releases/download/v${BAZELISK_VER}/bazelisk-linux-amd64 -P /opt/bazelisk \
  && chmod a+x /opt/bazelisk/bazelisk-linux-amd64 \
  && ln -fs /opt/bazelisk/bazelisk-linux-amd64 /opt/bazelisk/bazel

#
# install clang+llvm
ARG LLVM_VER=15.0.6
RUN wget https://github.com/llvm/llvm-project/releases/download/llvmorg-${LLVM_VER}/clang+llvm-${LLVM_VER}-x86_64-linux-gnu-ubuntu-18.04.tar.xz
RUN tar -xvf clang+llvm-${LLVM_VER}-x86_64-linux-gnu-ubuntu-18.04.tar.xz \
  && mv clang+llvm-${LLVM_VER}-x86_64-linux-gnu-ubuntu-18.04 llvm-${LLVM_VER} \
  && rm clang+llvm-${LLVM_VER}-x86_64-linux-gnu-ubuntu-18.04.tar.xz

#
# Install pip
# hadolint ignore=DL3008
RUN apt-get update \
  && apt-get install -y --reinstall python3-venv python3-pip --no-install-recommends \
  && pip install --no-cache-dir --upgrade pip

#
# Install github-cli. It doens't work as a feature of devcontainer.json
ARG GH_CLI_VER=2.32.0
WORKDIR /tmp
RUN wget -q https://github.com/cli/cli/releases/download/v${GH_CLI_VER}/gh_${GH_CLI_VER}_linux_amd64.deb \
  && dpkg -i gh_${GH_CLI_VER}_linux_amd64.deb

#
# Install NodeJS
# RUN wget -qO- https://deb.nodesource.com/setup_20.x | bash -
# # hadolint ignore=DL3008
# RUN apt-get install -y nodejs --no-install-recommends

#
# SGX setup
RUN mkdir -p /opt/intel

WORKDIR /opt/intel
RUN wget -q -O sgx_linux_x64_sdk.bin https://download.01.org/intel-sgx/latest/linux-latest/distro/ubuntu22.04-server/sgx_linux_x64_sdk_2.25.100.3.bin \
  && chmod +x sgx_linux_x64_sdk.bin
RUN echo 'yes' | ./sgx_linux_x64_sdk.bin
RUN source /opt/intel/sgxsdk/environment

RUN echo 'deb [arch=amd64] https://download.01.org/intel-sgx/sgx_repo/ubuntu jammy main' | sudo tee /etc/apt/sources.list.d/intel-sgx.list \
   && wget -qO - https://download.01.org/intel-sgx/sgx_repo/ubuntu/intel-sgx-deb.key | apt-key add -

RUN apt update \
   && apt install -y libsgx-launch libsgx-urts

#
# Install wasm-tools from binary releasing
ARG WASM_TOOLS_VER=1.201.0
WORKDIR /opt
RUN wget -c --progress=dot:giga https://github.com/bytecodealliance/wasm-tools/releases/download/v${WASM_TOOLS_VER}/wasm-tools-${WASM_TOOLS_VER}-x86_64-linux.tar.gz
RUN tar xzf wasm-tools-${WASM_TOOLS_VER}-x86_64-linux.tar.gz \
  && rm wasm-tools-${WASM_TOOLS_VER}-x86_64-linux.tar.gz \
  && ln -sf /opt/wasm-tools-${WASM_TOOLS_VER}-x86_64-linux/wasm-tools /usr/bin/wasm-tools

#
# Install wit-bindgen
ARG WIT_BINDGEN_VER=0.21.0
WORKDIR /opt
RUN wget -c --progress=dot:giga https://github.com/bytecodealliance/wit-bindgen/releases/download/v${WIT_BINDGEN_VER}/wit-bindgen-${WIT_BINDGEN_VER}-x86_64-linux.tar.gz
RUN tar xzf wit-bindgen-${WIT_BINDGEN_VER}-x86_64-linux.tar.gz \
  && rm wit-bindgen-${WIT_BINDGEN_VER}-x86_64-linux.tar.gz

#
# Install perf
ARG KERNEL_VER=5.15.0-97
RUN apt update \
      && apt-get install -y linux-tools-generic linux-tools-${KERNEL_VER}-generic \
      && apt-get install -y linux-headers-${KERNEL_VER}

#
# Install extra clang tools and lldb
RUN apt update \
  && apt install -y clang-tools clang-tidy clang-format clangd \
  && apt install -y lldb python3-lldb

#
# Install Rust
ENV CARGO_HOME=/home/vscode/.cargo
ENV RUSTUP_HOME=/home/vscode/.rustup
RUN curl -sSf https://sh.rustup.rs | sh -s -- -y
ENV PATH="${CARGO_HOME}/bin:${PATH}"

#
# Install CodeQL CLI
RUN cd /opt \
  && wget https://github.com/github/codeql-action/releases/download/codeql-bundle-v2.19.2/codeql-bundle-linux64.tar.gz \
  && tar xf codeql-bundle-linux64.tar.gz \
  && rm codeql-bundle-linux64.tar.gz
ENV PATH="/opt/codeql:${PATH}"

# set path
ENV PATH="/home/vscode/.opam/default/bin:/opt/bazelisk:/usr/lib/llvm-${LLVM_VER}/bin:${PATH}"

ENV PYTHONPATH="/usr/lib/llvm-14/lib/python3.10/dist-packages/"

# clean
RUN apt-get autoremove -y \
  && apt-get clean -y \
  && rm -rf /var/lib/apt/lists/* \
  && rm -rf /tmp/*

# set workdir when container run
WORKDIR /workspaces
