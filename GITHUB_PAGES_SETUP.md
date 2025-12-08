# GitHub Pages Deployment Instructions

## Quick Setup

To enable the GitHub Pages demo site:

1. **Push the docs folder to GitHub:**
   ```bash
   git add docs/
   git commit -m "Add GitHub Pages interactive demo"
   git push origin main
   ```

2. **Enable GitHub Pages:**
   - Go to: https://github.com/amk09/CSS-Object-Recognition/settings/pages
   - Under "Source", select: **Deploy from a branch**
   - Choose branch: **main**
   - Choose folder: **/docs**
   - Click **Save**

3. **Wait 1-2 minutes** for the site to build and deploy

4. **Access your demo at:**
   - https://amk09.github.io/CSS-Object-Recognition/

## What's Included

The docs/ folder contains:

- **index.html**: Beautiful responsive web interface with gradient design
  - Image upload capability
  - Demo shape generators (circle, rectangle, star, heart)
  - Interactive sigma slider (0.0 to 100.0)
  - Side-by-side visualization panels

- **css-demo.js**: Client-side JavaScript implementation
  - Gaussian smoothing with separable convolution
  - Curvature computation using finite differences
  - CSS image generation with zero-crossing detection
  - Real-time canvas rendering

- **README.md**: Comprehensive documentation
  - Algorithm explanation
  - Usage examples
  - Technical details
  - References to Mokhtarian's paper

- **_config.yml**: Jekyll configuration for GitHub Pages

## Features

The web demo provides:
- ✅ No backend required - runs entirely in browser
- ✅ Instant visualization updates as you move the slider
- ✅ Professional UI with purple gradient theme
- ✅ Responsive design for mobile/tablet/desktop
- ✅ Educational content explaining CSS theory

## Testing Locally

To test before deploying:

```bash
# Option 1: Simple Python server
cd docs/
python -m http.server 8000
# Open: http://localhost:8000

# Option 2: Live server (if you have Node.js)
npx live-server docs/
```

## Customization

You can customize the demo by editing:

- **index.html**: Change colors, layout, text
- **css-demo.js**: Modify algorithm parameters, add new shapes
- **_config.yml**: Update Jekyll theme and metadata

## Troubleshooting

If the page doesn't load:
1. Check that GitHub Pages is enabled in repository settings
2. Verify the branch and folder are set correctly (/docs on main)
3. Look for build errors in the Actions tab
4. Make sure index.html is directly in docs/ (not a subdirectory)

## Next Steps

Consider adding:
- [ ] Sample images in docs/images/ folder
- [ ] More demo shapes (polygon, ellipse, etc.)
- [ ] WebAssembly version of C++ core for better performance
- [ ] Tutorial page with step-by-step examples
- [ ] Comparison with other shape descriptors

---

**Your demo will be live at: https://amk09.github.io/CSS-Object-Recognition/**
