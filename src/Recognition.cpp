#include "Recognition.h"
#include <filesystem>
#include <fstream>
#include <algorithm>
#include <iostream>
#include <cmath>

#ifdef _OPENMP
#include <omp.h>
#endif

namespace fs = std::filesystem;

namespace recognition {

Recognition::Recognition() : maxSigma_(4.0), numScales_(20) {}

Recognition::~Recognition() {}

void Recognition::setCSSParameters(double maxSigma, int numScales) {
    maxSigma_ = maxSigma;
    numScales_ = numScales;
}

void Recognition::setEdgeDetectionParams(double lowThresh, double highThresh) {
    cssComputer_.setEdgeDetectionParams(lowThresh, highThresh);
}

// ============================================================================
// Database Management
// ============================================================================

bool Recognition::loadShapeDatabase(const std::string& databaseDir) {
    if (!fs::exists(databaseDir) || !fs::is_directory(databaseDir)) {
        std::cerr << "Error: Database directory does not exist: " << databaseDir << std::endl;
        return false;
    }
    
    int loadedCount = 0;
    
    for (const auto& entry : fs::directory_iterator(databaseDir)) {
        if (entry.is_regular_file()) {
            std::string path = entry.path().string();
            std::string ext = entry.path().extension().string();
            
            // Check if it's an image file
            if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" || 
                ext == ".bmp" || ext == ".tif") {
                
                cv::Mat img = cv::imread(path, cv::IMREAD_GRAYSCALE);
                if (!img.empty()) {
                    std::string name = entry.path().stem().string();
                    addShape(name, img);
                    loadedCount++;
                    std::cout << "Loaded: " << name << std::endl;
                }
            }
        }
    }
    
    std::cout << "Database loaded: " << loadedCount << " shapes" << std::endl;
    return loadedCount > 0;
}

void Recognition::addShape(const std::string& name, const cv::Mat& image) {
    ShapeEntry entry;
    entry.name = name;
    entry.imagePath = "";
    
    // Extract contour
    entry.contour = cssComputer_.extractContour(image);
    
    if (entry.contour.empty()) {
        std::cerr << "Warning: Could not extract contour for " << name << std::endl;
        return;
    }
    
    // Compute CSS
    entry.cssImage = cssComputer_.computeCSS(entry.contour, maxSigma_, numScales_);
    
    database_.push_back(entry);
}

void Recognition::addShape(const std::string& name, const std::vector<cv::Point>& contour) {
    ShapeEntry entry;
    entry.name = name;
    entry.imagePath = "";
    entry.contour = contour;
    
    // Compute CSS
    entry.cssImage = cssComputer_.computeCSS(contour, maxSigma_, numScales_);
    
    database_.push_back(entry);
}

void Recognition::clearDatabase() {
    database_.clear();
}

void Recognition::saveDatabase(const std::string& filepath) {
    std::ofstream ofs(filepath, std::ios::binary);
    if (!ofs) {
        std::cerr << "Error: Cannot open file for writing: " << filepath << std::endl;
        return;
    }
    
    // Write number of shapes
    size_t numShapes = database_.size();
    ofs.write(reinterpret_cast<const char*>(&numShapes), sizeof(numShapes));
    
    for (const auto& shape : database_) {
        // Write name length and name
        size_t nameLen = shape.name.size();
        ofs.write(reinterpret_cast<const char*>(&nameLen), sizeof(nameLen));
        ofs.write(shape.name.c_str(), nameLen);
        
        // Write contour size and points
        size_t contourSize = shape.contour.size();
        ofs.write(reinterpret_cast<const char*>(&contourSize), sizeof(contourSize));
        for (const auto& pt : shape.contour) {
            ofs.write(reinterpret_cast<const char*>(&pt.x), sizeof(pt.x));
            ofs.write(reinterpret_cast<const char*>(&pt.y), sizeof(pt.y));
        }
        
        // Write CSS zero crossings
        size_t zcSize = shape.cssImage.zeroCrossings.size();
        ofs.write(reinterpret_cast<const char*>(&zcSize), sizeof(zcSize));
        for (const auto& zc : shape.cssImage.zeroCrossings) {
            ofs.write(reinterpret_cast<const char*>(&zc.first), sizeof(zc.first));
            ofs.write(reinterpret_cast<const char*>(&zc.second), sizeof(zc.second));
        }
    }
    
    std::cout << "Database saved to: " << filepath << std::endl;
}

