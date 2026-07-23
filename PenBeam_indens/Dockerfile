# Dockerfile for building PenBeam_indens Fortran code
# Multi-stage build for smaller final image

FROM gcc:11 AS builder

# Install build dependencies
RUN apt-get update && apt-get install -y \
    gfortran \
    make \
    && rm -rf /var/lib/apt/lists/*

# Set working directory
WORKDIR /build

# Copy Makefile and source code
COPY Makefile .
COPY code/ ./code/

# Build using Makefile
RUN make all

# Runtime stage (smaller image)
FROM debian:bullseye-slim

# Install runtime dependencies
RUN apt-get update && apt-get install -y \
    libgfortran5 \
    && rm -rf /var/lib/apt/lists/*

# Set working directory
WORKDIR /app

# Copy compiled executable from builder stage
COPY --from=builder /build/convolve_main .
COPY --from=builder /build/code/coni ./code/coni
COPY --from=builder /build/code/cono ./code/cono
COPY code/penbeam_input.txt ./code/

# Set up environment
ENV PATH="/app:${PATH}"

# Default command - runs the program with input file
CMD ["sh", "-c", "./convolve_main < code/penbeam_input.txt"]
