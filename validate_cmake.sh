#!/bin/bash
# CMake Build Validation Script
# This script validates the CMake build configuration for the dH repository

echo "====================================="
echo "CMake Build Validation Script"
echo "====================================="
echo ""

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Function to print status
print_status() {
    if [ $1 -eq 0 ]; then
        echo -e "${GREEN}✓${NC} $2"
    else
        echo -e "${RED}✗${NC} $2"
    fi
}

# Function to print info
print_info() {
    echo -e "${YELLOW}ℹ${NC} $1"
}

echo "1. Checking prerequisites..."
echo "-------------------------------------"

# Check CMake
if command -v cmake &> /dev/null; then
    CMAKE_VERSION=$(cmake --version | head -1)
    print_status 0 "CMake found: $CMAKE_VERSION"
else
    print_status 1 "CMake not found"
    exit 1
fi

echo ""
echo "2. Testing minimal CMake configuration (no dependencies)..."
echo "-------------------------------------"

# Create build directory
BUILD_DIR="build_validation"
rm -rf "$BUILD_DIR"
mkdir -p "$BUILD_DIR"

# Test 1: Configure without any optional components
cd "$BUILD_DIR"
cmake .. \
    -DBUILD_BRIMSTONE_APP=OFF \
    -DBUILD_FOUNDATION_LIBS=OFF \
    -DBUILD_COMPONENT_LIBS=OFF \
    > /dev/null 2>&1

if [ $? -eq 0 ]; then
    print_status 0 "CMake configuration successful (minimal build)"
else
    print_status 1 "CMake configuration failed (minimal build)"
    cd ..
    exit 1
fi

# Test 2: Generate build files
cmake --build . > /dev/null 2>&1
if [ $? -eq 0 ]; then
    print_status 0 "Build file generation successful"
else
    print_status 1 "Build file generation failed"
fi

cd ..

echo ""
echo "3. Checking CMake file structure..."
echo "-------------------------------------"

# Check for required CMake files
files=(
    "CMakeLists.txt"
    "RtModel/CMakeLists.txt"
    "Graph/CMakeLists.txt"
    "Brimstone/CMakeLists.txt"
    "cmake/BrimstoneConfig.cmake.in"
)

for file in "${files[@]}"; do
    if [ -f "$file" ]; then
        print_status 0 "$file exists"
    else
        print_status 1 "$file missing"
    fi
done

echo ""
echo "4. Validating CMake syntax..."
echo "-------------------------------------"

# Parse CMakeLists.txt for common issues
for cmake_file in "${files[@]%.*}.txt"; do
    if [ -f "$cmake_file" ]; then
        # Check for proper project() command
        if grep -q "^project(" "$cmake_file"; then
            print_status 0 "$cmake_file has project() command"
        else
            print_status 1 "$cmake_file missing project() command"
        fi
    fi
done

echo ""
echo "5. Summary"
echo "-------------------------------------"
print_info "Basic CMake configuration validation complete"
echo ""
print_info "To build on Windows with ITK and MFC:"
echo "  1. Install ITK and build it"
echo "  2. Install Visual Studio with MFC support"
echo "  3. Run: cmake .. -DITK_DIR=<path-to-ITK-build>"
echo "  4. Run: cmake --build . --config Release"
echo ""
print_info "To build only libraries (no GUI):"
echo "  cmake .. -DBUILD_BRIMSTONE_APP=OFF -DITK_DIR=<path-to-ITK-build>"
echo ""

# Cleanup
rm -rf "$BUILD_DIR"

echo "====================================="
echo "Validation Complete"
echo "====================================="