void Recognition::loadDatabase(const std::string& filepath) {
    std::ifstream ifs(filepath, std::ios::binary);
    if (!ifs) {
        std::cerr << "Error: Cannot open file for reading: " << filepath << std::endl;
        return;
    }
    
    clearDatabase();
    
    // Read number of shapes
    size_t numShapes;
    ifs.read(reinterpret_cast<char*>(&numShapes), sizeof(numShapes));
    
    for (size_t i = 0; i < numShapes; i++) {
        ShapeEntry shape;
        
        // Read name
        size_t nameLen;
        ifs.read(reinterpret_cast<char*>(&nameLen), sizeof(nameLen));
        shape.name.resize(nameLen);
        ifs.read(&shape.name[0], nameLen);
        
        // Read contour
        size_t contourSize;
        ifs.read(reinterpret_cast<char*>(&contourSize), sizeof(contourSize));
        shape.contour.resize(contourSize);
        for (auto& pt : shape.contour) {
            ifs.read(reinterpret_cast<char*>(&pt.x), sizeof(pt.x));
            ifs.read(reinterpret_cast<char*>(&pt.y), sizeof(pt.y));
        }
        
        // Read CSS zero crossings
        size_t zcSize;
        ifs.read(reinterpret_cast<char*>(&zcSize), sizeof(zcSize));
        shape.cssImage.zeroCrossings.resize(zcSize);
        for (auto& zc : shape.cssImage.zeroCrossings) {
            ifs.read(reinterpret_cast<char*>(&zc.first), sizeof(zc.first));
            ifs.read(reinterpret_cast<char*>(&zc.second), sizeof(zc.second));
        }
        
        shape.cssImage.maxSigma = maxSigma_;
        shape.cssImage.numScales = numScales_;
        
        database_.push_back(shape);
    }
    
    std::cout << "Database loaded: " << database_.size() << " shapes" << std::endl;
}

// ============================================================================
// Shape Recognition
// ============================================================================

std::vector<std::vector<double>> Recognition::cssToSequences(const css::CSSImage& css) {
    // Convert CSS zero-crossings to format suitable for TOED
    // Group by arc length position
    std::vector<std::vector<double>> sequences;
    
    for (const auto& zc : css.zeroCrossings) {
        sequences.push_back({zc.first, zc.second});  // [arcLength, sigma]
    }
    
    return sequences;
}

double Recognition::toedDistance(const std::vector<std::vector<double>>& seq1,
                                const std::vector<std::vector<double>>& seq2) {
    // Use TOED library for distance computation
    // For now, use simple Euclidean distance as fallback
    // You can integrate your TOED implementation here
    
    if (seq1.empty() || seq2.empty()) {
        return std::numeric_limits<double>::max();
    }
    
    // Simple matching distance (can be replaced with actual TOED)
    double totalDist = 0.0;
    
    for (const auto& pt1 : seq1) {
        double minDist = std::numeric_limits<double>::max();
        
        for (const auto& pt2 : seq2) {
            double d = std::sqrt(std::pow(pt1[0] - pt2[0], 2) + 
                               std::pow(pt1[1] - pt2[1], 2));
            minDist = std::min(minDist, d);
        }
        
        totalDist += minDist;
    }
    
    return totalDist / seq1.size();
}

double Recognition::computeShapeDistance(const css::CSSImage& css1, const css::CSSImage& css2) {
    auto seq1 = cssToSequences(css1);
    auto seq2 = cssToSequences(css2);
    return toedDistance(seq1, seq2);
}

std::vector<ShapeEntry> Recognition::recognizeShape(const cv::Mat& queryImage, int topK) {
    // Extract contour from query image
    auto contour = cssComputer_.extractContour(queryImage);
    
    if (contour.empty()) {
        std::cerr << "Error: Could not extract contour from query image" << std::endl;
        return std::vector<ShapeEntry>();
    }
    
    return recognizeShape(contour, topK);
}

