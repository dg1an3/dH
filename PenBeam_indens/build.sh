#!/bin/bash
# Simple build script for PenBeam_indens

set -e  # Exit on error

echo "======================================"
echo "PenBeam_indens Build Script"
echo "======================================"

# Check if running in Docker
if [ -f /.dockerenv ]; then
    echo "Running inside Docker container"
else
    echo "Running on host system"
fi

# Check for gfortran
if ! command -v gfortran &> /dev/null; then
    echo "Error: gfortran not found. Please install it:"
    echo "  Ubuntu/Debian: sudo apt-get install gfortran"
    echo "  RHEL/CentOS:   sudo yum install gcc-gfortran"
    echo "  MacOS:         brew install gcc"
    exit 1
fi

echo "Fortran compiler: $(gfortran --version | head -n1)"
echo ""

# Create output directories
echo "Creating output directories..."
mkdir -p code/coni code/cono

# Source files in dependency order
SOURCES=(
    "code/convolve_input_data.for"
    "code/energy_lookup.for"
    "code/interp_energy.for"
    "code/make_vector.for"
    "code/mydensity_get.for"
    "code/newdensity_get.for"
    "code/mydiv_fluence_calc.for"
    "code/myformat_read.for"
    "code/myformat_write.for"
    "code/mymodify_calc.for"
    "code/mynew_sphere_convolve.for"
    "code/ray_trace_set_up.for"
    "code/myconvolve_main.for"
)

# Compiler flags
FFLAGS="-O2 -std=legacy -fno-automatic"

echo "Compiling Fortran sources..."
gfortran $FFLAGS -o convolve_main "${SOURCES[@]}"

if [ $? -eq 0 ]; then
    echo ""
    echo "======================================"
    echo "Build successful!"
    echo "Executable: ./convolve_main"
    echo "======================================"
    echo ""
    echo "To run: ./convolve_main < code/penbeam_input.txt"
    exit 0
else
    echo ""
    echo "Build failed!"
    exit 1
fi
