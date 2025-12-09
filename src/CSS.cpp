#include "CSS.h"
#include <cmath>
#include <algorithm>
#include <iostream>
#include <fstream>

namespace css
{

    CSS::CSS() : cannyLow_(50), cannyHigh_(150), gaussianKernelSize_(5) {}

    CSS::~CSS() {}

    void CSS::setEdgeDetectionParams(double lowThresh, double highThresh)
    {
        cannyLow_ = lowThresh;
        cannyHigh_ = highThresh;
    }

    // ============================================================================
    // Contour Extraction
    // ============================================================================

    std::vector<cv::Point> CSS::extractContour(const cv::Mat &image)
    {
        cv::Mat gray, binary, edges;

        // Convert to grayscale if needed
        if (image.channels() == 3)
        {
            cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);
        }
        else
        {
            gray = image.clone();
        }

        // Apply Gaussian blur
        cv::GaussianBlur(gray, gray, cv::Size(5, 5), 0);

        // Try adaptive thresholding first to get binary image
        cv::adaptiveThreshold(gray, binary, 255, cv::ADAPTIVE_THRESH_GAUSSIAN_C,
                              cv::THRESH_BINARY_INV, 11, 2);

        // Apply morphological operations to close gaps
        cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(3, 3));
        cv::morphologyEx(binary, binary, cv::MORPH_CLOSE, kernel);

        // Find contours on binary image
        std::vector<std::vector<cv::Point>> contours;
        cv::findContours(binary, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_NONE);

        // If no contours found with thresholding, try Canny as fallback
        if (contours.empty())
        {
            std::cout << "Binary thresholding failed, trying Canny edge detection..." << std::endl;
            cv::Canny(gray, edges, cannyLow_, cannyHigh_);
            cv::findContours(edges, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_NONE);
        }

        // Return the largest contour
        if (contours.empty())
        {
            std::cerr << "Warning: No contours found!" << std::endl;
            return std::vector<cv::Point>();
        }

        // Debug: print number of contours found
        std::cout << "Found " << contours.size() << " contours" << std::endl;

        auto largest = std::max_element(contours.begin(), contours.end(),
                                        [](const std::vector<cv::Point> &a, const std::vector<cv::Point> &b)
                                        {
                                            return cv::contourArea(a) < cv::contourArea(b);
                                        });

        std::cout << "Largest contour area: " << cv::contourArea(*largest) << std::endl;

        return *largest;
    }

    // ============================================================================
    // Gaussian Smoothing
    // ============================================================================

    std::vector<double> CSS::createGaussianKernel(double sigma, int &kernelSize)
    {
        // Kernel size should be odd and cover ~3 sigma on each side
        kernelSize = static_cast<int>(std::ceil(6 * sigma));
        if (kernelSize % 2 == 0)
            kernelSize++;
        if (kernelSize < 3)
            kernelSize = 3;

        std::vector<double> kernel(kernelSize);
        int center = kernelSize / 2;
        double sum = 0.0;

        for (int i = 0; i < kernelSize; i++)
        {
            int x = i - center;
            kernel[i] = std::exp(-(x * x) / (2.0 * sigma * sigma));
            sum += kernel[i];
        }

        // Normalize
        for (double &k : kernel)
        {
            k /= sum;
        }

        return kernel;
    }

    void CSS::gaussianSmooth(const std::vector<cv::Point> &input,
                             std::vector<ContourPoint> &output,
                             double sigma)
    {
        int n = input.size();
        output.resize(n);

        int kernelSize;
        std::vector<double> kernel = createGaussianKernel(sigma, kernelSize);
        int halfKernel = kernelSize / 2;

        // Smooth X and Y coordinates separately (circular convolution)
        for (int i = 0; i < n; i++)
        {
            double sumX = 0.0, sumY = 0.0;

            for (int j = 0; j < kernelSize; j++)
            {
                int idx = (i - halfKernel + j + n) % n; // Circular indexing
                sumX += input[idx].x * kernel[j];
                sumY += input[idx].y * kernel[j];
            }

            output[i].x = sumX;
            output[i].y = sumY;
        }
    }

    std::vector<ContourPoint> CSS::smoothContour(const std::vector<cv::Point> &contour, double sigma)
    {
        std::vector<ContourPoint> smoothed;
        gaussianSmooth(contour, smoothed, sigma);

        // Compute arc length
        double totalLength = 0.0;
        smoothed[0].arcLength = 0.0;

        for (size_t i = 1; i < smoothed.size(); i++)
        {
            double dx = smoothed[i].x - smoothed[i - 1].x;
            double dy = smoothed[i].y - smoothed[i - 1].y;
            totalLength += std::sqrt(dx * dx + dy * dy);
            smoothed[i].arcLength = totalLength;
        }

        // Normalize arc length to [0, 1]
        if (totalLength > 0)
        {
            for (auto &pt : smoothed)
            {
                pt.arcLength /= totalLength;
            }
        }

        return smoothed;
    }

    // ============================================================================
    // Curvature Computation
    // ============================================================================

    void CSS::computeDerivatives(const std::vector<ContourPoint> &contour,
                                 std::vector<double> &dx,
                                 std::vector<double> &dy,
                                 std::vector<double> &d2x,
                                 std::vector<double> &d2y)
    {
        int n = contour.size();
        dx.resize(n);
        dy.resize(n);
        d2x.resize(n);
        d2y.resize(n);

        // Use simple finite differences (contour is already smoothed)
        // For proper implementation, should convolve with g_u and g_uu kernels

        // First derivatives (central differences)
        for (int i = 0; i < n; i++)
        {
            int prev = (i - 1 + n) % n;
            int next = (i + 1) % n;

            dx[i] = (contour[next].x - contour[prev].x) / 2.0;
            dy[i] = (contour[next].y - contour[prev].y) / 2.0;
        }

        // Second derivatives
        for (int i = 0; i < n; i++)
        {
            int prev = (i - 1 + n) % n;
            int next = (i + 1) % n;

            d2x[i] = contour[next].x - 2 * contour[i].x + contour[prev].x;
            d2y[i] = contour[next].y - 2 * contour[i].y + contour[prev].y;
        }
    }

    void CSS::computeDerivativesWithGaussian(const std::vector<cv::Point> &contour,
                                             double sigma,
                                             std::vector<double> &dx,
                                             std::vector<double> &dy,
                                             std::vector<double> &d2x,
                                             std::vector<double> &d2y)
    {
        int n = contour.size();
        dx.resize(n);
        dy.resize(n);
        d2x.resize(n);
        d2y.resize(n);

        // Create Gaussian derivative kernels
        int kernelSize = static_cast<int>(std::ceil(6 * sigma));
        if (kernelSize % 2 == 0)
            kernelSize++;
        if (kernelSize < 3)
            kernelSize = 3;

        int center = kernelSize / 2;
        std::vector<double> g(kernelSize);    // g(u, σ)
        std::vector<double> g_u(kernelSize);  // ∂g/∂u
        std::vector<double> g_uu(kernelSize); // ∂²g/∂u²

        // Compute kernels: g, g_u, g_uu
        double sum_g = 0.0;
        for (int i = 0; i < kernelSize; i++)
        {
            double u = i - center;
            double sigma2 = sigma * sigma;

            // g(u, σ) = (1/√(2πσ²)) * exp(-u²/(2σ²))
            g[i] = std::exp(-(u * u) / (2.0 * sigma2));
            sum_g += g[i];

            // g_u(u, σ) = -(u/σ²) * g(u, σ)
            g_u[i] = -(u / sigma2) * g[i];

            // g_uu(u, σ) = ((u²/σ⁴) - (1/σ²)) * g(u, σ)
            g_uu[i] = ((u * u) / (sigma2 * sigma2) - 1.0 / sigma2) * g[i];
        }

        // Normalize g (g_u and g_uu should sum to 0)
        for (int i = 0; i < kernelSize; i++)
        {
            g[i] /= sum_g;
            g_u[i] /= sum_g;
            g_uu[i] /= sum_g;
        }

        // Convolve x(u) and y(u) with g_u and g_uu
        for (int i = 0; i < n; i++)
        {
            double sum_xu = 0.0, sum_yu = 0.0;
            double sum_xuu = 0.0, sum_yuu = 0.0;

            for (int j = 0; j < kernelSize; j++)
            {
                int idx = (i - center + j + n) % n; // Circular indexing

                // X'(u, σ) = x(u) * g_u(u, σ)
                sum_xu += contour[idx].x * g_u[j];
                sum_yu += contour[idx].y * g_u[j];

                // X''(u, σ) = x(u) * g_uu(u, σ)
                sum_xuu += contour[idx].x * g_uu[j];
                sum_yuu += contour[idx].y * g_uu[j];
            }

            dx[i] = sum_xu;
            dy[i] = sum_yu;
            d2x[i] = sum_xuu;
            d2y[i] = sum_yuu;
        }
    }

    std::vector<double> CSS::computeCurvature(const std::vector<ContourPoint> &smoothedContour)
    {
        std::vector<double> dx, dy, d2x, d2y;
        computeDerivatives(smoothedContour, dx, dy, d2x, d2y);

        int n = smoothedContour.size();
        std::vector<double> curvature(n);

        // Curvature formula: k = (x'y'' - y'x'') / (x'^2 + y'^2)^(1.5)
        for (int i = 0; i < n; i++)
        {
            double numerator = dx[i] * d2y[i] - dy[i] * d2x[i];
            double denominator = std::pow(dx[i] * dx[i] + dy[i] * dy[i], 1.5);

            if (denominator > 1e-10)
            {
                curvature[i] = numerator / denominator;
            }
            else
            {
                curvature[i] = 0.0;
            }
        }

        return curvature;
    }

    // ============================================================================
    // Zero Crossings
    // ============================================================================

    std::vector<int> CSS::findZeroCrossings(const std::vector<double> &curvature)
    {
        std::vector<int> crossings;
        int n = curvature.size();

        for (int i = 0; i < n; i++)
        {
            int next = (i + 1) % n;

            // Check for sign change
            if (curvature[i] * curvature[next] < 0)
            {
                crossings.push_back(i);
            }
        }

        return crossings;
    }

    // ============================================================================
    // CSS Image Computation
    // ============================================================================

    CSSImage CSS::computeCSS(const std::vector<cv::Point> &contour,
                             double maxSigma,
                             int numScales)
    {
        CSSImage css;
        css.maxSigma = maxSigma;
        css.numScales = numScales;

        if (contour.empty())
        {
            std::cerr << "Error: Empty contour!" << std::endl;
            return css;
        }

        // Compute CSS at multiple scales using proper Gaussian convolution
        for (int i = 0; i < numScales; i++)
        {
            double sigma = (i + 1) * maxSigma / numScales;

            // Compute derivatives by convolving with Gaussian derivative kernels
            std::vector<double> dx, dy, d2x, d2y;
            computeDerivativesWithGaussian(contour, sigma, dx, dy, d2x, d2y);

            // Compute arc length (approximate)
            int n = contour.size();
            std::vector<double> arcLength(n);
            arcLength[0] = 0.0;
            double totalLength = 0.0;

            for (int j = 1; j < n; j++)
            {
                double ds = std::sqrt(dx[j - 1] * dx[j - 1] + dy[j - 1] * dy[j - 1]);
                totalLength += ds;
                arcLength[j] = totalLength;
            }

            // Normalize arc length
            for (int j = 0; j < n; j++)
            {
                arcLength[j] /= (totalLength + 1e-10);
            }

            // Compute curvature: κ = (X'Y'' - Y'X'') / (X'² + Y'²)^(3/2)
            std::vector<double> curvature(n);
            for (int j = 0; j < n; j++)
            {
                double numerator = dx[j] * d2y[j] - dy[j] * d2x[j];
                double denominator = std::pow(dx[j] * dx[j] + dy[j] * dy[j], 1.5);

                if (denominator > 1e-10)
                {
                    curvature[j] = numerator / denominator;
                }
                else
                {
                    curvature[j] = 0.0;
                }
            }

            // Find zero crossings
            auto crossings = findZeroCrossings(curvature);

            // Store zero crossings with their arc length and scale
            for (int idx : crossings)
            {
                css.zeroCrossings.push_back({arcLength[idx], sigma});
            }
        }

        // Create visual representation
        css.image = visualizeCSSImage(css);

        return css;
    }

    // ============================================================================
    // Visualization
    // ============================================================================

    cv::Mat CSS::visualizeContour(const std::vector<cv::Point> &contour,
                                  const std::vector<double> &curvature,
                                  cv::Size imgSize)
    {
        cv::Mat vis = cv::Mat::zeros(imgSize, CV_8UC3);

        if (contour.empty())
            return vis;

        // Find bounding box
        cv::Rect bbox = cv::boundingRect(contour);

        // Scale and translate to fit image
        double scale = std::min((imgSize.width - 40.0) / bbox.width,
                                (imgSize.height - 40.0) / bbox.height);

        std::vector<cv::Point> scaled;
        for (const auto &pt : contour)
        {
            int x = static_cast<int>((pt.x - bbox.x) * scale + 20);
            int y = static_cast<int>((pt.y - bbox.y) * scale + 20);
            scaled.push_back(cv::Point(x, y));
        }

        // Draw contour with curvature color coding
        if (!curvature.empty())
        {
            auto crossings = findZeroCrossings(curvature);
            int crossing_idx = 0;
            double maxCurv = *std::max_element(curvature.begin(), curvature.end());
            double minCurv = *std::min_element(curvature.begin(), curvature.end());
            double range = maxCurv - minCurv;

            for (size_t i = 0; i < scaled.size(); i++)
            {
                int next = (i + 1) % scaled.size();

                // Color based on curvature (blue=negative, red=positive)
                double normalized = (curvature[i] - minCurv) / (range + 1e-10);
                cv::Scalar color(255 * (1 - normalized), 0, 255 * normalized);
                cv::line(vis, scaled[i], scaled[next], color, 2);

                if (crossing_idx < crossings.size() && i == crossings[crossing_idx])
                {
                    // it's a zero crossing - mark with green circle
                    cv::circle(vis, scaled[i], 5, cv::Scalar(0, 255, 0), -1);
                    crossing_idx++;
                }
            }
        }
        else
        {
            // Just draw the contour
            cv::polylines(vis, scaled, true, cv::Scalar(0, 255, 0), 2);
        }

        return vis;
    }

    // Overloaded version that uses arc length from smoothed contour
    cv::Mat CSS::visualizeContour(const std::vector<ContourPoint> &smoothedContour,
                                  const std::vector<double> &curvature,
                                  cv::Size imgSize)
    {
        cv::Mat vis = cv::Mat::zeros(imgSize, CV_8UC3);

        if (smoothedContour.empty())
            return vis;

        // Convert to cv::Point and find bounding box
        std::vector<cv::Point> contour;
        for (const auto &pt : smoothedContour)
        {
            contour.push_back(cv::Point(static_cast<int>(pt.x), static_cast<int>(pt.y)));
        }
        cv::Rect bbox = cv::boundingRect(contour);

        // Scale and translate to fit image
        double scale = std::min((imgSize.width - 40.0) / bbox.width,
                                (imgSize.height - 40.0) / bbox.height);

        std::vector<cv::Point> scaled;
        for (const auto &pt : contour)
        {
            int x = static_cast<int>((pt.x - bbox.x) * scale + 20);
            int y = static_cast<int>((pt.y - bbox.y) * scale + 20);
            scaled.push_back(cv::Point(x, y));
        }

        // Draw contour with curvature color coding
        if (!curvature.empty())
        {
            auto crossings = findZeroCrossings(curvature);
            double maxCurv = *std::max_element(curvature.begin(), curvature.end());
            double minCurv = *std::min_element(curvature.begin(), curvature.end());
            double range = maxCurv - minCurv;

            for (size_t i = 0; i < scaled.size(); i++)
            {
                int next = (i + 1) % scaled.size();

                // Color based on curvature (blue=negative, red=positive)
                double normalized = (curvature[i] - minCurv) / (range + 1e-10);
                cv::Scalar color(255 * (1 - normalized), 0, 255 * normalized);
                cv::line(vis, scaled[i], scaled[next], color, 2);
            }

            // Draw zero crossings with arc length annotations
            for (int idx : crossings)
            {
                // Draw green circle at zero crossing
                cv::circle(vis, scaled[idx], 6, cv::Scalar(0, 255, 0), -1);
                cv::circle(vis, scaled[idx], 7, cv::Scalar(255, 255, 255), 1);

                // Get arc length and format it
                double arcLen = smoothedContour[idx].arcLength;
                std::string label = std::to_string(arcLen).substr(0, 4);

                // Position text above and to the right of the point
                cv::Point textPos = scaled[idx] + cv::Point(10, -10);

                // Draw text background
                cv::putText(vis, label, textPos, cv::FONT_HERSHEY_SIMPLEX,
                            0.4, cv::Scalar(0, 0, 0), 3, cv::LINE_AA);
                // Draw text
                cv::putText(vis, label, textPos, cv::FONT_HERSHEY_SIMPLEX,
                            0.4, cv::Scalar(0, 255, 0), 1, cv::LINE_AA);
            }
        }
        else
        {
            // Just draw the contour
            cv::polylines(vis, scaled, true, cv::Scalar(0, 255, 0), 2);
        }

        return vis;
    }

    cv::Mat CSS::visualizeCSSImage(const CSSImage &css)
    {
        if (css.zeroCrossings.empty())
        {
            return cv::Mat::zeros(400, 600, CV_8UC3);
        }

        int width = 800;
        int height = 600;
        // Black background
        cv::Mat img = cv::Mat::zeros(height, width, CV_8UC3);

        // Draw axes in white
        cv::line(img, cv::Point(50, height - 50), cv::Point(width - 20, height - 50),
                 cv::Scalar(255, 255, 255), 2);
        cv::line(img, cv::Point(50, height - 50), cv::Point(50, 20),
                 cv::Scalar(255, 255, 255), 2);

        // Labels in white
        cv::putText(img, "Arc Length", cv::Point(width / 2 - 60, height - 15),
                    cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(255, 255, 255), 2);
        cv::putText(img, "Scale (sigma)", cv::Point(5, height / 2),
                    cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(255, 255, 255), 2);

        // Plot zero crossings as white points
        for (const auto &zc : css.zeroCrossings)
        {
            int x = static_cast<int>(50 + zc.first * (width - 70));
            int y = static_cast<int>(height - 50 - (zc.second / css.maxSigma) * (height - 70));
            cv::circle(img, cv::Point(x, y), 2, cv::Scalar(255, 255, 255), -1);
        }

        return img;
    }

    std::vector<cv::Mat> CSS::generateProgressFrames(const std::vector<cv::Point> &contour,
                                                     double maxSigma,
                                                     int numScales)
    {
        std::vector<cv::Mat> frames;

        for (int i = 0; i <= numScales; i++)
        {
            double sigma = i * maxSigma / numScales;
            if (sigma < 0.1)
                sigma = 0.1; // Avoid zero sigma

            auto smoothed = smoothContour(contour, sigma);

            // Convert to cv::Point for visualization
            std::vector<cv::Point> smoothedPts;
            for (const auto &pt : smoothed)
            {
                smoothedPts.push_back(cv::Point(static_cast<int>(pt.x),
                                                static_cast<int>(pt.y)));
            }

            auto curvature = computeCurvature(smoothed);
            cv::Mat frame = visualizeContour(smoothedPts, curvature);

            // Add sigma value on frame
            std::string text = "Sigma: " + std::to_string(sigma).substr(0, 4);
            cv::putText(frame, text, cv::Point(10, 30),
                        cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(255, 255, 255), 2);

            frames.push_back(frame);
        }

        return frames;
    }

    // ============================================================================
    // GIF Export (requires external tool or library)
    // ============================================================================

    bool CSS::saveAsGIF(const std::vector<cv::Mat> &frames,
                        const std::string &filename,
                        int delayMs)
    {
        // Save individual frames
        std::string baseDir = filename.substr(0, filename.find_last_of('.'));
        std::string framesDir = baseDir + "_frames";

        // Create directory
        std::string mkdirCmd = "mkdir -p " + framesDir;
        system(mkdirCmd.c_str());

        // Save frames as PNG
        for (size_t i = 0; i < frames.size(); i++)
        {
            std::string framePath = framesDir + "/frame_" +
                                    std::string(4 - std::to_string(i).length(), '0') +
                                    std::to_string(i) + ".png";
            cv::imwrite(framePath, frames[i]);
        }

        // Use ImageMagick to create GIF
        std::string gifCmd = "convert -delay " + std::to_string(delayMs / 10) +
                             " -loop 0 " + framesDir + "/frame_*.png " + filename;
        int result = system(gifCmd.c_str());

        if (result == 0)
        {
            std::cout << "GIF saved to: " << filename << std::endl;
            std::cout << "Individual frames saved to: " << framesDir << std::endl;
            return true;
        }
        else
        {
            std::cerr << "Error creating GIF. Make sure ImageMagick is installed." << std::endl;
            std::cout << "Individual frames saved to: " << framesDir << std::endl;
            return false;
        }
    }

    // ============================================================================
    // Distance Metrics
    // ============================================================================

    double CSS::cssDistance(const CSSImage &css1, const CSSImage &css2)
    {
        // Simple distance metric based on matching zero crossings
        // More sophisticated approach would use TOED (Time-Ordered Edit Distance)

        if (css1.zeroCrossings.empty() || css2.zeroCrossings.empty())
        {
            return std::numeric_limits<double>::max();
        }

        // For now, use simple Hausdorff-like distance
        double dist = 0.0;

        for (const auto &zc1 : css1.zeroCrossings)
        {
            double minDist = std::numeric_limits<double>::max();

            for (const auto &zc2 : css2.zeroCrossings)
            {
                double d = std::sqrt(std::pow(zc1.first - zc2.first, 2) +
                                     std::pow(zc1.second - zc2.second, 2));
                minDist = std::min(minDist, d);
            }

            dist += minDist;
        }

        return dist / css1.zeroCrossings.size();
    }

    // ============================================================================
    // Helper Functions
    // ============================================================================

    cv::Mat preprocessImage(const cv::Mat &input)
    {
        cv::Mat processed;

        // Convert to grayscale
        if (input.channels() == 3)
        {
            cv::cvtColor(input, processed, cv::COLOR_BGR2GRAY);
        }
        else
        {
            processed = input.clone();
        }

        // Threshold to binary
        cv::threshold(processed, processed, 127, 255, cv::THRESH_BINARY);

        return processed;
    }

    std::vector<cv::Point> resampleContour(const std::vector<cv::Point> &contour, int numPoints)
    {
        if (contour.size() <= numPoints)
        {
            return contour;
        }

        std::vector<cv::Point> resampled;
        double step = static_cast<double>(contour.size()) / numPoints;

        for (int i = 0; i < numPoints; i++)
        {
            int idx = static_cast<int>(i * step);
            resampled.push_back(contour[idx]);
        }

        return resampled;
    }

} // namespace css
