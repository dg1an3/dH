meta:
  id: energy_kernel
  title: Brimstone Energy Deposition Kernel
  file-extension: dat
  endian: le
  license: Proprietary

doc: |
  Energy deposition kernel lookup table for dose calculation.
  Contains pre-computed cumulative energy values for spherical
  convolution in radiotherapy dose calculation.

  Used by: CEnergyDepKernel class
  Files: 2MV_kernel.dat, 6MV_kernel.dat, 15MV_kernel.dat

seq:
  - id: header
    type: kernel_header
  - id: radial_bounds
    type: f4
    repeat: expr
    repeat-expr: header.num_radial + 1
    doc: Radial distance boundaries in mm
  - id: phi_angles
    type: phi_entry
    repeat: expr
    repeat-expr: header.num_phi

types:
  kernel_header:
    seq:
      - id: magic
        contents: 'BKRN'
        doc: Magic number identifying kernel file
      - id: version
        type: u2
        doc: File format version
      - id: energy_mv
        type: f4
        doc: Beam energy in megavolts (2, 6, or 15)
      - id: mu
        type: f4
        doc: Linear attenuation coefficient (1/cm)
      - id: num_phi
        type: u2
        doc: Number of azimuthal angles (typically 48)
      - id: num_theta
        type: u2
        doc: Number of polar angles (typically 2)
      - id: num_radial
        type: u2
        doc: Number of radial steps (typically 64)
      - id: reserved
        size: 6
        doc: Reserved for future use

  phi_entry:
    doc: Energy values for one azimuthal angle
    seq:
      - id: phi_angle
        type: f4
        doc: Azimuthal angle in radians
      - id: theta_entries
        type: theta_entry
        repeat: expr
        repeat-expr: _root.header.num_theta

  theta_entry:
    doc: Energy values for one polar angle
    seq:
      - id: theta_angle
        type: f4
        doc: Polar angle in radians
      - id: cumulative_energy
        type: f4
        repeat: expr
        repeat-expr: _root.header.num_radial
        doc: Cumulative energy deposition at each radial step
