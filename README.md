# CSS Image Generation and Object recognition
This project is dedicated to generate CSS image for any silhouette-based image. The project contains offline version and interactive version for better understanding of CSS image. 

Object Recognition is still under development. 
## Build Instructions

```bash
# Create build directory
mkdir -p build && cd build

# Configure with CMake
cmake ..

# Compile
make -j4

# The executable will be in bin/css_recognition_app
```

## Usage

### 1. Demo Offline Mode - Generate CSS Animation

Process a single image and create a GIF animation showing the smoothing process:

```bash
./bin/css_recognition_app demo path/to/image.png
```

This will generate:
- `image_css.png` - CSS image visualization
- `image_animation.gif` - Animated progression of smoothing
- `image_animation_frames/` - Individual frames

You can also run gif generation command by
```bash
python3 create_gif.py ath/to/image.png path/to/image.gif 150
```

### 2. Interactive CSS Image Demo

CSS image has 2 axis: arc length as its y axis, and Gaussian filter width as its x axis. This demo allows you tweak around Gaussian filter width to see what the smoothed image is like and where it is located in CSS image. 

To run the demo, you can use the script:
```bash
sh run_interactive.sh
```
Then run it with your input image:
```bash
./run_interactive.sh path/to/image.png
```


## Algorithm Overview

### CSS (Curvature Scale Space)

1. **Contour Extraction**: Extract boundary using Canny edge detection
2. **Gaussian Smoothing**: Apply at multiple scales σ = 0.1 to 4.0
3. **Curvature Calculation**: k = (x'y'' - y'x'') / (x'² + y'²)^(3/2)
4. **Zero-Crossing Detection**: Find inflection points where curvature changes sign
5. **CSS Image**: Plot zero-crossings as (arc length, σ) points

## Dependencies

- **OpenCV** (4.0+) - Image processing and visualization
- **Eigen3** - Linear algebra
- **OpenMP** - Parallel processing
- **Boost** - Utilities
- **ImageMagick** (optional) - GIF generation via `convert` command

For Brown Oscar User, try building the project with provided script:

```bash
sh quickstart.sh
```
## References

- Mokhtarian, F., & Mackworth, A. (1992). "A Theory of Multiscale, Curvature-Based Shape Representation for Planar Curves" IEEE TPAMI
- Mokhtarian, F., & Murase, H. (1998). "Silhouette-Based Isolated Object Recognition through Curvature Scale Space" IEEE TPAMI

## Author

Jue Han (jue_han@brown.edu)  
LEMS, Brown University  
November 2024
