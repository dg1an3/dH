#!/bin/bash
# Build egs_kerma application for kerma calculations

set -e

echo "======================================"
echo "Building egs_kerma"
echo "======================================"

# Source EGSnrc environment
source ${EGS_HOME}/egsnrc_env.sh

# Navigate to egs_kerma directory
cd ${HEN_HOUSE}/user_codes/egs_kerma

# Build the application
echo "Compiling egs_kerma..."
make clean || true
make

echo ""
echo "======================================"
echo "egs_kerma build complete"
echo "======================================"
