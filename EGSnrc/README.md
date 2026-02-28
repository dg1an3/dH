# EGSnrc Docker Environment for Energy Deposition Kernel Generation

This directory contains a Docker-based setup for running **EGSnrc** (Electron Gamma Shower - National Research Council of Canada), a Monte Carlo simulation package for modeling radiation transport in radiotherapy applications.

## Purpose

Generate energy deposition kernels for different beam energies (6 MV, 15 MV, etc.) used by the Brimstone radiotherapy treatment planning system. These kernels are essential for accurate dose calculations using the pencil beam convolution algorithm.

## What is EGSnrc?

EGSnrc is a software toolkit for Monte Carlo simulation of ionizing radiation transport through matter. It's widely used in medical physics for:
- Dose calculations in radiotherapy
- Radiation dosimetry
- Detector response modeling
- Generating energy deposition kernels

## Directory Structure

```
EGSnrc/
├── Dockerfile              # Docker build configuration
├── docker-compose.yml      # Container orchestration
├── README.md              # This file
├── .dockerignore          # Docker build optimization
├── scripts/               # Helper scripts
│   ├── build_dosxyznrc.sh        # Build DOSXYZnrc application
│   ├── build_egs_kerma.sh        # Build egs_kerma application
│   ├── generate_kernel.sh        # Generate kernel wrapper
│   ├── setup_pegs4.sh            # Setup material data
│   └── convert_kernel.sh         # Convert to Brimstone format
├── input/                 # Input file templates
│   ├── 6MV_kernel_template.egsinp
│   └── README_inputs.md
├── output/                # Simulation results (created)
└── pegs4/                 # Material data files (created)
```

## Quick Start

### 1. Build the Docker Image

```bash
cd EGSnrc
docker-compose build
```

### 2. Start Interactive Container

```bash
# Start the container with interactive shell
docker-compose run --rm egsnrc

# Inside the container, you'll have access to:
# - EGSnrc environment (already sourced)
# - Input templates in /work/input/
# - Output directory at /work/output/
# - Helper scripts in /opt/scripts/
```

### 3. Generate a Kernel (Example Workflow)

```bash
# Inside the container
source /opt/EGSnrc/egsnrc_env.sh

# Step 1: Setup PEGS4 material data
/opt/scripts/setup_pegs4.sh

# Step 2: Build required EGSnrc application (DOSRZnrc typically)
cd ${HEN_HOUSE}/user_codes/dosrznrc
make

# Step 3: Run simulation with your input file
dosrznrc -i /work/input/6MV_kernel_template.egsinp \
         -p /work/pegs4/521icru.pegs4dat \
         -o /work/output/6MV_kernel

# Step 4: Convert output to Brimstone format (outside container)
exit
docker-compose run --rm egsnrc-convert \
  /opt/scripts/convert_kernel.sh \
  output/6MV_kernel.3ddose \
  /brimstone_kernels/6MV_kernel.dat
```

## Docker Services

### `egsnrc` (Main Service)
Interactive container for running simulations.

```bash
docker-compose run --rm egsnrc
```

### `egsnrc-dev` (Development)
Full build environment with access to source code.

```bash
docker-compose run --rm egsnrc-dev
```

### `egsnrc-kernel` (Batch Job)
For automated kernel generation.

```bash
docker-compose run --rm -e ENERGY=15 -e PARTICLES=10000000 egsnrc-kernel
```

### `egsnrc-convert` (Converter)
Convert EGSnrc output to Brimstone kernel format.

```bash
docker-compose run --rm egsnrc-convert /opt/scripts/convert_kernel.sh \
  output/kernel.3ddose /brimstone_kernels/kernel.dat
```

## Common EGSnrc Applications

### DOSRZnrc
Most common for generating energy deposition kernels.
- Cylindrical (R-Z) geometry
- Efficient for symmetric problems
- Best for kernel calculations

Build and run:
```bash
cd ${HEN_HOUSE}/user_codes/dosrznrc
make
dosrznrc -i <input.egsinp> -p <pegs4.dat> -o <output>
```

### DOSXYZnrc
3D Cartesian geometry dose calculations.

Build and run:
```bash
/opt/scripts/build_dosxyznrc.sh
dosxyznrc -i <input.egsinp> -p <pegs4.dat> -o <output>
```

### egs_kerma
Kerma calculations for dosimetry.

Build and run:
```bash
/opt/scripts/build_egs_kerma.sh
egs_kerma -i <input.egsinp> -p <pegs4.dat>
```

## Energy Deposition Kernel Generation

### Typical Workflow

1. **Define Geometry**
   - Cylindrical phantom (R-Z coordinates)
   - Water medium (H2O700ICRU)
   - Grid: 64 radial bins × 128 axial bins
   - Spacing: 0.5 cm typical

2. **Set Source Parameters**
   - Monoenergetic photon beam
   - Energies: 2, 6, 15, 18 MV
   - Point source at 100 cm SSD

3. **Configure Physics**
   - ECUT: 0.521 MeV (electron cutoff)
   - PCUT: 0.010 MeV (photon cutoff)
   - Enable Rayleigh scattering
   - Enable atomic relaxations

4. **Run Simulation**
   - Particles: 10^7 to 10^9 for good statistics
   - Batch processing for uncertainty analysis
   - Typical runtime: hours to days

