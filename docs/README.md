# CSS Object Recognition

Interactive web demonstration of Curvature Scale Space (CSS) shape representation and object recognition.

## 🌐 Live Demo

**[Try the interactive demo here!](https://amk09.github.io/CSS-Object-Recognition/)**

## 📖 About

This project implements the Curvature Scale Space (CSS) algorithm for shape representation and object recognition, based on the seminal paper:

> **"A Theory of Multiscale, Curvature-Based Shape Representation for Planar Curves"**  
> Farzin Mokhtarian and Alan Mackworth (1992)

CSS is a powerful multi-scale shape representation technique that tracks the evolution of curvature zero-crossings as a planar curve is smoothed with increasing Gaussian filters. The method is:

- ✅ **Rotation invariant**: Shapes are recognized regardless of orientation
- ✅ **Translation invariant**: Position doesn't affect recognition
- ✅ **Scale invariant**: Works with shapes of different sizes
- ✅ **Robust to noise**: Multi-scale analysis filters out noise

## 🎨 Interactive Features

The web demo allows you to:

1. **Upload your own images** or select demo shapes (circle, rectangle, star, heart)
2. **Adjust the Gaussian sigma** using an interactive slider (0.0 to 100.0)
3. **Visualize in real-time**:
   - **Left panel**: Smoothed contour at current scale
   - **Right panel**: CSS image showing zero-crossings across all scales

## 🛠️ Technical Implementation

### C++ Core (Desktop Application)

The core implementation is in C++ with OpenCV, featuring:

- **Contour extraction**: Multi-strategy approach with threshold and Canny edge detection
- **Gaussian convolution**: Derivative kernels for curvature computation
- **Multi-scale analysis**: 50 scales from σ=0 to σ=100
- **Zero-crossing detection**: Identifies curvature inflection points
- **Interactive tool**: Real-time slider control (`css_interactive`)

#### Building from Source

```bash
# Clone the repository
git clone https://github.com/amk09/CSS-Object-Recognition.git
cd CSS-Object-Recognition

# Build with CMake
mkdir -p build && cd build
cmake ..
make -j4

# Run interactive tool
../run_interactive.sh path/to/image.jpg

# Run demo mode (generates GIF animation)
../run.sh demo path/to/image.jpg
```

#### Requirements

- CMake >= 3.26
- OpenCV >= 4.6.0
- Eigen >= 3.4.0
- Boost >= 1.80.0
- OpenMP >= 4.5
- C++17 compiler

### Web Demo (Client-side JavaScript)

The GitHub Pages demo is implemented purely in client-side JavaScript:

- **No backend required**: All processing happens in the browser
- **Gaussian smoothing**: Implemented with separable convolution
- **Curvature computation**: Finite difference approximation
- **Interactive visualization**: HTML5 Canvas rendering

## 📊 How CSS Works

### Algorithm Overview

1. **Contour Extraction**: Extract the boundary of the object as a parametric curve
2. **Gaussian Smoothing**: Apply Gaussian filters with increasing σ (scale parameter)
3. **Curvature Computation**: Calculate curvature κ(u,σ) using derivatives:
   ```
   κ = (x'y'' - y'x'') / (x'² + y'²)^(3/2)
   ```
4. **Zero-Crossing Detection**: Find points where curvature changes sign
5. **CSS Image**: Plot (u,σ) for all zero-crossings across scales

### The Four Vertex Theorem Connection

The CSS algorithm leverages the **Four Vertex Theorem** from differential geometry:

> *Every simple closed smooth planar curve has at least four vertices (curvature extrema).*

As a shape is smoothed with increasing Gaussian σ:
- Complex shapes → fewer vertices
- Eventually → smooth oval with exactly 4 vertices
- All curvature zero-crossings → disappear at finite σ

The pattern of disappearing zero-crossings forms a unique "fingerprint" for shape recognition.

## 📁 Project Structure

```
CSS-Object-Recognition/
├── docs/                  # GitHub Pages web demo
│   ├── index.html        # Main page
│   └── css-demo.js       # Client-side implementation
├── src/                   # C++ source files
│   ├── CSS.cpp           # Core CSS algorithm (666 lines)
│   ├── Recognition.cpp   # Shape matching with TOED
│   └── toed/             # Time-Ordered Edit Distance
├── include/              # C++ header files
│   ├── CSS.h
│   ├── Recognition.h
│   └── toed/
├── cmd/                  # Command-line applications
│   ├── css_recognition.cpp    # Main app (demo/build/recognize/webcam)
│   └── css_interactive.cpp    # Interactive slider tool
├── input/                # Sample input images
├── test_shapes/          # Test shape animations
├── bin/                  # Compiled binaries
│   ├── css_recognition_app
│   └── css_interactive
├── CMakeLists.txt        # Build configuration
├── run.sh                # Wrapper script for main app
├── run_interactive.sh    # Wrapper for interactive tool
└── create_gif.py         # Python GIF generation script
```

## 🎯 Usage Examples

### Desktop Application

```bash
# Interactive mode with slider
./run_interactive.sh input/scissors.jpg

# Generate smoothing animation GIF
./run.sh demo input/profile.jpg

# Build shape database
./run.sh build database/ shapes/

# Recognize shapes
./run.sh recognize query.jpg database/

# Live webcam recognition
./run.sh webcam database/
```

### Web Demo

1. Open `https://amk09.github.io/CSS-Object-Recognition/`
2. Upload an image or select a demo shape
3. Move the sigma slider to see shape evolution
4. Observe:
   - Left: How the contour smooths
   - Right: Where zero-crossings appear/disappear

## 📚 References

**Primary Paper:**
- Mokhtarian, F., & Mackworth, A. (1992). *A Theory of Multiscale, Curvature-Based Shape Representation for Planar Curves*. IEEE Transactions on Pattern Analysis and Machine Intelligence, 14(8), 789-805.

**Shape Matching:**
- Mokhtarian, F., Abbasi, S., & Kittler, J. (1997). *Efficient and Robust Retrieval by Shape Content through Curvature Scale Space*. Image Databases and Multi-Media Search.

**Four Vertex Theorem:**
- Mukhopadhyaya, S. (1909). *New Methods in the Geometry of a Plane Arc*. Bulletin of the Calcutta Mathematical Society.

## 🤝 Contributing

Contributions are welcome! Areas for improvement:

- [ ] WebAssembly port of C++ core for faster browser processing
- [ ] Integration with OpenCV.js for image upload support
- [ ] Shape database management and batch recognition
- [ ] 3D shape extension (CSS surface representation)
- [ ] Machine learning integration for classification

## 📄 License

This project is available for educational and research purposes. Please cite the original Mokhtarian & Mackworth paper when using this implementation.

## 🙏 Acknowledgments

- Developed at Brown University Center for Computation and Visualization (CCV)
- Based on foundational work by Farzin Mokhtarian and Alan Mackworth
- Built with OpenCV, Eigen, and Boost libraries

---

**Made with ❤️ for computer vision and computational geometry**
