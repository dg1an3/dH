#!/bin/bash
# Generate energy deposition kernel using EGSnrc

set -e

# Parse arguments
ENERGY=${1:-6}  # Default 6 MV
PARTICLES=${2:-1000000}  # Default 1 million particles
OUTPUT_NAME=${3:-"${ENERGY}MV_kernel"}

echo "======================================"
echo "Generating Energy Deposition Kernel"
echo "======================================"
echo "Energy: ${ENERGY} MV"
echo "Particles: ${PARTICLES}"
echo "Output: ${OUTPUT_NAME}"
echo ""

# Source EGSnrc environment
source ${EGS_HOME}/egsnrc_env.sh

# Check if input file exists
INPUT_FILE="/work/input/${OUTPUT_NAME}.egsinp"
if [ ! -f "${INPUT_FILE}" ]; then
    echo "Error: Input file not found: ${INPUT_FILE}"
    echo "Please create an input file or use the template generator."
    exit 1
fi

# Run the simulation
echo "Running EGSnrc simulation..."
cd /work/output

# This is a placeholder - actual kernel generation would use
# a specific EGSnrc user code like DOSRZnrc or a custom application
echo "Note: Actual kernel generation requires a properly configured"
echo "EGSnrc user code and input file tailored for your specific needs."

echo ""
echo "======================================"
echo "Kernel generation template ready"
echo "======================================"
