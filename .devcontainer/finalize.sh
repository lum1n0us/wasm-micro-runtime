printf "Running 'postCreateCommand' Script\n"

printf "Installing Rust Targets\n"

printf "Installing Python Dependencies\n"

printf "Installing NPM Dependencies\n"

printf "Installing Ocaml stuff\n"

opam init --yes --shell-setup
eval $(opam env --switch=default)
opam install --yes dune menhir
