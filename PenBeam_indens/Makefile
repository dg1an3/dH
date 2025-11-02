# Makefile for PenBeam_indens Fortran code
# This can be used both locally and in Docker

# Compiler settings
FC = gfortran
FFLAGS = -O2 -std=legacy -fno-automatic
LDFLAGS =

# Target executable
TARGET = convolve_main

# Source files
SOURCES = myconvolve_main.for \
          convolve_input_data.for \
          energy_lookup.for \
          interp_energy.for \
          make_vector.for \
          mydensity_get.for \
          mydiv_fluence_calc.for \
          myformat_read.for \
          myformat_write.for \
          mymodify_calc.for \
          mynew_sphere_convolve.for \
          ray_trace_set_up.for \
          newdensity_get.for

# Object files (replace .for with .o)
OBJECTS = $(SOURCES:.for=.o)

# Default target
all: setup $(TARGET)

# Create necessary directories
setup:
	@mkdir -p code/coni code/cono

# Link the executable
$(TARGET): $(OBJECTS)
	$(FC) $(LDFLAGS) -o $@ $^

# Compile .for files to .o files
%.o: code/%.for
	$(FC) $(FFLAGS) -c $< -o $@

# Clean build artifacts
clean:
	rm -f $(OBJECTS) $(TARGET)
	rm -rf code/cono/* code/coni/*

# Clean and rebuild
rebuild: clean all

# Run the program
run: $(TARGET)
	./$(TARGET) < code/penbeam_input.txt

# Docker targets
docker-build:
	docker build -t penbeam_indens:latest .

docker-run:
	docker run --rm -it penbeam_indens:latest

.PHONY: all setup clean rebuild run docker-build docker-run