std::vector<ShapeEntry> Recognition::recognizeShape(const std::vector<cv::Point>& queryContour, int topK) {
    if (database_.empty()) {
        std::cerr << "Error: Database is empty!" << std::endl;
        return std::vector<ShapeEntry>();
    }
    
    // Compute CSS for query
    css::CSSImage queryCSS = cssComputer_.computeCSS(queryContour, maxSigma_, numScales_);
    
    // Compute distances to all shapes in database
    std::vector<ShapeEntry> results = database_;
    
    #pragma omp parallel for if(results.size() > 10)
    for (size_t i = 0; i < results.size(); i++) {
        results[i].matchScore = computeShapeDistance(queryCSS, results[i].cssImage);
    }
    
    // Sort by distance (ascending)
    std::sort(results.begin(), results.end(),
        [](const ShapeEntry& a, const ShapeEntry& b) {
            return a.matchScore < b.matchScore;
        });
    
    // Return top K matches
    if (topK < results.size()) {
        results.resize(topK);
    }
    
    return results;
}

// ============================================================================
// Visualization
// ============================================================================

cv::Mat Recognition::visualizeMatches(const cv::Mat& queryImage,
                                      const std::vector<ShapeEntry>& matches,
                                      int maxDisplay) {
    if (matches.empty()) {
        return queryImage.clone();
    }
    
    int numToShow = std::min(maxDisplay, static_cast<int>(matches.size()));
    int tileSize = 200;
    int padding = 10;
    
    // Create output image
    int outputWidth = tileSize * (numToShow + 1) + padding * (numToShow + 2);
    int outputHeight = tileSize + 2 * padding + 60;
    cv::Mat output = cv::Mat::ones(outputHeight, outputWidth, CV_8UC3) * 255;
    
    // Draw query image
    cv::Mat queryResized;
    cv::resize(queryImage, queryResized, cv::Size(tileSize, tileSize));
    if (queryResized.channels() == 1) {
        cv::cvtColor(queryResized, queryResized, cv::COLOR_GRAY2BGR);
    }
    cv::Rect queryRoi(padding, padding, tileSize, tileSize);
    queryResized.copyTo(output(queryRoi));
    cv::putText(output, "Query", cv::Point(padding + 10, tileSize + padding + 25),
                cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(0, 0, 0), 2);
    
    // Draw matches
    for (int i = 0; i < numToShow; i++) {
        int x = padding * (i + 2) + tileSize * (i + 1);
        
        // Draw contour on white background
        cv::Mat matchImg = cv::Mat::ones(tileSize, tileSize, CV_8UC3) * 255;
        
        auto contour = matches[i].contour;
        if (!contour.empty()) {
            cv::Rect bbox = cv::boundingRect(contour);
            double scale = std::min((tileSize - 20.0) / bbox.width,
                                   (tileSize - 20.0) / bbox.height);
            
            std::vector<cv::Point> scaled;
            for (const auto& pt : contour) {
                int px = static_cast<int>((pt.x - bbox.x) * scale + 10);
                int py = static_cast<int>((pt.y - bbox.y) * scale + 10);
                scaled.push_back(cv::Point(px, py));
            }
            
            cv::polylines(matchImg, scaled, true, cv::Scalar(0, 200, 0), 2);
        }
        
        cv::Rect matchRoi(x, padding, tileSize, tileSize);
        matchImg.copyTo(output(matchRoi));
        
        // Add label with score
        std::string label = matches[i].name;
        std::string score = "D: " + std::to_string(matches[i].matchScore).substr(0, 5);
        cv::putText(output, label, cv::Point(x + 10, tileSize + padding + 25),
                    cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 0), 1);
        cv::putText(output, score, cv::Point(x + 10, tileSize + padding + 50),
                    cv::FONT_HERSHEY_SIMPLEX, 0.4, cv::Scalar(100, 100, 100), 1);
    }
    
    return output;
}

} // namespace recognition
