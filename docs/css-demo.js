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
                this.processImage();
                this.showVisualization();
            };
            img.src = e.target.result;
        };
        reader.readAsDataURL(file);
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
        if (!this.contours) {
            // Extract contours from image (simplified version)
            // In production, this would use OpenCV.js
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
        
        // Draw contour
        ctx.strokeStyle = '#667eea';
        ctx.lineWidth = 2;
        ctx.beginPath();
        ctx.moveTo(smoothed[0].x, smoothed[0].y);
        
        for (let i = 1; i < smoothed.length; i++) {
            ctx.lineTo(smoothed[i].x, smoothed[i].y);
        }
        ctx.closePath();
        ctx.stroke();
        
        // Draw points
        ctx.fillStyle = '#764ba2';
        smoothed.forEach(p => {
            ctx.beginPath();
            ctx.arc(p.x, p.y, 3, 0, 2 * Math.PI);
            ctx.fill();
        });
        
        // Draw sigma label
        ctx.fillStyle = '#2c3e50';
        ctx.font = 'bold 16px Arial';
        ctx.fillText(`σ = ${this.currentSigma.toFixed(1)}`, 10, 25);
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
