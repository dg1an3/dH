#!/bin/bash
# Setup PEGS4 data files for different materials

set -e

echo "======================================"
echo "Setting up PEGS4 data files"
echo "======================================"

# Source EGSnrc environment
source ${EGS_HOME}/egsnrc_env.sh

# PEGS4 is used to generate material data files
# Common materials for radiotherapy:
# - Water (H2O)
# - Air
# - Tissue (various densities)
# - Bone

PEGS4_DIR="/work/pegs4"
mkdir -p ${PEGS4_DIR}

echo ""
echo "PEGS4 data directory: ${PEGS4_DIR}"
echo ""
echo "To generate material data files, you need to:"
echo "1. Create a PEGS4 input file (.pegs4inp)"
echo "2. Run: pegs4 -i <input_file> -o <output_file>"
echo ""
echo "Common materials for radiotherapy:"
echo "  - H2O (water)"
echo "  - AIR700ICRU"
echo "  - TISSUE_ICRP"
echo ""

# Check if standard pegs4 data exists in HEN_HOUSE
if [ -d "${HEN_HOUSE}/pegs4/data" ]; then
    echo "Standard PEGS4 data found at: ${HEN_HOUSE}/pegs4/data"
    echo "Copying to work directory..."
    cp -r ${HEN_HOUSE}/pegs4/data/* ${PEGS4_DIR}/ 2>/dev/null || true
fi

echo ""
echo "======================================"
echo "PEGS4 setup complete"
echo "======================================"
