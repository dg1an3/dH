# EGSnrc Input Files

This directory contains input file templates for generating energy deposition kernels.

## File Format

EGSnrc uses a structured input format with sections delimited by `:start` and `:stop` tags.

## Available Templates

### 6MV_kernel_template.egsinp
Template for generating a 6 MV photon energy deposition kernel in water.

Key parameters:
- **Energy**: 6.0 MeV (typical for 6 MV linear accelerator)
- **Medium**: H2O700ICRU (water)
- **Geometry**: Cylindrical (R-Z) with 64 radial and 128 axial bins
- **Grid spacing**: 0.5 cm
- **ECUT**: 0.521 MeV (electron cutoff energy)
- **PCUT**: 0.010 MeV (photon cutoff energy)

## Customizing Input Files

To create a kernel for different energy:

1. Copy the template:
   ```bash
   cp 6MV_kernel_template.egsinp 15MV_kernel_template.egsinp
   ```

2. Modify key parameters:
   - **Energy**: Change photon energy (e.g., 15.0 MeV for 15 MV)
   - **ECUT/PCUT**: Adjust cutoff energies if needed
   - **ncase**: Increase for better statistics (recommended: 10^7 - 10^9)

3. For different geometries:
   - Adjust `NRCYL` (number of radial bins)
   - Modify radial boundaries in `:start radii:` section
   - Adjust `NPLANE` (number of axial planes)
   - Modify plane positions in `:start planes:` section

## Common Beam Energies

Medical linear accelerators typically use:
- **6 MV**: Most common, good for shallow tumors
- **15 MV**: Higher penetration, for deep-seated tumors
- **18 MV**: Deep tumors, skin sparing
- **10 MV**: Balance between 6 and 15 MV

For electron beams:
- 4-20 MeV range
- Requires different ECUT/PCUT settings

## PEGS4 Data Files

Material definitions require PEGS4 data files. Common materials:

- **H2O700ICRU**: Water at 700 mg/cm³
- **AIR700ICRU**: Air
- **TISSUE_ICRP**: ICRP tissue compositions
- **LUNG700ICRU**: Lung tissue

These should be available in `/work/pegs4/` or generated using the `pegs4` utility.

## Running Simulations

```bash
# Inside the EGSnrc Docker container
source /opt/EGSnrc/egsnrc_env.sh

# Run DOSRZnrc with input file
dosrznrc -i input/6MV_kernel_template.egsinp -o output/6MV_kernel
```

## Output Files

Simulation produces:
- **`.egslst`**: Detailed text listing of results
- **`.3ddose`**: 3D dose distribution (if enabled)
- **`.egsdat`**: Binary data file with detailed results

## Converting to Brimstone Format

The output needs to be converted to the binary `.dat` format used by Brimstone.
A conversion script is provided in `/opt/scripts/convert_kernel.sh`.

## References

- EGSnrc Manual: https://nrc-cnrc.github.io/EGSnrc/
- DOSRZnrc User Manual: Part of EGSnrc documentation
- PEGS4 Manual: For material data generation
