// src/core/types.h
#pragma once

#include <opencv2/opencv.hpp>
#include <string>
#include <vector>
#include <memory>

namespace YoloApp {
namespace Core {

/**
 * @brief Represents a single object detection
 */
struct Detection {
    cv::Rect boundingBox;
    float confidence;
    int classId;
    std::string className;
    
    Detection() : confidence(0.0f), classId(-1) {}
    
    Detection(const cv::Rect& bbox, float conf, int id, const std::string& name)
        : boundingBox(bbox), confidence(conf), classId(id), className(name) {}
};

/**
 * @brief Contains results for a single image
 */
struct ImageResult {
    std::string imagePath;
    std::vector<Detection> detections;
    cv::Mat originalImage;
    cv::Mat annotatedImage;
    std::string metadata;
    bool processed = false;
    
    ImageResult() = default;
    explicit ImageResult(const std::string& path) : imagePath(path) {}
    
    int getDetectionCount() const { return static_cast<int>(detections.size()); }
    bool hasDetections() const { return !detections.empty(); }
};

/**
 * @brief Contains results for an entire folder
 */
struct FolderResult {
    std::string folderPath;
    std::string folderName;
    std::vector<std::shared_ptr<ImageResult>> images;
    int totalDetections = 0;
    int imageCount = 0;
    bool processed = false;
    
    FolderResult() = default;
    explicit FolderResult(const std::string& path);
    
    void updateCounts();
    std::string getSummary() const;
};

/**
 * @brief Detection configuration parameters
 */
struct DetectionConfig {
    float confidenceThreshold = 0.5f;
    float nmsThreshold = 0.4f;
    int inputWidth = 640;
    int inputHeight = 640;
    std::vector<std::string> targetClasses; // Empty means all classes
    
    bool isValid() const {
        return confidenceThreshold > 0.0f && confidenceThreshold <= 1.0f &&
               nmsThreshold > 0.0f && nmsThreshold <= 1.0f &&
               inputWidth > 0 && inputHeight > 0;
    }
};

/**
 * @brief Processing statistics
 */
struct ProcessingStats {
    int totalFolders = 0;
    int processedFolders = 0;
    int totalImages = 0;
    int processedImages = 0;
    int totalDetections = 0;
    std::chrono::steady_clock::time_point startTime;
    std::chrono::steady_clock::time_point endTime;
    
    void start() { startTime = std::chrono::steady_clock::now(); }
    void finish() { endTime = std::chrono::steady_clock::now(); }
    
    double getElapsedSeconds() const {
        auto duration = endTime - startTime;
        return std::chrono::duration<double>(duration).count();
    }
    
    double getImagesPerSecond() const {
        double elapsed = getElapsedSeconds();
        return elapsed > 0 ? processedImages / elapsed : 0.0;
    }
};

} // namespace Core
} // namespace YoloApp
