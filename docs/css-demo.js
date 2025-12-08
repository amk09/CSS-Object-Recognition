// CSS Demo JavaScript - Client-side implementation
// This handles image processing and visualization in the browser

class CSSDemo {
    constructor() {
        this.originalImage = null;
        this.contours = null;
        this.cssData = null;
        this.currentSigma = 5.0;
        
        this.contourCanvas = document.getElementById('contourCanvas');
        this.cssCanvas = document.getElementById('cssCanvas');
        this.contourCtx = this.contourCanvas.getContext('2d');
        this.cssCtx = this.cssCanvas.getContext('2d');
        
        this.setupEventListeners();
        this.loadDemoImages();
    }
    
    setupEventListeners() {
        const fileInput = document.getElementById('fileInput');
        const sigmaSlider = document.getElementById('sigmaSlider');
        
        fileInput.addEventListener('change', (e) => {
            const file = e.target.files[0];
            if (file) {
                this.loadImage(file);
            }
        });
        
        sigmaSlider.addEventListener('input', (e) => {
            this.currentSigma = parseFloat(e.target.value) / 10.0;
            document.getElementById('sigmaValue').textContent = this.currentSigma.toFixed(1);
            this.updateVisualization();
        });
    }
    
    loadDemoImages() {
        const demoContainer = document.getElementById('demoImages');
        const demoShapes = [
            { name: 'Circle', generate: () => this.generateCircle() },
            { name: 'Rectangle', generate: () => this.generateRectangle() },
            { name: 'Star', generate: () => this.generateStar() },
            { name: 'Heart', generate: () => this.generateHeart() }
        ];
        
        demoShapes.forEach(shape => {
            const div = document.createElement('div');
            div.className = 'demo-image';
            div.innerHTML = `<div style="display:flex;align-items:center;justify-content:center;height:150px;background:#f0f0f0;font-weight:600;color:#667eea;">${shape.name}</div>`;
            div.onclick = () => shape.generate();
            demoContainer.appendChild(div);
        });
    }
    
    async loadImage(file) {
        const reader = new FileReader();
        reader.onload = (e) => {
            const img = new Image();
            img.onload = () => {
                this.originalImage = img;
                this.extractContourFromImage(img);
                this.showVisualization();
            };
            img.src = e.target.result;
        };
        reader.readAsDataURL(file);
    }
    
    extractContourFromImage(img) {
        // Create canvas to process image
        const canvas = document.createElement('canvas');
        const ctx = canvas.getContext('2d');
        
        // Scale down large images
        const maxDim = 600;
        let width = img.width;
        let height = img.height;
        
        if (width > maxDim || height > maxDim) {
            const scale = maxDim / Math.max(width, height);
            width = Math.floor(width * scale);
            height = Math.floor(height * scale);
        }
        
        canvas.width = width;
        canvas.height = height;
        
        // Draw image
        ctx.drawImage(img, 0, 0, width, height);
        
        // Get image data
        const imageData = ctx.getImageData(0, 0, width, height);
        const data = imageData.data;
        
        // Convert to grayscale and threshold
        const threshold = 128;
        const edges = [];
        
        for (let y = 1; y < height - 1; y++) {
            for (let x = 1; x < width - 1; x++) {
                const idx = (y * width + x) * 4;
                const gray = (data[idx] + data[idx + 1] + data[idx + 2]) / 3;
                
                // Simple edge detection
                if (gray < threshold) {
                    const hasWhiteNeighbor = this.checkNeighbors(data, x, y, width, height, threshold);
                    if (hasWhiteNeighbor) {
                        edges.push({ x, y });
                    }
                }
            }
        }
        
        // If we found edges, trace the contour
        if (edges.length > 0) {
            this.contours = this.traceContour(edges, width, height);
        } else {
            // Fallback: create rectangle around image
            this.contours = this.createRectangleContour(width, height);
        }
        
        this.processImage();
    }
    
    checkNeighbors(data, x, y, width, height, threshold) {
        const offsets = [
            [-1, -1], [0, -1], [1, -1],
            [-1, 0],           [1, 0],
            [-1, 1],  [0, 1],  [1, 1]
        ];
        
        for (const [dx, dy] of offsets) {
            const nx = x + dx;
            const ny = y + dy;
            if (nx >= 0 && nx < width && ny >= 0 && ny < height) {
                const idx = (ny * width + nx) * 4;
                const gray = (data[idx] + data[idx + 1] + data[idx + 2]) / 3;
                if (gray >= threshold) {
                    return true;
                }
            }
        }
        return false;
    }
    
