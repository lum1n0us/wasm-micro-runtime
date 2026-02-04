set -o errexit
set -o pipefail
set -o nounset

echo "Running local finalize script..."

#
# Python Package Installation
#
# Install required packages
pip3 install --no-cache-dir --break-system-packages -r .devcontainer/requirements.txt

# Install opencode
curl -fsSL https://opencode.ai/install | bash

echo "You might want to copy your Opencode auth.json to ~/.local/share/opencode/auth.json"
mkdir -p ~/.local/share/opencode

echo "Local finalize script completed. ✅"
