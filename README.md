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
For Brown Oscar User, simply run the following command. The provided script is also compatible with other modes:
```bash
./run.sh demo path/to/image.png
```


This will generate:
- `image_css.png` - CSS image visualization
- `image_animation.gif` - Animated progression of smoothing
- `image_animation_frames/` - Individual frames

You can also run gif generation command by
```bash
python3 create_gif.py path/to/image.png path/to/image.gif 150
```

### 2. Interactive CSS Image Demo

CSS image has 2 axes: arc length as its y axis, and Gaussian filter width as its x axis. This demo allows you tweak around Gaussian filter width to see what the smoothed image is like and where it is located in CSS image. 

To run the demo, use the command:
```bash
./bin/css_interactive path/to/image.png
```
For Brown Oscar User, run the provided script for quick start:
```bash
./run_interactive.sh path/to/image.png
```
Quick Demo:


https://github.com/user-attachments/assets/ff8680c2-6584-4132-ab98-a55100650d1c

### 3. Object Recognition Database
CSS-based Object Recogniton needs database to understand the images. To create a database with a sequence of images, run:
```bash
./bin/css_recognition_app build path/to/folder
```
- `folder` - contains the images of the objects you want to store. Name the image by object name. Make each image representative and silhouette for better CSS image generation. 

### 4. Object Recognition
Given inputed silhouette image, this program is able to recognize the object from built database. It will return top K candidates from candidates marked with score:
```bash
./bin/css_recognition_app recognize path/to/image
```
The input image should be one of the file types:
`.png`, `.jpg`, `.jpeg`, `.bmp`, `.tif`.

A summary of recognition image will be stored in the same folder of your input image.
## Algorithm Overview

### CSS (Curvature Scale Space)

1. **Contour Extraction**: Extract boundary using Canny edge detection
2. **Gaussian Smoothing**: Apply at multiple scales σ = 0 to inifinity
3. **Curvature Calculation**: k = (x'y'' - y'x'') / (x'² + y'²)^(3/2)
4. **Zero-Crossing Detection**: Find inflection points where curvature changes sign
5. **CSS Image**: Plot zero-crossings as (arc length, σ) points

### Object Recognition 

1. **Database Construction**: Load database by given sequence of images, marked with corresponding CSS annotation
2. **Candidate Selection**: Match the given query image with top K candidates in database
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

- F. Mokhatarian and H. Murase, "Silhouette-based object recognition through curvature scale space," 1993 (4th) International Conference on Computer Vision, Berlin, Germany, 1993, pp. 269-274, doi: 10.1109/ICCV.1993.378207.
- F. Mokhtarian, "Silhouette-based isolated object recognition through curvature scale space," in IEEE Transactions on Pattern Analysis and Machine Intelligence, vol. 17, no. 5, pp. 539-544, May 1995, doi: 10.1109/34.391387.

## Author

Jue Han (jue_han@brown.edu)  
LEMS, Brown University  
November 2024
