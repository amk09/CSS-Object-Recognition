#include "CSS.h"
#include <opencv2/opencv.hpp>
#include <iostream>
#include <string>

// Global variables for trackbar callback
css::CSS *g_cssComputer = nullptr;
std::vector<cv::Point> *g_originalContour = nullptr;
cv::Mat *g_displayImage = nullptr;
css::CSSImage *g_fullCSSImage = nullptr;
int g_sigmaValue = 10; // Sigma * 10 for trackbar (0-1000 -> 0.0-100.0)
double g_maxSigma = 100.0;
int g_numScales = 50;

void updateVisualization(int, void *)
{
    if (!g_cssComputer || !g_originalContour || !g_displayImage)
        return;

    double sigma = g_sigmaValue / 10.0; // Convert back to actual sigma

    // Smooth contour at current sigma
    auto smoothedContour = g_cssComputer->smoothContour(*g_originalContour, sigma);

    // Compute curvature
    auto curvature = g_cssComputer->computeCurvature(smoothedContour);

    // Visualize smoothed contour with arc length annotations
    cv::Mat contourVis = g_cssComputer->visualizeContour(smoothedContour, curvature);

    // Add sigma text
    std::string sigmaText = "Sigma = " + std::to_string(sigma).substr(0, 5);
    cv::putText(contourVis, sigmaText, cv::Point(10, 30),
                cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(255, 255, 255), 2);

    // Create CSS visualization with current position marked
    cv::Mat cssVis = g_fullCSSImage->image.clone();

    // Draw a horizontal line at current sigma level
    int cssHeight = cssVis.rows;
    int cssWidth = cssVis.cols;
    int yPos = static_cast<int>(cssHeight - 50 - (sigma / g_maxSigma) * (cssHeight - 70));
    cv::line(cssVis, cv::Point(50, yPos), cv::Point(cssWidth - 20, yPos),
             cv::Scalar(0, 255, 0), 2);

    // Add sigma marker text
    cv::putText(cssVis, "Current", cv::Point(cssWidth - 150, yPos - 10),
                cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 255, 0), 2);

    // Combine visualizations side by side
    cv::Mat combined;
    if (contourVis.rows == cssVis.rows)
    {
        cv::hconcat(contourVis, cssVis, combined);
    }
    else
    {
        // Resize to match heights
        int targetHeight = std::max(contourVis.rows, cssVis.rows);
        cv::resize(contourVis, contourVis, cv::Size(contourVis.cols, targetHeight));
        cv::resize(cssVis, cssVis, cv::Size(cssVis.cols, targetHeight));
        cv::hconcat(contourVis, cssVis, combined);
    }

    *g_displayImage = combined;
    cv::imshow("Interactive CSS Visualization", combined);
}

void interactiveMode(const std::string &imagePath)
{
    std::cout << "\n=== Interactive CSS Visualization ===" << std::endl;
    std::cout << "Processing: " << imagePath << std::endl;

    // Load image
    cv::Mat img = cv::imread(imagePath, cv::IMREAD_GRAYSCALE);
    if (img.empty())
    {
        std::cerr << "Error: Cannot load image: " << imagePath << std::endl;
        return;
    }

    // Downscale large images
    int maxDimension = 800;
    if (img.rows > maxDimension || img.cols > maxDimension)
    {
        double scale = maxDimension / static_cast<double>(std::max(img.rows, img.cols));
        cv::Size newSize(static_cast<int>(img.cols * scale), static_cast<int>(img.rows * scale));
        cv::resize(img, img, newSize, 0, 0, cv::INTER_AREA);
        std::cout << "Image downscaled to: " << img.cols << "x" << img.rows << std::endl;
    }

    // Create CSS computer
    css::CSS cssComputer;

    // Extract contour
    std::cout << "Extracting contour..." << std::endl;
    auto contour = cssComputer.extractContour(img);
    if (contour.empty())
    {
        std::cerr << "Error: No contour found!" << std::endl;
        return;
    }
    std::cout << "Contour points: " << contour.size() << std::endl;

    // Compute full CSS image
    std::cout << "Computing full CSS image..." << std::endl;
    css::CSSImage cssImage = cssComputer.computeCSS(contour, g_maxSigma, g_numScales);
    std::cout << "Zero crossings found: " << cssImage.zeroCrossings.size() << std::endl;

    // Setup global variables
    g_cssComputer = &cssComputer;
    g_originalContour = &contour;
    g_fullCSSImage = &cssImage;

    cv::Mat displayImage;
    g_displayImage = &displayImage;

    // Create window and trackbar
    cv::namedWindow("Interactive CSS Visualization", cv::WINDOW_NORMAL);
    cv::resizeWindow("Interactive CSS Visualization", 1600, 600);

    cv::createTrackbar("Sigma x10", "Interactive CSS Visualization",
                       &g_sigmaValue, 1000, updateVisualization);

    // Initial visualization
    updateVisualization(0, nullptr);

    std::cout << "\n=== Controls ===" << std::endl;
    std::cout << "- Use slider to adjust Gaussian sigma (0.0 to 100.0)" << std::endl;
    std::cout << "- Press 's' to save current visualization" << std::endl;
    std::cout << "- Press 'q' or ESC to quit" << std::endl;

    // Main loop
    int frameCount = 0;
    while (true)
    {
        int key = cv::waitKey(30);

        if (key == 'q' || key == 27)
        { // 'q' or ESC
            break;
        }
        else if (key == 's')
        { // Save
            std::string filename = "interactive_sigma_" +
                                   std::to_string(g_sigmaValue / 10.0).substr(0, 5) + ".png";
            cv::imwrite(filename, displayImage);
            std::cout << "Saved: " << filename << std::endl;
        }
    }

    cv::destroyAllWindows();
}

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        std::cout << "\n=== Interactive CSS Visualization ===" << std::endl;
        std::cout << "\nUsage: " << argv[0] << " <image_path>" << std::endl;
        std::cout << "\nExample:" << std::endl;
        std::cout << "  " << argv[0] << " test_shapes/fish.jpg" << std::endl;
        return 1;
    }

    interactiveMode(argv[1]);
    return 0;
}
