# PenBeam_indens - Fortran Dose Convolution Code

This directory contains the PenBeam_indens Fortran code for dose convolution calculations in radiotherapy treatment planning.

## Overview

PenBeam_indens calculates the amount of energy deposited in a water-like phantom using superposition/convolution methods. The code was originally developed by Rock Mackie at the University of Wisconsin Department of Medical Physics.

## Building and Running

### Option 1: Docker (Recommended)

#### Using Docker Compose (Easiest)

```bash
# Build and run
docker-compose up penbeam

# Run with custom input/output directories mounted
docker-compose up penbeam

# Interactive development container
docker-compose run --rm penbeam-dev
```

#### Using Docker CLI

```bash
# Build the Docker image
docker build -t penbeam_indens:latest .

# Run the container
docker run --rm -it \
  -v $(pwd)/code/coni:/app/code/coni \
  -v $(pwd)/code/cono:/app/code/cono \
  penbeam_indens:latest

# Or run interactively
docker run --rm -it penbeam_indens:latest /bin/bash
```

### Option 2: Local Build (Requires gfortran)

#### Prerequisites
- GNU Fortran compiler (`gfortran`)
- Make

On Ubuntu/Debian:
```bash
sudo apt-get install gfortran make
```

#### Build and Run

```bash
# Build the executable
make

# Run the program
make run

# Or run manually
./convolve_main < code/penbeam_input.txt

# Clean build artifacts
make clean

# Rebuild from scratch
make rebuild
```

## Project Structure

```
PenBeam_indens/
├── Dockerfile              # Docker build configuration
├── docker-compose.yml      # Docker Compose configuration
├── Makefile               # Build configuration
├── README.md              # This file
└── code/
    ├── *.for              # Fortran source files
    ├── penbeam_input.txt  # Input configuration file
    ├── coni/              # Input data directory
    └── cono/              # Output data directory
```

## Source Files

- `myconvolve_main.for` - Main program
- `convolve_input_data.for` - Input data handling
- `mydiv_fluence_calc.for` - Diverging fluence calculation
- `mynew_sphere_convolve.for` - Spherical convolution routine
- `mydensity_get.for` - Density matrix setup
- `mymodify_calc.for` - Beam modifier calculations
- `ray_trace_set_up.for` - Ray tracing geometry setup
- `myformat_write.for` / `myformat_read.for` - I/O formatting
- `energy_lookup.for` / `interp_energy.for` - Energy interpolation
- `make_vector.for` - Vector utilities

## Input/Output

### Input
The program reads configuration from `code/penbeam_input.txt` which specifies:
- Beam energy
- Phantom dimensions and voxel sizes
- Source-to-surface distance (SSD)
- Field boundaries
- Region of interest

### Output
Results are written to:
- `code/cono/format_dose.dat` - Dose distribution
- `code/cono/format_fluence.dat` - Fluence distribution
- Console output with calculation progress

## Docker Details

The Dockerfile uses a multi-stage build:

1. **Builder stage**: Compiles the Fortran code using GCC 11 with gfortran
2. **Runtime stage**: Creates a minimal Debian-based image with only the executable and runtime libraries

This approach results in a smaller final image (~200MB vs ~1GB).

### Volume Mounts

Mount these directories to access input/output:
- `./code/coni` - Input data directory
- `./code/cono` - Output data directory
- `./code/penbeam_input.txt` - Configuration file (read-only)

## Development

To modify and rebuild:

```bash
# Edit source files in code/*.for
# Then rebuild using Docker
docker-compose build

# Or rebuild locally
make rebuild
```

For interactive development:
```bash
docker-compose run --rm penbeam-dev
```

This gives you a shell in the builder environment where you can compile and test changes.

## License

Copyright (c) 1985-1989 Rock Mackie, Department of Medical Physics, University of Wisconsin, Madison, WI

See original source files for copyright notices.

## Notes

- Original code was designed for Watcom Fortran compiler on Windows
- This Docker setup uses gfortran (GNU Fortran) which is compatible with Fortran 77
- The code uses fixed-form Fortran format (.for extension)
- Array indices use non-standard ranges (e.g., -63:63)