5. **Post-Process**
   - Extract dose distribution
   - Convert coordinates (cylindrical → spherical)
   - Normalize and scale
   - Write Brimstone binary format

## Input File Templates

Templates are provided in `input/`:

- **6MV_kernel_template.egsinp**: 6 MV photon kernel
- Customize for other energies (2, 15, 18 MV)

Key sections to modify:
```
:start spectrum:
    Energy = 6.0  # Change to desired MV
:stop spectrum:

:start run control:
    ncase = 1000000  # Increase for production runs
:stop run control:
```

## PEGS4 Material Data

EGSnrc requires material cross-section data generated by PEGS4.

### Standard Materials
- **H2O700ICRU**: Water (dose calculations)
- **AIR700ICRU**: Air
- **TISSUE_ICRP**: Various tissue types
- **LUNG700ICRU**: Lung tissue

### Generating Material Data

```bash
# Inside container
cd /work/pegs4

# Create PEGS4 input file (example for water)
cat > water.pegs4inp << EOF
MEDIA=H2O700ICRU
RHO=1.0
...
EOF

# Generate material data
pegs4 -i water.pegs4inp -o 521icru.pegs4dat
```

Pre-generated files are typically available in `${HEN_HOUSE}/pegs4/data/`.

## Converting to Brimstone Format

The generated kernels must be converted to Brimstone's binary format.

### Current Status
A template conversion script is provided in `scripts/convert_kernel.sh`. This needs customization for:

1. **Binary Format**: Match `EnergyDepKernel::LoadKernel()` expectations
2. **Coordinate Transform**: Cylindrical (R-Z) → Spherical (θ, φ, r)
3. **Data Interpolation**: Resample to Brimstone's grid
4. **Normalization**: Energy scaling and units

### Customization Required

Examine Brimstone kernel reader:
```cpp
// See: RtModel/EnergyDepKernel.cpp
void CEnergyDepKernel::LoadKernel()
{
    // Understand binary format structure
    // Typically: header + angular bins + radial data
}
```

## Performance Considerations

### Simulation Time
- **Quick test**: 10^6 particles (~minutes)
- **Production**: 10^9 particles (hours to days)
- **Variance reduction**: Use EGSnrc techniques to improve efficiency

### Computational Resources
- CPU-intensive (no GPU acceleration in standard EGSnrc)
- Parallel runs: Use multiple containers with different random seeds
- Combine results using EGSnrc utilities

### Parallel Execution Example

```bash
# Run 10 parallel simulations with different seeds
for i in {1..10}; do
    docker-compose run -d --rm \
        -e SEED1=$((97 + $i)) \
        -e SEED2=$((33 + $i*2)) \
        egsnrc-kernel \
        dosrznrc -i /work/input/6MV_kernel_template.egsinp \
                 -o /work/output/6MV_kernel_run${i}
done
```

## Verification and Validation

### Check Output Quality

1. **Statistical Uncertainty**
   - Check `.egslst` file for uncertainties
   - Should be < 2% for clinical use
   - Increase particles if needed

2. **Visual Inspection**
   - Plot depth-dose curves
   - Verify physical behavior (buildup, exponential falloff)
   - Compare with published data

3. **Benchmarking**
   - Compare with measured data
   - Use standard phantoms
   - Validate against literature

## Troubleshooting

### Build Failures
```bash
# Check EGSnrc environment
source /opt/EGSnrc/egsnrc_env.sh
echo $HEN_HOUSE
echo $my_machine

# Verify compiler availability
gfortran --version
gcc --version
```

### Runtime Errors
```bash
# Check PEGS4 data path
ls -la /work/pegs4/

# Verify input file syntax
# Look for missing :stop tags or typos
cat /work/input/6MV_kernel_template.egsinp
```

### Slow Performance
- Reduce particle count for testing
- Optimize cutoff energies (ECUT/PCUT)
- Use variance reduction techniques
- Run parallel jobs

## Integration with Brimstone

Generated kernels are used by:
- `RtModel/EnergyDepKernel.cpp` - Kernel loading and lookup
- `RtModel/SphereConvolve.cpp` - Spherical convolution
- `RtModel/BeamDoseCalc.cpp` - Dose calculation

Kernels stored in:
```
Brimstone/
├── 6MV_kernel.dat
├── 15MV_kernel.dat
└── 2MV_kernel.dat
```

## Additional Resources

### EGSnrc Documentation
- Homepage: https://nrc-cnrc.github.io/EGSnrc/
- GitHub: https://github.com/nrc-cnrc/EGSnrc
- Manual: Included in distribution

### Medical Physics References
- Ahnesjö A, Aspradakis MM. Dose calculations for external photon beams in radiotherapy. Phys Med Biol. 1999
- Rogers DWO. Fifty years of Monte Carlo simulations for medical physics. Phys Med Biol. 2006

### Support
- EGSnrc Users Group: https://github.com/nrc-cnrc/EGSnrc/discussions
- Medical Physics Stack Exchange

## License

EGSnrc is distributed under NRC License. See EGSnrc documentation for details.

Brimstone integration: Copyright (c) 2007-2021, Derek G. Lane

## Contributors

Energy deposition kernel methodology:
- Rock Mackie (University of Wisconsin)
- T. Rock Mackie, et al. - Photon beam dose computations (1985-1989)

EGSnrc:
- National Research Council of Canada
- Ionizing Radiation Standards Group
