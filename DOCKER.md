# Docker Setup for dH Radiotherapy Planning System

This document describes the Docker containerization setup for the dH radiotherapy treatment planning system components.

## Overview

The dH project includes multiple computational components that have been containerized for easier development, deployment, and reproducibility:

1. **PenBeam_indens**: Fortran-based dose convolution calculations
2. **EGSnrc**: Monte Carlo simulations for energy deposition kernel generation

## Quick Start

### Build All Services

```bash
# From project root
docker-compose build
```

### Run Specific Services

```bash
# Run PenBeam convolution
docker-compose up penbeam

# Interactive EGSnrc shell
docker-compose run --rm egsnrc

# Development containers
docker-compose run --rm penbeam-dev
docker-compose run --rm egsnrc-dev
```

## Service Descriptions

### PenBeam_indens Services

#### `penbeam`
Production service for running dose convolution calculations.

```bash
docker-compose up penbeam
```

**Volumes:**
- `./PenBeam_indens/code/coni` - Input data
- `./PenBeam_indens/code/cono` - Output data
- `./PenBeam_indens/code/penbeam_input.txt` - Configuration (read-only)

#### `penbeam-dev`
Development environment with full build tools.

```bash
docker-compose run --rm penbeam-dev
```

**Use cases:**
- Modify Fortran source code
- Rebuild with custom compiler flags
- Debug build issues

### EGSnrc Services

#### `egsnrc`
Interactive container for Monte Carlo simulations.

```bash
docker-compose run --rm egsnrc
```

**Volumes:**
- `./EGSnrc/input` - Simulation input files (read-only)
- `./EGSnrc/output` - Simulation results
- `./EGSnrc/pegs4` - Material data files
- `./Brimstone` - Output kernels to Brimstone directory

#### `egsnrc-dev`
Full development environment with EGSnrc source code.

```bash
docker-compose run --rm egsnrc-dev
```

**Use cases:**
- Compile custom EGSnrc user codes
- Modify simulation parameters
- Build new applications (DOSRZnrc, DOSXYZnrc, etc.)

### Utility Services

#### `kernel-convert`
Convert EGSnrc output to Brimstone kernel format.

```bash
docker-compose run --rm kernel-convert \
  output/6MV_kernel.3ddose \
  /brimstone_kernels/6MV_kernel.dat
```

## Common Workflows

### 1. Generate Energy Deposition Kernel

```bash
# Step 1: Start EGSnrc container
docker-compose run --rm egsnrc

# Inside container:
# Build DOSRZnrc
cd ${HEN_HOUSE}/user_codes/dosrznrc
make

# Run simulation
dosrznrc -i /work/input/6MV_kernel_template.egsinp \
         -p /work/pegs4/521icru.pegs4dat \
         -o /work/output/6MV_kernel

# Exit container
exit

# Step 2: Convert to Brimstone format
docker-compose run --rm kernel-convert \
  output/6MV_kernel.3ddose \
  /brimstone_kernels/6MV_kernel.dat
```

### 2. Run Dose Convolution

```bash
# Option 1: Using docker-compose
docker-compose up penbeam

# Option 2: Interactive session
docker-compose run --rm penbeam /bin/bash
# Inside container: ./convolve_main < code/penbeam_input.txt
```

### 3. Development Workflow

```bash
# Start development container
docker-compose run --rm penbeam-dev

# Inside container:
# Modify code, rebuild, test
vi code/myconvolve_main.for
make clean
make
./convolve_main < code/penbeam_input.txt
```

## Volume Mappings

### Shared Kernel Storage

The `Brimstone/` directory is shared between services:
- EGSnrc writes generated kernels here
- Brimstone application reads kernels from here
- Ensures consistency across the system

```
Host                          Container
====                          =========
./Brimstone/*.dat      →      /brimstone_kernels/*.dat
```

### Service-Specific Data

Each service has isolated input/output:

**PenBeam_indens:**
```
./PenBeam_indens/code/coni    →    /app/code/coni
./PenBeam_indens/code/cono    →    /app/code/cono
```

**EGSnrc:**
```
./EGSnrc/input                →    /work/input
./EGSnrc/output               →    /work/output
./EGSnrc/pegs4                →    /work/pegs4
```

