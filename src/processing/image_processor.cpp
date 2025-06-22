
// src/processing/image_processor.cpp
#include "image_processor.h"
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <sstream>

namespace fs = std::filesystem;

namespace YoloApp {
namespace Processing {

cv::Mat ImageProcessor::loadImage(const std::string& imagePath) {
    cv::Mat image = cv::imread(imagePath, cv::IMREAD_COLOR);
    if (image.empty()) {
        throw std::runtime_error("Failed to load image: " + imagePath);
    }
    return image;
}

cv::Mat ImageProcessor::createAnnotatedImage(const cv::Mat& originalImage, 
                                           const std::vector<Core::Detection>& detections) {
    cv::Mat annotated = originalImage.clone();
    
    for (const auto& detection : detections) {
        drawDetectionBox(annotated, detection);
    }
    
    return annotated;
}

std::string ImageProcessor::generateMetadata(const std::string& imagePath, 
                                           const cv::Mat& image,
                                           const std::vector<Core::Detection>& detections) {
    std::ostringstream metadata;
    
    // File information
    fs::path filePath(imagePath);
    metadata << "File: " << filePath.filename().string() << "\n";
    metadata << "Path: " << imagePath << "\n";
    
    // Image dimensions
    metadata << "Dimensions: " << image.cols << " x " << image.rows << "\n";
    metadata << "Channels: " << image.channels() << "\n";
    
    // File size
    try {
        size_t fileSize = fs::file_size(filePath);
        metadata << "File Size: " << (fileSize / 1024.0) << " KB\n";
    } catch (const fs::filesystem_error&) {
        metadata << "File Size: Unknown\n";
    }
    
    // Detection summary
    metadata << "\nDetections: " << detections.size() << "\n";
    
    if (!detections.empty()) {
        metadata << "\nDetailed Results:\n";
        for (size_t i = 0; i < detections.size(); ++i) {
            const auto& det = detections[i];
            metadata << "  " << (i + 1) << ". " << det.className 
                    << " (confidence: " << std::fixed << std::setprecision(1) 
                    << (det.confidence * 100) << "%)\n";
            metadata << "     Box: [" << det.boundingBox.x << ", " << det.boundingBox.y 
                    << ", " << det.boundingBox.width << ", " << det.boundingBox.height << "]\n";
        }
    }
    
    return metadata.str();
}

void ImageProcessor::processImageResult(std::shared_ptr<Core::ImageResult> imageResult,
                                       Core::IDetector& detector) {
    if (!imageResult || imageResult->processed) {
        return;
    }
    
    try {
        // Load original image
        imageResult->originalImage = loadImage(imageResult->imagePath);
        
        // Perform detection
        imageResult->detections = detector.detectObjects(imageResult->originalImage);
        
        // Create annotated image
        imageResult->annotatedImage = createAnnotatedImage(imageResult->originalImage, 
                                                          imageResult->detections);
        
        // Generate metadata
        imageResult->metadata = generateMetadata(imageResult->imagePath, 
                                               imageResult->originalImage,
                                               imageResult->detections);
        
        imageResult->processed = true;
        
    } catch (const std::exception& e) {
        // Mark as processed even if failed to avoid retrying
        imageResult->processed = true;
        imageResult->metadata = "Error processing image: " + std::string(e.what());
    }
}

cv::Mat ImageProcessor::resizeImage(const cv::Mat& image, const cv::Size& maxSize) {
    if (image.empty()) {
        return image;
    }
    
    float scaleX = static_cast<float>(maxSize.width) / image.cols;
    float scaleY = static_cast<float>(maxSize.height) / image.rows;
    float scale = std::min(scaleX, scaleY);
    
    if (scale >= 1.0f) {
        return image; // No need to resize
    }
    
    cv::Size newSize(static_cast<int>(image.cols * scale), 
                     static_cast<int>(image.rows * scale));
    
    cv::Mat resized;
    cv::resize(image, resized, newSize, 0, 0, cv::INTER_LINEAR);
    return resized;
}

std::pair<cv::Size, size_t> ImageProcessor::getImageInfo(const std::string& imagePath) {
    cv::Size size(0, 0);
    size_t fileSize = 0;
    
    try {
        cv::Mat image = cv::imread(imagePath, cv::IMREAD_UNCHANGED);
        if (!image.empty()) {
            size = cv::Size(image.cols, image.rows);
        }
        
        fileSize = fs::file_size(imagePath);
    } catch (const std::exception&) {
        // Return default values
    }
    
    return {size, fileSize};
}

void ImageProcessor::drawDetectionBox(cv::Mat& image, const Core::Detection& detection) {
    cv::Scalar color = getClassColor(detection.classId);
    
    // Draw bounding box
    cv::rectangle(image, detection.boundingBox, color, 2);
    
    // Prepare label
    std::ostringstream labelStream;
    labelStream << detection.className << " " 
                << std::fixed << std::setprecision(0) 
                << (detection.confidence * 100) << "%";
    std::string label = labelStream.str();
    
    // Calculate text size
    int baseline = 0;
    cv::Size textSize = cv::getTextSize(label, cv::FONT_HERSHEY_SIMPLEX, 0.5, 1, &baseline);
    
    // Draw label background
    cv::Point labelPos(detection.boundingBox.x, detection.boundingBox.y - 5);
    cv::Rect labelRect(labelPos.x, labelPos.y - textSize.height - baseline,
                       textSize.width + 4, textSize.height + baseline + 4);
    
    cv::rectangle(image, labelRect, color, cv::FILLED);
    
    // Draw label text
    cv::putText(image, label, cv::Point(labelPos.x + 2, labelPos.y - baseline),
                cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 255, 255), 1);
}

cv::Scalar ImageProcessor::getClassColor(int classId) {
    // Generate consistent colors for different classes
    static const std::vector<cv::Scalar> colors = {
        cv::Scalar(255, 0, 0),     // Red
        cv::Scalar(0, 255, 0),     // Green
        cv::Scalar(0, 0, 255),     // Blue
        cv::Scalar(255, 255, 0),   // Yellow
        cv::Scalar(255, 0, 255),   // Magenta
        cv::Scalar(0, 255, 255),   // Cyan
        cv::Scalar(128, 0, 128),   // Purple
        cv::Scalar(255, 165, 0),   // Orange
        cv::Scalar(0, 128, 0),     // Dark Green
        cv::Scalar(128, 0, 0),     // Maroon
    };
    
    return colors[classId % colors.size()];
}

} // namespace Processing
} // namespace YoloApp