    traceContour(edges, width, height) {
        if (edges.length === 0) return [];
        
        // Simple contour ordering: sort by angle from center
        const centerX = width / 2;
        const centerY = height / 2;
        
        edges.forEach(p => {
            p.angle = Math.atan2(p.y - centerY, p.x - centerX);
        });
        
        edges.sort((a, b) => a.angle - b.angle);
        
        // Subsample to get ~100-200 points
        const targetPoints = 150;
        const step = Math.max(1, Math.floor(edges.length / targetPoints));
        const contour = [];
        
        for (let i = 0; i < edges.length; i += step) {
            contour.push({
                x: edges[i].x,
                y: edges[i].y
            });
        }
        
        return contour;
    }
    
    createRectangleContour(width, height) {
        const margin = 20;
        const points = [];
        const pointsPerSide = 25;
        
        // Top
        for (let i = 0; i < pointsPerSide; i++) {
            points.push({ 
                x: margin + (i / pointsPerSide) * (width - 2 * margin), 
                y: margin 
            });
        }
        // Right
        for (let i = 0; i < pointsPerSide; i++) {
            points.push({ 
                x: width - margin, 
                y: margin + (i / pointsPerSide) * (height - 2 * margin) 
            });
        }
        // Bottom
        for (let i = 0; i < pointsPerSide; i++) {
            points.push({ 
                x: width - margin - (i / pointsPerSide) * (width - 2 * margin), 
                y: height - margin 
            });
        }
        // Left
        for (let i = 0; i < pointsPerSide; i++) {
            points.push({ 
                x: margin, 
                y: height - margin - (i / pointsPerSide) * (height - 2 * margin) 
            });
        }
        
        return points;
    }
    
    generateCircle() {
        const points = [];
        const numPoints = 100;
        const radius = 200;
        const centerX = 300;
        const centerY = 300;
        
        for (let i = 0; i < numPoints; i++) {
            const angle = (i / numPoints) * 2 * Math.PI;
            points.push({
                x: centerX + radius * Math.cos(angle),
                y: centerY + radius * Math.sin(angle)
            });
        }
        
        this.contours = points;
        this.processImage();
        this.showVisualization();
    }
    
    generateRectangle() {
        const points = [];
        const width = 300;
        const height = 200;
        const x = 150;
        const y = 200;
        const pointsPerSide = 25;
        
        // Top
        for (let i = 0; i < pointsPerSide; i++) {
            points.push({ x: x + (i / pointsPerSide) * width, y: y });
        }
        // Right
        for (let i = 0; i < pointsPerSide; i++) {
            points.push({ x: x + width, y: y + (i / pointsPerSide) * height });
        }
        // Bottom
        for (let i = 0; i < pointsPerSide; i++) {
            points.push({ x: x + width - (i / pointsPerSide) * width, y: y + height });
        }
        // Left
        for (let i = 0; i < pointsPerSide; i++) {
            points.push({ x: x, y: y + height - (i / pointsPerSide) * height });
        }
        
        this.contours = points;
        this.processImage();
        this.showVisualization();
    }
    
    generateStar() {
        const points = [];
        const numPoints = 10;
        const outerRadius = 200;
        const innerRadius = 80;
        const centerX = 300;
        const centerY = 300;
        
        for (let i = 0; i < numPoints * 2; i++) {
            const angle = (i / (numPoints * 2)) * 2 * Math.PI - Math.PI / 2;
            const radius = i % 2 === 0 ? outerRadius : innerRadius;
            points.push({
                x: centerX + radius * Math.cos(angle),
                y: centerY + radius * Math.sin(angle)
            });
        }
        
        this.contours = points;
        this.processImage();
        this.showVisualization();
    }
    
