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

# Create simple test shapes using ImageMagick (if available)
if command -v convert &> /dev/null; then
    echo "Creating test shapes..."
    
    # Circle
    convert -size 200x200 xc:white -fill black -draw "circle 100,100 100,50" test_shapes/circle.png
    
    # Square
    convert -size 200x200 xc:white -fill black -draw "rectangle 50,50 150,150" test_shapes/square.png
    
    # Triangle
    convert -size 200x200 xc:white -fill black -draw "polygon 100,30 30,170 170,170" test_shapes/triangle.png
    
    # Star
    convert -size 200x200 xc:white -fill black -draw "polygon 100,20 120,80 180,80 130,120 150,180 100,140 50,180 70,120 20,80 80,80" test_shapes/star.png
    
    echo "Test shapes created in test_shapes/"
else
    echo "ImageMagick not found. Please create test images manually in test_shapes/"
    echo "Continuing with existing images..."
fi

echo ""

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

# Run demo if shapes exist
if [ -f "test_shapes/circle.png" ]; then
    echo "Running demo on circle..."
    ./bin/css_recognition_app demo test_shapes/circle.png
    echo ""
fi

# Build database if multiple shapes exist
SHAPE_COUNT=$(ls test_shapes/*.png 2>/dev/null | wc -l)
if [ "$SHAPE_COUNT" -ge 2 ]; then
    echo "Building shape database..."
    ./bin/css_recognition_app build test_shapes/
    echo ""
    
    echo "Testing recognition on square..."
    ./bin/css_recognition_app recognize test_shapes/square.png
    echo ""
fi

echo "=== Quick Start Complete ==="
echo ""
echo "Next steps:"
echo "  1. Check the generated files (CSS images, GIF animations)"
echo "  2. Add your own shapes to test_shapes/"
echo "  3. Try: ./bin/css_recognition_app webcam"
echo ""
