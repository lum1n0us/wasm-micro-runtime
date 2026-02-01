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

echo "Local finalize script completed. ✅"