    generateHeart() {
        const points = [];
        const numPoints = 100;
        const scale = 15;
        const centerX = 300;
        const centerY = 250;
        
        for (let i = 0; i < numPoints; i++) {
            const t = (i / numPoints) * 2 * Math.PI;
            const x = scale * (16 * Math.pow(Math.sin(t), 3));
            const y = -scale * (13 * Math.cos(t) - 5 * Math.cos(2*t) - 2 * Math.cos(3*t) - Math.cos(4*t));
            points.push({ x: centerX + x, y: centerY + y });
        }
        
        this.contours = points;
        this.processImage();
        this.showVisualization();
    }
    
    processImage() {
        if (!this.contours || this.contours.length === 0) {
            console.error('No contours available');
            return;
        }
        
        // Compute CSS data for all scales
        this.cssData = this.computeCSS();
        this.updateVisualization();
    }
    
    computeCSS() {
        const maxSigma = 100.0;
        const numScales = 50;
        const cssData = [];
        
        for (let i = 0; i < numScales; i++) {
            const sigma = (i / numScales) * maxSigma;
            const zeroCrossings = this.findZeroCrossings(sigma);
            
            zeroCrossings.forEach(pos => {
                cssData.push({ sigma, position: pos });
            });
        }
        
        return cssData;
    }
    
    findZeroCrossings(sigma) {
        if (!this.contours || this.contours.length === 0) return [];
        
        const smoothed = this.smoothContour(sigma);
        const curvatures = this.computeCurvature(smoothed);
        const zeroCrossings = [];
        
        for (let i = 0; i < curvatures.length; i++) {
            const prev = curvatures[(i - 1 + curvatures.length) % curvatures.length];
            const curr = curvatures[i];
            
            if (prev * curr < 0) { // Sign change
                zeroCrossings.push(i / curvatures.length);
            }
        }
        
        return zeroCrossings;
    }
    
    smoothContour(sigma) {
        if (!this.contours || sigma === 0) return this.contours;
        
        const smoothed = [];
        const n = this.contours.length;
        
        // Gaussian kernel size
        const kernelSize = Math.ceil(sigma * 3) * 2 + 1;
        const kernel = this.createGaussianKernel(kernelSize, sigma);
        
        for (let i = 0; i < n; i++) {
            let sumX = 0, sumY = 0, sumWeight = 0;
            
            for (let j = 0; j < kernelSize; j++) {
                const idx = (i + j - Math.floor(kernelSize / 2) + n) % n;
                const weight = kernel[j];
                sumX += this.contours[idx].x * weight;
                sumY += this.contours[idx].y * weight;
                sumWeight += weight;
            }
            
            smoothed.push({
                x: sumX / sumWeight,
                y: sumY / sumWeight
            });
        }
        
        return smoothed;
    }
    
    createGaussianKernel(size, sigma) {
        const kernel = [];
        const mean = Math.floor(size / 2);
        let sum = 0;
        
        for (let i = 0; i < size; i++) {
            const x = i - mean;
            const val = Math.exp(-(x * x) / (2 * sigma * sigma));
            kernel.push(val);
            sum += val;
        }
        
        // Normalize
        return kernel.map(v => v / sum);
    }
    
    computeCurvature(contour) {
        const curvatures = [];
        const n = contour.length;
        
        for (let i = 0; i < n; i++) {
            const prev = contour[(i - 1 + n) % n];
            const curr = contour[i];
            const next = contour[(i + 1) % n];
            
            // First derivatives
            const dx1 = curr.x - prev.x;
            const dy1 = curr.y - prev.y;
            const dx2 = next.x - curr.x;
            const dy2 = next.y - curr.y;
            
            // Second derivatives
            const ddx = dx2 - dx1;
            const ddy = dy2 - dy1;
            
            // Curvature: (x'y'' - y'x'') / (x'^2 + y'^2)^(3/2)
            const dx = (dx1 + dx2) / 2;
            const dy = (dy1 + dy2) / 2;
            const denominator = Math.pow(dx * dx + dy * dy, 1.5);
            
            if (denominator > 1e-6) {
                curvatures.push((dx * ddy - dy * ddx) / denominator);
            } else {
                curvatures.push(0);
            }
        }
        
        return curvatures;
    }
    
    updateVisualization() {
        if (!this.contours) return;
        
        // Draw smoothed contour
        this.drawContour();
        
        // Draw CSS image
        this.drawCSSImage();
    }
    
