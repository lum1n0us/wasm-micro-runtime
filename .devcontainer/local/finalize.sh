echo "Running local finalize script..."

#
# Python Package Installation
#
# Install required packages
pip3 install --no-cache-dir --break-system-packages -r .devcontainer/requirements.txt

echo "Local finalize script completed. ✅"
