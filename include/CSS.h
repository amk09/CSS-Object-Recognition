#ifndef CSS_H
#define CSS_H

#include <opencv2/opencv.hpp>
#include <vector>
#include <utility>
#include <string>

// =======================================================================================================
// CSS: Curvature Scale Space for Shape Description
//
// Based on: Mokhtarian & Mackworth, "A Theory of Multiscale, Curvature-Based Shape Representation
//           for Planar Curves" (IEEE TPAMI 1992)
//
// ChangeLogs
//    Nov 24, 2024    Created CSS implementation
//
//> (c) LEMS, Brown University
//> Jue Han (jhan192@brown.edu)
// =======================================================================================================

namespace css
{

    // Point on a contour with curvature information
    struct ContourPoint
    {
        double x, y;
        double curvature;
        double arcLength;
    };

    // CSS representation - zero-crossings at different scales
    struct CSSImage
    {
        std::vector<std::pair<double, double>> zeroCrossings; // (arcLength, sigma)
        int numScales;
        double maxSigma;
        cv::Mat image; // Visual representation
    };

    class CSS
    {
    public:
        CSS();
        ~CSS();

        // Main pipeline
        std::vector<cv::Point> extractContour(const cv::Mat &image);
        CSSImage computeCSS(const std::vector<cv::Point> &contour,
                            double maxSigma = 4.0,
                            int numScales = 20);

        // Core algorithms
        std::vector<ContourPoint> smoothContour(const std::vector<cv::Point> &contour, double sigma);
        std::vector<double> computeCurvature(const std::vector<ContourPoint> &smoothedContour);
        std::vector<int> findZeroCrossings(const std::vector<double> &curvature);

        // Visualization
        cv::Mat visualizeContour(const std::vector<cv::Point> &contour,
                                 const std::vector<double> &curvature,
                                 cv::Size imgSize = cv::Size(512, 512));
        cv::Mat visualizeContour(const std::vector<ContourPoint> &smoothedContour,
                                 const std::vector<double> &curvature,
                                 cv::Size imgSize = cv::Size(512, 512));
        cv::Mat visualizeCSSImage(const CSSImage &css);
        std::vector<cv::Mat> generateProgressFrames(const std::vector<cv::Point> &contour,
                                                    double maxSigma,
                                                    int numScales);

        // GIF generation
        bool saveAsGIF(const std::vector<cv::Mat> &frames,
                       const std::string &filename,
                       int delayMs = 100);

        // Distance metrics
        double cssDistance(const CSSImage &css1, const CSSImage &css2);

        // Utilities
        void setEdgeDetectionParams(double lowThresh, double highThresh);

    private:
        // Gaussian kernel for smoothing
        std::vector<double> createGaussianKernel(double sigma, int &kernelSize);

        // Apply Gaussian to X and Y coordinates separately
        void gaussianSmooth(const std::vector<cv::Point> &input,
                            std::vector<ContourPoint> &output,
                            double sigma);

        // Compute derivatives for curvature
        void computeDerivatives(const std::vector<ContourPoint> &contour,
                                std::vector<double> &dx,
                                std::vector<double> &dy,
                                std::vector<double> &d2x,
                                std::vector<double> &d2y);

        // Compute derivatives by convolving with Gaussian derivative kernels
        void computeDerivativesWithGaussian(const std::vector<cv::Point> &contour,
                                            double sigma,
                                            std::vector<double> &dx,
                                            std::vector<double> &dy,
                                            std::vector<double> &d2x,
                                            std::vector<double> &d2y);

        // Arc length computation
        std::vector<double> computeArcLength(const std::vector<ContourPoint> &contour);

        // Canny edge detection parameters
        double cannyLow_;
        double cannyHigh_;
        int gaussianKernelSize_;
    };

    // Helper functions
    cv::Mat preprocessImage(const cv::Mat &input);
    std::vector<cv::Point> resampleContour(const std::vector<cv::Point> &contour, int numPoints);

} // namespace css

#endif // CSS_H