    drawContour() {
        const ctx = this.contourCtx;
        const canvas = this.contourCanvas;
        
        // Clear canvas
        ctx.fillStyle = 'white';
        ctx.fillRect(0, 0, canvas.width, canvas.height);
        
        // Get smoothed contour
        const smoothed = this.smoothContour(this.currentSigma);
        
        if (smoothed.length === 0) return;
        
        // Calculate bounding box
        let minX = Infinity, maxX = -Infinity;
        let minY = Infinity, maxY = -Infinity;
        
        smoothed.forEach(p => {
            minX = Math.min(minX, p.x);
            maxX = Math.max(maxX, p.x);
            minY = Math.min(minY, p.y);
            maxY = Math.max(maxY, p.y);
        });
        
        // Calculate scale and offset to fit in canvas with padding
        const padding = 50;
        const contentWidth = maxX - minX;
        const contentHeight = maxY - minY;
        const scaleX = (canvas.width - 2 * padding) / contentWidth;
        const scaleY = (canvas.height - 2 * padding) / contentHeight;
        const scale = Math.min(scaleX, scaleY);
        
        const offsetX = padding + (canvas.width - 2 * padding - contentWidth * scale) / 2 - minX * scale;
        const offsetY = padding + (canvas.height - 2 * padding - contentHeight * scale) / 2 - minY * scale;
        
        // Draw contour
        ctx.strokeStyle = '#667eea';
        ctx.lineWidth = 2;
        ctx.beginPath();
        ctx.moveTo(smoothed[0].x * scale + offsetX, smoothed[0].y * scale + offsetY);
        
        for (let i = 1; i < smoothed.length; i++) {
            ctx.lineTo(smoothed[i].x * scale + offsetX, smoothed[i].y * scale + offsetY);
        }
        ctx.closePath();
        ctx.stroke();
        
        // Draw points
        ctx.fillStyle = '#764ba2';
        smoothed.forEach(p => {
            ctx.beginPath();
            ctx.arc(p.x * scale + offsetX, p.y * scale + offsetY, 3, 0, 2 * Math.PI);
            ctx.fill();
        });
        
        // Draw sigma label
        ctx.fillStyle = '#2c3e50';
        ctx.font = 'bold 16px Arial';
        ctx.fillText(`σ = ${this.currentSigma.toFixed(1)}`, 10, 25);
        ctx.fillText(`Points: ${smoothed.length}`, 10, 45);
    }
    
    drawCSSImage() {
        const ctx = this.cssCtx;
        const canvas = this.cssCanvas;
        
        // Clear with black background
        ctx.fillStyle = 'black';
        ctx.fillRect(0, 0, canvas.width, canvas.height);
        
        if (!this.cssData) return;
        
        const maxSigma = 100.0;
        
        // Draw CSS points (white)
        ctx.fillStyle = 'white';
        this.cssData.forEach(point => {
            const x = point.position * canvas.width;
            const y = (1 - point.sigma / maxSigma) * canvas.height;
            
            ctx.beginPath();
            ctx.arc(x, y, 2, 0, 2 * Math.PI);
            ctx.fill();
        });
        
        // Draw current sigma line (red)
        const currentY = (1 - this.currentSigma / maxSigma) * canvas.height;
        ctx.strokeStyle = '#ff4444';
        ctx.lineWidth = 2;
        ctx.beginPath();
        ctx.moveTo(0, currentY);
        ctx.lineTo(canvas.width, currentY);
        ctx.stroke();
        
        // Draw axes labels
        ctx.fillStyle = 'white';
        ctx.font = '14px Arial';
        ctx.fillText('Contour Position →', 10, canvas.height - 10);
        ctx.save();
        ctx.translate(15, canvas.height / 2);
        ctx.rotate(-Math.PI / 2);
        ctx.fillText('Scale (σ) →', 0, 0);
        ctx.restore();
        
        // Draw current sigma value
        ctx.fillStyle = '#ff4444';
        ctx.font = 'bold 16px Arial';
        ctx.fillText(`σ = ${this.currentSigma.toFixed(1)}`, canvas.width - 100, currentY - 10);
    }
    
    showVisualization() {
        document.getElementById('controls').style.display = 'block';
        document.getElementById('visualization').style.display = 'grid';
    }
}

// Initialize when DOM is ready
document.addEventListener('DOMContentLoaded', () => {
    window.cssDemo = new CSSDemo();
});
