# Dockerfile for EGSnrc Monte Carlo simulation
# Builds EGSnrc for generating energy deposition kernels for radiotherapy

FROM ubuntu:22.04 AS builder

# Avoid interactive prompts during build
ENV DEBIAN_FRONTEND=noninteractive

# Install build dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    gfortran \
    g++ \
    gcc \
    make \
    tcsh \
    wget \
    git \
    libx11-dev \
    libxpm-dev \
    libxft-dev \
    libxext-dev \
    grace \
    imagemagick \
    && rm -rf /var/lib/apt/lists/*

# Set up EGSnrc environment
ENV EGS_HOME=/opt/EGSnrc
ENV EGS_CONFIG=${EGS_HOME}/HEN_HOUSE/specs
ENV HEN_HOUSE=${EGS_HOME}/HEN_HOUSE

# Create EGSnrc directory structure
RUN mkdir -p ${EGS_HOME} \
    && mkdir -p ${EGS_HOME}/egs_home \
    && mkdir -p ${EGS_HOME}/egs_home/dosxyznrc \
    && mkdir -p ${EGS_HOME}/egs_home/egs_kerma

WORKDIR ${EGS_HOME}

# Download EGSnrc
# Using a specific version for reproducibility
ARG EGSNRC_VERSION=2021
RUN wget -q https://github.com/nrc-cnrc/EGSnrc/archive/refs/tags/v${EGSNRC_VERSION}.tar.gz \
    && tar -xzf v${EGSNRC_VERSION}.tar.gz \
    && mv EGSnrc-${EGSNRC_VERSION} HEN_HOUSE \
    && rm v${EGSNRC_VERSION}.tar.gz

# Create configuration file for EGSnrc
RUN mkdir -p ${EGS_CONFIG} && \
    echo "# EGSnrc configuration" > ${EGS_CONFIG}/config.conf && \
    echo "my_machine = linux" >> ${EGS_CONFIG}/config.conf && \
    echo "canonical_system = x86_64-unknown-linux-gnu" >> ${EGS_CONFIG}/config.conf && \
    echo "# Compiler settings" >> ${EGS_CONFIG}/config.conf

# Create spec file for Linux configuration
RUN mkdir -p ${HEN_HOUSE}/specs && \
    cat > ${HEN_HOUSE}/specs/linux.conf << 'EOF'
# EGSnrc configuration for Linux
CANONICAL_SYSTEM = x86_64-unknown-linux-gnu
my_machine = linux

# Fortran compiler
F77 = gfortran
FFLAGS = -O2 -fno-automatic -fno-backslash

# C compiler
CC = gcc
CFLAGS = -O2

# C++ compiler
CXX = g++
CXXFLAGS = -O2

# Linker flags
FLIBS = -lgfortran -lm
CLIBS = -lm

# EGSnrc specific
DEFS = -DLINUX

EOF

# Set up environment script
RUN cat > ${EGS_HOME}/egsnrc_env.sh << 'EOF'
#!/bin/bash
export EGS_HOME=/opt/EGSnrc
export HEN_HOUSE=${EGS_HOME}/HEN_HOUSE
export EGS_CONFIG=${HEN_HOUSE}/specs
export my_machine=linux
export PATH=${HEN_HOUSE}/bin/${my_machine}:${PATH}
export LD_LIBRARY_PATH=${HEN_HOUSE}/lib/${my_machine}:${LD_LIBRARY_PATH}
EOF

# Make environment script executable
RUN chmod +x ${EGS_HOME}/egsnrc_env.sh

# Build EGSnrc core
WORKDIR ${HEN_HOUSE}
RUN bash -c "source ${EGS_HOME}/egsnrc_env.sh && \
    cd ${HEN_HOUSE} && \
    make || true"

# Runtime stage
FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive

# Install runtime dependencies
RUN apt-get update && apt-get install -y \
    gfortran \
    gcc \
    g++ \
    tcsh \
    bash \
    libgfortran5 \
    grace \
    imagemagick \
    bc \
    && rm -rf /var/lib/apt/lists/*

# Set up EGSnrc environment
ENV EGS_HOME=/opt/EGSnrc
ENV HEN_HOUSE=${EGS_HOME}/HEN_HOUSE
ENV EGS_CONFIG=${HEN_HOUSE}/specs
ENV my_machine=linux
ENV PATH=${HEN_HOUSE}/bin/${my_machine}:${PATH}

# Copy EGSnrc installation from builder
COPY --from=builder /opt/EGSnrc /opt/EGSnrc

# Create working directories
RUN mkdir -p /work/input \
    && mkdir -p /work/output \
    && mkdir -p /work/pegs4 \
    && mkdir -p ${EGS_HOME}/egs_home

WORKDIR /work

# Add helper scripts
COPY scripts/ /opt/scripts/
RUN chmod +x /opt/scripts/*.sh 2>/dev/null || true

# Set up environment for all users
RUN echo "source ${EGS_HOME}/egsnrc_env.sh" >> /etc/bash.bashrc

# Default command
CMD ["/bin/bash"]
