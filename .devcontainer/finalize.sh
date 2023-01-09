#!/bin/bash -euvx

printf "Running 'postCreateCommand' Script ...\n"

printf "Installing Rust Targets\n"
sudo chown -R vscode ${CARGO_HOME}
sudo chown -R vscode ${RUSTUP_HOME}
rustup update stable --no-self-update
rustup default stable
rustup target add wasm32-unknown-unknown
rustup target add wasm32-wasi
rustup component add clippy
rustup component add rustfmt

printf "Installing Python Dependencies\n"
pip install -r .devcontainer/requirements.txt

printf "Installing NPM Dependencies\n"

printf "Installing Ocaml stuff\n"
opam init --yes --shell-setup
eval $(opam env --switch=default)
opam install --yes dune menhir

printf "'postCreateCommand' Script Done\n"
