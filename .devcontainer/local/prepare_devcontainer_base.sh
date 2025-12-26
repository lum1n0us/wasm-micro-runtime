#!/bin/bash

# Build a devcontainer-base image from .devcontainer/Dockerfile

if [[ .devcontainer/Dockerfile -nt .devcontainer/local/devcontainer-base-image-built ]]; then
    # if .devcontainer/Dockerfile changes, rebuild the image
    echo "Building devcontainer-base image..."

    docker build -f .devcontainer/Dockerfile -t devcontainer-base --build-arg VARIANT=debian-12 --build-arg WASI_SDK_VER=25 --build-arg WABT_VER=1.0.37 .devcontainer
    if [ $? -ne 0 ]; then
        echo "Failed to build devcontainer-base image."
        exit 1
    fi

    docker image inspect devcontainer-base --format '{{.Id}}' > .devcontainer/local/devcontainer-base-image-built
else
    echo "devcontainer-base image is up to date."
fi

echo "devcontainer-base image is ready."
