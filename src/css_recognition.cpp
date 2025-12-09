#include "CSS.h"
#include "Recognition.h"
#include <opencv2/opencv.hpp>
#include <iostream>
#include <string>

void printUsage(const char *programName)
{
    std::cout << "\n=== CSS Object Recognition System ===" << std::endl;
    std::cout << "\nUsage: " << programName << " <mode> [options]\n"
              << std::endl;
    std::cout << "Modes:" << std::endl;
    std::cout << "  1. demo <image>              - Demo CSS on single image with GIF output" << std::endl;
    std::cout << "  2. build <database_dir>      - Build shape database from images" << std::endl;
    std::cout << "  3. recognize <query_image>   - Recognize shape from query image" << std::endl;
    std::cout << "  4. webcam                    - Live recognition from webcam" << std::endl;
    std::cout << "\nExamples:" << std::endl;
    std::cout << "  " << programName << " demo shapes/apple.png" << std::endl;
    std::cout << "  " << programName << " build database/shapes/" << std::endl;
    std::cout << "  " << programName << " recognize test.png" << std::endl;
    std::cout << "  " << programName << " webcam" << std::endl;
    std::cout << std::endl;
}

void demoMode(const std::string &imagePath)
{
    std::cout << "\n=== Demo Mode ===" << std::endl;
    std::cout << "Processing: " << imagePath << std::endl;

    // Load image
    cv::Mat img = cv::imread(imagePath, cv::IMREAD_GRAYSCALE);
    if (img.empty())
    {
        std::cerr << "Error: Cannot load image: " << imagePath << std::endl;
        return;
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

    // Visualize and save original contour (before smoothing)
    std::cout << "Saving original contour..." << std::endl;
    std::vector<double> emptyKappa; // No curvature visualization for raw contour
    cv::Mat contourVis = cssComputer.visualizeContour(contour, emptyKappa);
    std::string contourPath = imagePath.substr(0, imagePath.find_last_of('.')) + "_contour.png";
    cv::imwrite(contourPath, contourVis);
    std::cout << "Original contour saved to: " << contourPath << std::endl;

    // Compute CSS
    std::cout << "Computing CSS..." << std::endl;
    double maxSigma = 100.0; // Increased to 100 for stronger convergence to ellipse/circle
    int numScales = 50;      // More scales for smoother progression
    auto cssImg = cssComputer.computeCSS(contour, maxSigma, numScales);
    std::cout << "Zero crossings found: " << cssImg.zeroCrossings.size() << std::endl;

    // Generate progress frames
    std::cout << "Generating animation frames..." << std::endl;
    auto frames = cssComputer.generateProgressFrames(contour, maxSigma, numScales);

    // Save CSS image
    std::string cssPath = imagePath.substr(0, imagePath.find_last_of('.')) + "_css.png";
    cv::imwrite(cssPath, cssImg.image);
    std::cout << "CSS image saved to: " << cssPath << std::endl;

    // Save GIF
    std::string gifPath = imagePath.substr(0, imagePath.find_last_of('.')) + "_animation.gif";
    cssComputer.saveAsGIF(frames, gifPath, 150);

    // Display results
    cv::imshow("Original", img);
    cv::imshow("CSS Image", cssImg.image);
    if (!frames.empty())
    {
        cv::imshow("Final Frame", frames.back());
    }

    std::cout << "\nPress any key to exit..." << std::endl;
    cv::waitKey(0);
}

void buildDatabaseMode(const std::string &databaseDir)
{
    std::cout << "\n=== Build Database Mode ===" << std::endl;
    std::cout << "Loading shapes from: " << databaseDir << std::endl;

    recognition::Recognition recognizer;

    if (!recognizer.loadShapeDatabase(databaseDir))
    {
        std::cerr << "Error: Failed to load database!" << std::endl;
        return;
    }

    // Save database
    std::string dbPath = "shape_database.dat";
    recognizer.saveDatabase(dbPath);

    std::cout << "\nDatabase built successfully!" << std::endl;
    std::cout << "Total shapes: " << recognizer.getDatabaseSize() << std::endl;
}

void recognizeMode(const std::string &queryPath)
{
    std::cout << "\n=== Recognition Mode ===" << std::endl;
    std::cout << "Query image: " << queryPath << std::endl;

    // Load query image
    cv::Mat query = cv::imread(queryPath);
    if (query.empty())
    {
        std::cerr << "Error: Cannot load query image: " << queryPath << std::endl;
        return;
    }

    // Load database
    recognition::Recognition recognizer;
    recognizer.loadDatabase("shape_database.dat");

    if (recognizer.getDatabaseSize() == 0)
    {
        std::cerr << "Error: Database is empty! Run build mode first." << std::endl;
        return;
    }

    // Recognize
    std::cout << "Recognizing shape..." << std::endl;
    auto matches = recognizer.recognizeShape(query, 5);

    if (matches.empty())
    {
        std::cerr << "No matches found!" << std::endl;
        return;
    }

    // Display results
    std::cout << "\nTop matches:" << std::endl;
    for (size_t i = 0; i < matches.size(); i++)
    {
        std::cout << (i + 1) << ". " << matches[i].name
                  << " (distance: " << matches[i].matchScore << ")" << std::endl;
    }

    // Visualize
    cv::Mat visualization = recognizer.visualizeMatches(query, matches);
    cv::imshow("Recognition Results", visualization);

    std::string outPath = queryPath.substr(0, queryPath.find_last_of('.')) + "_matches.png";
    cv::imwrite(outPath, visualization);
    std::cout << "\nResults saved to: " << outPath << std::endl;

    std::cout << "\nPress any key to exit..." << std::endl;
    cv::waitKey(0);
}

void webcamMode()
{
    std::cout << "\n=== Webcam Mode ===" << std::endl;

    // Load database
    recognition::Recognition recognizer;
    recognizer.loadDatabase("shape_database.dat");

    if (recognizer.getDatabaseSize() == 0)
    {
        std::cerr << "Error: Database is empty! Run build mode first." << std::endl;
        return;
    }

    std::cout << "Database loaded: " << recognizer.getDatabaseSize() << " shapes" << std::endl;

    // Open webcam
    cv::VideoCapture cap(0);
    if (!cap.isOpened())
    {
        std::cerr << "Error: Cannot open webcam!" << std::endl;
        return;
    }

    std::cout << "\nControls:" << std::endl;
    std::cout << "  SPACE - Capture and recognize current frame" << std::endl;
    std::cout << "  'q' or ESC - Quit" << std::endl;

    cv::Mat frame, gray;
    css::CSS cssComputer;

    while (true)
    {
        cap >> frame;
        if (frame.empty())
            break;

        // Convert to grayscale for display
        cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);

        // Try to extract contour for preview
        auto contour = cssComputer.extractContour(gray);
        cv::Mat display = frame.clone();

        if (!contour.empty())
        {
            cv::drawContours(display, std::vector<std::vector<cv::Point>>{contour},
                             -1, cv::Scalar(0, 255, 0), 2);

            // Show contour info
            std::string info = "Contour: " + std::to_string(contour.size()) + " points";
            cv::putText(display, info, cv::Point(10, 30),
                        cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0, 255, 0), 2);
        }

        cv::putText(display, "Press SPACE to recognize", cv::Point(10, frame.rows - 20),
                    cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(255, 255, 255), 2);

        cv::imshow("Webcam - CSS Recognition", display);

        int key = cv::waitKey(30);
        if (key == ' ')
        { // Space bar
            std::cout << "\nRecognizing..." << std::endl;

            auto matches = recognizer.recognizeShape(gray, 3);

            if (!matches.empty())
            {
                std::cout << "Best match: " << matches[0].name
                          << " (distance: " << matches[0].matchScore << ")" << std::endl;

                cv::Mat visualization = recognizer.visualizeMatches(frame, matches, 3);
                cv::imshow("Match Results", visualization);
            }
            else
            {
                std::cout << "No matches found!" << std::endl;
            }
        }
        else if (key == 'q' || key == 27)
        { // 'q' or ESC
            break;
        }
    }

    cap.release();
    cv::destroyAllWindows();
}

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        printUsage(argv[0]);
        return 1;
    }

    std::string mode = argv[1];

    try
    {
        if (mode == "demo" && argc >= 3)
        {
            demoMode(argv[2]);
        }
        else if (mode == "build" && argc >= 3)
        {
            buildDatabaseMode(argv[2]);
        }
        else if (mode == "recognize" && argc >= 3)
        {
            recognizeMode(argv[2]);
        }
        else if (mode == "webcam")
        {
            webcamMode();
        }
        else
        {
            std::cerr << "Error: Invalid mode or missing arguments!" << std::endl;
            printUsage(argv[0]);
            return 1;
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
