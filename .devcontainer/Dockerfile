# Copyright (C) 2019 Intel Corporation.  All rights reserved.
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

# See here for image contents: https://github.com/microsoft/vscode-dev-containers/tree/v0.245.2/containers/cpp/.devcontainer/base.Dockerfile
# [Choice] Debian / Ubuntu version (use Debian 12/11/9, Ubuntu 18.04/21.04 on local arm64/Apple Silicon): debian-12, debian-11, debian-10, debian-9, ubuntu-21.04, ubuntu-20.04, ubuntu-18.04
ARG VARIANT=ubuntu-22.04
FROM mcr.microsoft.com/devcontainers/cpp:1-${VARIANT}

ARG DEBIAN_FRONTEND=noninteractive
ENV TZ=Asian/Shanghai

# hadolint ignore=DL3008
RUN apt-get update \
  && apt-get upgrade -y \
  && apt-get install -y apt-transport-https apt-utils build-essential \
  ca-certificates ccache clang curl g++-multilib git gnupg \
  libgcc-12-dev lib32gcc-12-dev llvm lsb-release \
  ninja-build ocaml ocamlbuild \
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
ARG WASI_SDK_VER=20
RUN wget -c --progress=dot:giga https://github.com/WebAssembly/wasi-sdk/releases/download/wasi-sdk-${WASI_SDK_VER}/wasi-sdk-${WASI_SDK_VER}.0-linux.tar.gz -P /opt \
  && tar xf /opt/wasi-sdk-${WASI_SDK_VER}.0-linux.tar.gz -C /opt \
  && ln -sf /opt/wasi-sdk-${WASI_SDK_VER}.0 /opt/wasi-sdk \
  && rm /opt/wasi-sdk-${WASI_SDK_VER}.0-linux.tar.gz

#
#install wabt
ARG WABT_VER=1.0.33
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
# Install required python packages
# hadolint ignore=DL3013
RUN pip3 install --no-cache-dir black jupyter nose numpy pycparser pylint

#
# Install github-cli. It doens't work as a feature of devcontainer.json
ARG GH_CLI_VER=2.32.0
WORKDIR /tmp
RUN wget -q https://github.com/cli/cli/releases/download/v${GH_CLI_VER}/gh_${GH_CLI_VER}_linux_amd64.deb \
  && dpkg -i gh_${GH_CLI_VER}_linux_amd64.deb

#
# Install NodeJS
RUN wget -qO- https://deb.nodesource.com/setup_20.x | bash -
# hadolint ignore=DL3008
RUN apt-get install -y nodejs --no-install-recommends

#
# SGX setup
RUN mkdir -p /opt/intel

WORKDIR /opt/intel
RUN wget -q https://download.01.org/intel-sgx/sgx-linux/2.21/distro/ubuntu22.04-server/sgx_linux_x64_sdk_2.21.100.1.bin \
  && chmod +x sgx_linux_x64_sdk_2.21.100.1.bin
RUN echo 'yes' | ./sgx_linux_x64_sdk_2.21.100.1.bin
RUN source /opt/intel/sgxsdk/environment

# RUN echo 'deb [signed-by=/etc/apt/keyrings/intel-sgx-keyring.asc arch=amd64] https://download.01.org/intel-sgx/sgx_repo/ubuntu jammy main' | sudo tee /etc/apt/sources.list.d/intel-sgx.list \
#   && wget https://download.01.org/intel-sgx/sgx_repo/ubuntu/intel-sgx-deb.key \
#   && cat intel-sgx-deb.key | sudo tee /etc/apt/keyrings/intel-sgx-keyring.asc
# RUN sudo apt update \
#   && sudo apt install -y libsgx-launch libsgx-urts libsgx-enclave-common


#
# Install Rust
# WORKDIR /tmp
# ENV RUSTUP_HOME=/usr/local/cargo
# ENV CARGO_HOME=/usr/local/cargo
# RUN curl --proto '=https' --tlsv1.2 https://sh.rustup.rs -sSf -o install_rust.sh
# RUN chmod a+x install_rust.sh \
#   && ./install_rust.sh -v -y
# ENV PATH="/usr/local/cargo/bin:${PATH}"

#
# Install wasm-tools from binary releasing
ARG WASM_TOOLS_VER=1.0.38
WORKDIR /opt
RUN wget -c --progress=dot:giga https://github.com/bytecodealliance/wasm-tools/releases/download/wasm-tools-${WASM_TOOLS_VER}/wasm-tools-${WASM_TOOLS_VER}-x86_64-linux.tar.gz
RUN tar xzf wasm-tools-${WASM_TOOLS_VER}-x86_64-linux.tar.gz \
  && rm wasm-tools-${WASM_TOOLS_VER}-x86_64-linux.tar.gz \
  && ln -sf /opt/wasm-tools-${WASM_TOOLS_VER}-x86_64-linux/wasm-tools /usr/bin/wasm-tools

#
# Install wit-bindgen
RUN cargo install --git https://github.com/bytecodealliance/wit-bindgen wit-bindgen-cli

#
# Install perf
RUN apt update \
      && apt-get install -y linux-tools-generic linux-tools-$(uname -a | awk '{print $3}') \

# set path
ENV PATH="/opt/bazelisk:/usr/lib/llvm-${LLVM_VER}/bin:${PATH}"
# ENV CC=/usr/lib/llvm-${LLVM_VER}/bin/clang CXX=/usr/lib/llvm-${LLVM_VER}/bin/clang++
RUN printf "%s\n" "PS1='\n[ \u@wamr-dev-docker \W ]\n$ '" >> /root/.bashrc \
  && apt-get autoremove -y \
  && apt-get clean -y \
  && rm -rf /var/lib/apt/lists/* \
  && rm -rf /tmp/*

# set workdir when container run
WORKDIR /workspaces
