#!/bin/bash -euvx

printf "Running 'postCreateCommand' Script ...\n"

printf "Installing Rust Targets\n"
sudo chown -R vscode ${CARGO_HOME}
sudo chown -R vscode ${RUSTUP_HOME}
# rustup update stable --no-self-update
# rustup default stable
#rustup target add wasm32-unknown-unknown
RUSTUP_DIST_SERVER=https://fastly-static.rust-lang.org rustup target add wasm32-wasip1
#rustup component add clippy
RUSTUP_DIST_SERVER=https://fastly-static.rust-lang.org rustup component add rustfmt

printf "Installing Python Dependencies\n"
pip install -r .devcontainer/requirements.txt

printf "Installing NPM Dependencies\n"

printf "Installing Ocaml stuff\n"
opam init --yes --shell-setup
eval $(opam env --switch=default)
opam install --yes dune menhir

printf "'postCreateCommand' Script Done\n"