## Building Individual Services

### Build PenBeam Only

```bash
cd PenBeam_indens
docker-compose build
# or
docker build -t penbeam_indens:latest .
```

### Build EGSnrc Only

```bash
cd EGSnrc
docker-compose build
# or
docker build -t egsnrc:latest .
```

## Networking

All services are on the `dh_network` bridge network by default.

Services can communicate using container names:
- `penbeam`
- `egsnrc`
- etc.

## Environment Variables

### EGSnrc

Set via command line or environment section in docker-compose.yml:

```bash
# Run with custom particle count
docker-compose run --rm \
  -e PARTICLES=10000000 \
  -e ENERGY=15 \
  egsnrc
```

### PenBeam

Configuration via input file (`penbeam_input.txt`).

## Data Persistence

All data is persisted on the host filesystem through volume mounts:

- **Input files**: Preserved in respective `input/` directories
- **Output files**: Written to `output/` directories on host
- **Kernels**: Stored in `Brimstone/` directory

No data is lost when containers are removed.

## Troubleshooting

### Build Failures

```bash
# Clean rebuild
docker-compose build --no-cache penbeam

# Check logs
docker-compose logs penbeam
```

### Permission Issues

If you encounter permission errors with mounted volumes:

```bash
# Check file ownership
ls -la PenBeam_indens/code/cono/

# Fix permissions (if needed)
sudo chown -R $USER:$USER PenBeam_indens/code/cono/
```

### Container Won't Start

```bash
# Check container status
docker-compose ps

# View detailed logs
docker-compose logs -f egsnrc

# Remove and recreate
docker-compose down
docker-compose up -d
```

### Out of Disk Space

Monte Carlo simulations can generate large output files.

```bash
# Check disk usage
du -sh EGSnrc/output/*

# Clean old outputs
rm -rf EGSnrc/output/*.egsdat
rm -rf EGSnrc/output/*.egslst
```

## Performance Optimization

### Parallel Simulations

Run multiple EGSnrc simulations in parallel:

```bash
# Launch 4 parallel jobs with different seeds
for i in {1..4}; do
    docker-compose run -d --rm \
        --name egsnrc_job_$i \
        egsnrc \
        dosrznrc -i /work/input/6MV_kernel_template.egsinp \
                 -o /work/output/6MV_kernel_run${i}
done

# Monitor
docker ps

# Combine results later using EGSnrc tools
```

### Resource Limits

Limit CPU and memory usage in docker-compose.yml:

```yaml
services:
  egsnrc:
    # ... other config ...
    deploy:
      resources:
        limits:
          cpus: '4.0'
          memory: 8G
```

## Integration with Main Project

### Brimstone (Windows MFC Application)

The Windows-based Brimstone GUI application is **not containerized** as it requires:
- Windows-specific MFC framework
- Direct GPU access for visualization
- Native Windows GUI

However, it can use kernels generated by the containerized EGSnrc:

```
1. Generate kernel in EGSnrc container
   ↓
2. Convert to Brimstone format
   ↓
3. Place in Brimstone/*.dat
   ↓
4. Use in native Windows Brimstone application
```

### RtModel (C++ Library)

The core RtModel C++ library is also Windows/Visual Studio based and not containerized. However:

- Dose calculation algorithms can be tested independently
- Python utilities for CT processing can be containerized (future work)

## Future Enhancements

Potential additions to the Docker setup:

1. **Python utilities container**: For ITK-based CT processing
2. **Testing container**: Automated unit tests for algorithms
3. **CI/CD integration**: Automated builds and tests
4. **GPU acceleration**: CUDA-enabled EGSnrc builds
5. **Web interface**: Browser-based kernel generation tool

## Additional Resources

- **PenBeam_indens README**: `PenBeam_indens/README.md`
- **EGSnrc README**: `EGSnrc/README.md`
- **Project Documentation**: `CLAUDE.md`
- **Repository Structure**: `repository_structure.md`

## License

See individual component licenses:
- PenBeam_indens: Copyright Rock Mackie, University of Wisconsin
- EGSnrc: NRC Canada license
- Brimstone/dH: Copyright Derek G. Lane (U.S. Patent 7,369,645)
