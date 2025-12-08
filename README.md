# CSS Object Recognition System

A Curvature Scale Space (CSS) based object recognition system implementing the Mokhtarian-Mackworth algorithm for shape analysis and matching.

## Features

- **Edge Detection & Contour Extraction**: Automatic boundary detection using OpenCV
- **Multi-scale Gaussian Smoothing**: Progressive curve simplification at different scales
- **Curvature Computation**: Calculate curvature at each point on smoothed contours
- **CSS Image Generation**: Create 2D fingerprints showing zero-crossings across scales
- **Shape Matching**: Compare shapes using CSS features and TOED distance
- **Visualization**: Generate animated GIFs showing smoothing progression
- **Live Recognition**: Real-time shape recognition from webcam

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

### 1. Demo Mode - Generate CSS Animation

Process a single image and create a GIF animation showing the smoothing process:

```bash
./bin/css_recognition_app demo path/to/image.png
```

This will generate:
- `image_css.png` - CSS image visualization
- `image_animation.gif` - Animated progression of smoothing
- `image_animation_frames/` - Individual frames

### 2. Build Database Mode

Create a shape database from a directory of images:

```bash
./bin/css_recognition_app build database/shapes/
```

This will:
- Process all images in the directory
- Extract contours and compute CSS for each
- Save to `shape_database.dat`

### 3. Recognition Mode

Recognize a shape by comparing against the database:

```bash
./bin/css_recognition_app recognize query.png
```

Output:
- Top 5 matching shapes with distance scores
- Visualization showing query and matches
- Saved to `query_matches.png`

### 4. Webcam Mode - Live Recognition

Real-time shape recognition from webcam:

```bash
./bin/css_recognition_app webcam
```

Controls:
- **SPACE** - Capture current frame and recognize
- **Q** or **ESC** - Quit

## Algorithm Overview

### CSS (Curvature Scale Space)

1. **Contour Extraction**: Extract boundary using Canny edge detection
2. **Gaussian Smoothing**: Apply at multiple scales σ = 0.1 to 4.0
3. **Curvature Calculation**: k = (x'y'' - y'x'') / (x'² + y'²)^(3/2)
4. **Zero-Crossing Detection**: Find inflection points where curvature changes sign
5. **CSS Image**: Plot zero-crossings as (arc length, σ) points

### Shape Matching

- Uses TOED (Time-Ordered Edit Distance) to compare CSS representations
- Handles rotation, scale, and partial occlusion
- Returns ranked matches with distance scores

## Dependencies

- **OpenCV** (4.0+) - Image processing and visualization
- **Eigen3** - Linear algebra
- **OpenMP** - Parallel processing
- **Boost** - Utilities
- **ImageMagick** (optional) - GIF generation via `convert` command

## References

- Mokhtarian, F., & Mackworth, A. (1992). "A Theory of Multiscale, Curvature-Based Shape Representation for Planar Curves" IEEE TPAMI
- Mokhtarian, F., & Murase, H. (1998). "Silhouette-Based Isolated Object Recognition through Curvature Scale Space" IEEE TPAMI

## Author

Jue Han (jhan192@brown.edu)  
LEMS, Brown University  
November 2024
