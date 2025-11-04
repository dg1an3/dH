#!/bin/bash
# Build DOSXYZnrc application for dose calculations

set -e

echo "======================================"
echo "Building DOSXYZnrc"
echo "======================================"

# Source EGSnrc environment
source ${EGS_HOME}/egsnrc_env.sh

# Navigate to DOSXYZnrc directory
cd ${HEN_HOUSE}/user_codes/dosxyznrc

# Build the application
echo "Compiling DOSXYZnrc..."
make clean || true
make

echo ""
echo "======================================"
echo "DOSXYZnrc build complete"
echo "======================================"
