#!/bin/bash

# Quick start script for CSS Object Recognition
# This creates some test shapes and runs the demo
module_loaded() {
    module list 2>&1 | grep -q "$1"
}

[[ $(module_loaded cmake/3.26.3-xi6h36u) ]] || module load cmake/3.26.3-xi6h36u
[[ $(module_loaded eigen/3.4.0-uycckhi) ]] || module load eigen/3.4.0-uycckhi
[[ $(module_loaded zlib/1.2.13-jv5y5e7) ]] || module load zlib/1.2.13-jv5y5e7
[[ $(module_loaded zstd/1.5.5-zokfqsc) ]] || module load zstd/1.5.5-zokfqsc
[[ $(module_loaded boost/1.80.0-harukoy) ]] || module load boost/1.80.0-harukoy
[[ $(module_loaded opencv) ]] || module load opencv
echo "=== CSS Object Recognition - Quick Start ==="
echo ""

# Create test directory
mkdir -p test_shapes


# Build the project
if [ ! -f "bin/css_recognition_app" ]; then
    echo "Building project..."
    mkdir -p build
    cd build
    cmake ..
    make -j4
    cd ..
    echo ""
fi


echo "=== Quick Start Complete ==="
echo ""
echo "Next steps:"
echo "  1. Check the generated files (CSS images, GIF animations)"
echo "  2. Add your own shapes to test_shapes/"
echo "  3. Try: ./bin/./bin/css_recognition_app demo test_shapes/your_image"
echo ""
