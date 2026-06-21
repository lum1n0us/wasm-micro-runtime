# Copyright (C) 2019 Intel Corporation.  All rights reserved.
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

# hadolint global ignore=DL3008,DL3009

ARG VARIANT=debian-12
FROM mcr.microsoft.com/devcontainers/cpp:${VARIANT}

ARG DEBIAN_FRONTEND=noninteractive
ENV TZ=Asia/Shanghai

RUN apt-get update \
  && apt-get upgrade -y

RUN apt-get update \
  && apt-get install -y apt-transport-https apt-utils build-essential \
      ca-certificates ccache clang-format-14 curl file g++-multilib git gnupg \
      libgcc-12-dev lib32gcc-12-dev libzstd-dev lsb-release \
      ninja-build ocaml ocamlbuild opam \
      python3-venv python3-pip \
      software-properties-common tree tzdata \
      unzip valgrind vim wget zip --no-install-recommends

WORKDIR /opt

ARG WASI_SDK_VER=25
RUN wget -c --progress=dot:giga https://github.com/WebAssembly/wasi-sdk/releases/download/wasi-sdk-${WASI_SDK_VER}/wasi-sdk-${WASI_SDK_VER}.0-x86_64-linux.tar.gz -P /tmp \
  && tar xf /tmp/wasi-sdk-${WASI_SDK_VER}.0-x86_64-linux.tar.gz -C /opt \
  && ln -sf /opt/wasi-sdk-${WASI_SDK_VER}.0-x86_64-linux /opt/wasi-sdk

ARG WABT_VER=1.0.37
RUN wget -c --progress=dot:giga https://github.com/WebAssembly/wabt/releases/download/${WABT_VER}/wabt-${WABT_VER}-ubuntu-20.04.tar.gz -P /tmp \
  && tar xf /tmp/wabt-${WABT_VER}-ubuntu-20.04.tar.gz -C /opt \
  && ln -sf /opt/wabt-${WABT_VER} /opt/wabt

# set path

# Install wasm-tools
ARG WASM_TOOLS_VER=1.243.0
RUN wget -c --progress=dot:giga https://github.com/bytecodealliance/wasm-tools/releases/download/v${WASM_TOOLS_VER}/wasm-tools-${WASM_TOOLS_VER}-x86_64-linux.tar.gz -P /tmp
RUN tar xzf /tmp/wasm-tools-${WASM_TOOLS_VER}-x86_64-linux.tar.gz -C /opt\
  && ln -sf /opt/wasm-tools-${WASM_TOOLS_VER}-x86_64-linux/wasm-tools /usr/local/bin/wasm-tools

# Install GitHub CLI
RUN mkdir -p -m 755 /etc/apt/keyrings \
  && out=$(mktemp) && wget -nv -O$out https://cli.github.com/packages/githubcli-archive-keyring.gpg \
  && cat $out | tee /etc/apt/keyrings/githubcli-archive-keyring.gpg > /dev/null \
  && chmod go+r /etc/apt/keyrings/githubcli-archive-keyring.gpg \
  && mkdir -p -m 755 /etc/apt/sources.list.d \
  && echo "deb [arch=$(dpkg --print-architecture) signed-by=/etc/apt/keyrings/githubcli-archive-keyring.gpg] https://cli.github.com/packages stable main" | tee /etc/apt/sources.list.d/github-cli.list > /dev/null \
  && apt update \
  && apt install gh -y

# set proxy variables for sudoer
RUN echo 'Defaults env_keep += "http_proxy https_proxy no_proxy HTTP_PROXY HTTPS_PROXY NO_PROXY"' | sudo tee /etc/sudoers.d/proxy

# Install uv for virtual environment management
RUN curl -LsSf https://astral.sh/uv/install.sh | sh

# Install utilities for C/C++ development
RUN apt update \
  && apt install -y bear ccache clangd clang-tidy shellcheck

# set path
RUN ln -sf /usr/bin/clang-format-14 /usr/bin/clang-format

# clean up
RUN apt-get autoremove -y \
  && apt-get clean -y \
  && rm -rf /var/lib/apt/lists/* \
  && rm -rf /tmp/*
