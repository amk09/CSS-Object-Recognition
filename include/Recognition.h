#ifndef RECOGNITION_H
#define RECOGNITION_H

#include "CSS.h"
#include "toed/cpu_toed.hpp"
#include <opencv2/opencv.hpp>
#include <string>
#include <vector>
#include <map>

#ifdef _OPENMP
#include <omp.h>
#endif

// =======================================================================================================
// Recognition: Object Recognition based on CSS and TOED
//
// Based on: Mokhtarian & Murase, "Scale-Based Description and Recognition of Planar Curves and
//           Two-Dimensional Shapes" (IEEE TPAMI 1986)
//
// ChangeLogs
//    Nov 24, 2024    Created for object recognition
//
//> (c) LEMS, Brown University
//> Jue Han (jhan192@brown.edu)
// =======================================================================================================

namespace recognition
{

    // Database entry for a shape
    struct ShapeEntry
    {
        std::string name;
        std::string imagePath;
        std::vector<cv::Point> contour;
        css::CSSImage cssImage;
        double matchScore; // For query results
    };

    class Recognition
    {
    public:
        Recognition();
        ~Recognition();

        // Database management
        bool loadShapeDatabase(const std::string &databaseDir);
        void addShape(const std::string &name, const cv::Mat &image);
        void addShape(const std::string &name, const std::vector<cv::Point> &contour);
        void saveDatabase(const std::string &filepath);
        void loadDatabase(const std::string &filepath);
        void clearDatabase();

        // Recognition
        std::vector<ShapeEntry> recognizeShape(const cv::Mat &queryImage, int topK = 5);
        std::vector<ShapeEntry> recognizeShape(const std::vector<cv::Point> &queryContour, int topK = 5);

        // Distance computation
        double computeShapeDistance(const css::CSSImage &css1, const css::CSSImage &css2);

        // Configuration
        void setCSSParameters(double maxSigma, int numScales);
        void setEdgeDetectionParams(double lowThresh, double highThresh);

        // Database info
        int getDatabaseSize() const { return database_.size(); }
        const std::vector<ShapeEntry> &getDatabase() const { return database_; }

        // Visualization
        cv::Mat visualizeMatches(const cv::Mat &queryImage,
                                 const std::vector<ShapeEntry> &matches,
                                 int maxDisplay = 5);

    private:
        css::CSS cssComputer_;
        std::vector<ShapeEntry> database_;

        // CSS parameters
        double maxSigma_;
        int numScales_;

        // Convert CSS zero-crossings to sequences for TOED
        std::vector<std::vector<double>> cssToSequences(const css::CSSImage &css);

        // TOED distance computation
        double toedDistance(const std::vector<std::vector<double>> &seq1,
                            const std::vector<std::vector<double>> &seq2);
    };

} // namespace recognition

#endif // RECOGNITION_H
