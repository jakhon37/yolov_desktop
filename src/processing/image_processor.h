// src/processing/image_processor.h - FIXED VERSION
#pragma once

#include "../core/types.h"
#include "../core/detector.h"  // ADD THIS INCLUDE
#include <opencv2/opencv.hpp>
#include <memory>

namespace YoloApp {
namespace Processing {

/**
 * @brief Handles image loading, annotation, and metadata extraction
 */
class ImageProcessor {
public:
    ImageProcessor() = default;
    ~ImageProcessor() = default;
    
    /**
     * @brief Load an image from file
     */
    static cv::Mat loadImage(const std::string& imagePath);
    
    /**
     * @brief Create annotated image with detection boxes
     */
    static cv::Mat createAnnotatedImage(const cv::Mat& originalImage, 
                                       const std::vector<Core::Detection>& detections);
    
    /**
     * @brief Generate metadata string for an image
     */
    static std::string generateMetadata(const std::string& imagePath, 
                                       const cv::Mat& image,
                                       const std::vector<Core::Detection>& detections);
    
    /**
     * @brief Process a single image result (load, detect, annotate)
     */
    static void processImageResult(std::shared_ptr<Core::ImageResult> imageResult,
                                  Core::IDetector& detector);  // FIXED: Proper reference
    
    /**
     * @brief Resize image while maintaining aspect ratio
     */
    static cv::Mat resizeImage(const cv::Mat& image, const cv::Size& maxSize);
    
    /**
     * @brief Get image file information
     */
    static std::pair<cv::Size, size_t> getImageInfo(const std::string& imagePath);

private:
    static void drawDetectionBox(cv::Mat& image, const Core::Detection& detection);
    static cv::Scalar getClassColor(int classId);
};

} // namespace Processing
} // namespace YoloApp
