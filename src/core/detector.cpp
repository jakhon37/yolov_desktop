// src/core/detector.cpp - FIXED VERSION with warning fixes
#include "detector.h"
#include "config.h"
#include <fstream>
#include <algorithm>
#include <stdexcept>

namespace YoloApp {
namespace Core {

// FolderResult implementation
FolderResult::FolderResult(const std::string& path) : folderPath(path) {
    // Extract folder name from path
    size_t pos = path.find_last_of("/\\");
    folderName = (pos != std::string::npos) ? path.substr(pos + 1) : path;
}

void FolderResult::updateCounts() {
    imageCount = static_cast<int>(images.size());
    totalDetections = 0;
    for (const auto& img : images) {
        if (img) {
            totalDetections += img->getDetectionCount();
        }
    }
}

std::string FolderResult::getSummary() const {
    return folderName + " (" + std::to_string(imageCount) + " images, " + 
           std::to_string(totalDetections) + " detections)";
}

// YoloDetector implementation
YoloDetector::YoloDetector() : loaded_(false) {
    // Initialize with default COCO classes
    classNames_ = Config::COCO_CLASSES;
}


bool YOLODetector::loadModel(const std::string& modelPath, const std::string& configPath) {
    try {
        // Determine model type based on extension
        std::string ext = modelPath.substr(modelPath.find_last_of(".") + 1);
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
        
        if (ext == "onnx") {
            return loadONNXModel(modelPath);
        } else if (ext == "weights" && !configPath.empty()) {
            return loadDarknetModel(modelPath, configPath);
        } else {
            std::cerr << "Unsupported model format: " << ext << std::endl;
            return false;
        }
    } catch (const cv::Exception& e) {
        std::cerr << "Error loading model: " << e.what() << std::endl;
        return false;
    }
}

bool YOLODetector::loadONNXModel(const std::string& modelPath) {
    try {
        net = cv::dnn::readNetFromONNX(modelPath);
        if (net.empty()) {
            std::cerr << "Failed to load ONNX model" << std::endl;
            return false;
        }
        
        // Set backend and target
        net.setPreferableBackend(cv::dnn::DNN_BACKEND_OPENCV);
        net.setPreferableTarget(cv::dnn::DNN_TARGET_CPU);
        
        std::cout << "ONNX model loaded successfully" << std::endl;
        return true;
    } catch (const cv::Exception& e) {
        std::cerr << "Error loading ONNX model: " << e.what() << std::endl;
        return false;
    }
}

bool YOLODetector::loadDarknetModel(const std::string& weightsPath, const std::string& configPath) {
    try {
        net = cv::dnn::readNetFromDarknet(configPath, weightsPath);
        if (net.empty()) {
            std::cerr << "Failed to load Darknet model" << std::endl;
            return false;
        }
        
        // Set backend and target
        net.setPreferableBackend(cv::dnn::DNN_BACKEND_OPENCV);
        net.setPreferableTarget(cv::dnn::DNN_TARGET_CPU);
        
        std::cout << "Darknet model loaded successfully" << std::endl;
        return true;
    } catch (const cv::Exception& e) {
        std::cerr << "Error loading Darknet model: " << e.what() << std::endl;
        return false;
    }
}



bool YoloDetector::loadModel(const std::string& modelPath, 
                            const std::string& configPath,
                            const std::string& classesPath) {
    try {
        loaded_ = false;
        modelPath_ = modelPath;
        
        // Determine model type and load accordingly
        std::string extension = modelPath.substr(modelPath.find_last_of('.'));
        std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
        
        if (extension == ".onnx") {
            network_ = cv::dnn::readNetFromONNX(modelPath);
        } else if (extension == ".weights" && !configPath.empty()) {
            network_ = cv::dnn::readNetFromDarknet(configPath, modelPath);
        } else if (extension == ".pb") {
            network_ = cv::dnn::readNetFromTensorflow(modelPath);
        } else {
            throw std::runtime_error("Unsupported model format: " + extension);
        }
        
        if (network_.empty()) {
            throw std::runtime_error("Failed to load network from: " + modelPath);
        }
        
        // Get output layer names
        outputNames_ = network_.getUnconnectedOutLayersNames();
        
        // Load custom class names if provided
        if (!classesPath.empty()) {
            loadClassNames(classesPath);
        }
        
        // Set default backend and target
        network_.setPreferableBackend(cv::dnn::DNN_BACKEND_OPENCV);
        network_.setPreferableTarget(cv::dnn::DNN_TARGET_CPU);
        
        loaded_ = true;
        return true;
        
    } catch (const std::exception& e) {
        loaded_ = false;
        return false;
    }
}

std::vector<Detection> YoloDetector::detectObjects(const cv::Mat& image) {
    if (!loaded_ || image.empty()) {
        return {};
    }
    
    try {
        // Prepare input blob
        cv::Mat blob;
        cv::dnn::blobFromImage(image, blob, 1.0/255.0, 
                              cv::Size(config_.inputWidth, config_.inputHeight), 
                              cv::Scalar(0,0,0), true, false);
        
        network_.setInput(blob);
        
        // Run inference
        std::vector<cv::Mat> outputs;
        network_.forward(outputs, outputNames_);
        
        // Post-process results
        return postProcessDetections(outputs, image.size());
        
    } catch (const std::exception& e) {
        return {};
    }
}

void YoloDetector::setConfig(const DetectionConfig& config) {
    if (config.isValid()) {
        config_ = config;
    }
}

DetectionConfig YoloDetector::getConfig() const {
    return config_;
}

bool YoloDetector::isLoaded() const {
    return loaded_;
}

std::string YoloDetector::getModelInfo() const {
    if (!loaded_) {
        return "No model loaded";
    }
    
    return "Model: " + modelPath_ + 
           "\nClasses: " + std::to_string(classNames_.size()) +
           "\nInput size: " + std::to_string(config_.inputWidth) + "x" + std::to_string(config_.inputHeight) +
           "\nConfidence threshold: " + std::to_string(config_.confidenceThreshold) +
           "\nNMS threshold: " + std::to_string(config_.nmsThreshold);
}

void YoloDetector::loadClassNames(const std::string& classesPath) {
    std::ifstream file(classesPath);
    if (!file.is_open()) {
        return; // Keep default classes
    }
    
    classNames_.clear();
    std::string line;
    while (std::getline(file, line)) {
        if (!line.empty()) {
            classNames_.push_back(line);
        }
    }
}

std::vector<Detection> YoloDetector::postProcessDetections(
    const std::vector<cv::Mat>& outputs, 
    const cv::Size& imageSize) {
    
    std::vector<Detection> detections;
    
    if (outputs.empty()) {
        return detections;
    }
    
    float xFactor = static_cast<float>(imageSize.width) / config_.inputWidth;
    float yFactor = static_cast<float>(imageSize.height) / config_.inputHeight;
    
    std::vector<int> classIds;
    std::vector<float> confidences;
    std::vector<cv::Rect> boxes;
    
    // Process each output
    for (const auto& output : outputs) {
        const float* data = reinterpret_cast<const float*>(output.data);
        
        for (int i = 0; i < output.rows; ++i) {
            const float* row = data + i * output.cols;
            
            // Skip if not enough columns for bbox + objectness + classes
            if (output.cols < 5) continue;
            
            float objectness = row[4];
            if (objectness < config_.confidenceThreshold) continue;
            
            // Find best class
            float maxClassScore = 0.0f;
            int bestClassId = -1;
            
            for (int j = 5; j < output.cols; ++j) {
                float classScore = row[j];
                if (classScore > maxClassScore) {
                    maxClassScore = classScore;
                    bestClassId = j - 5;
                }
            }
            
            float confidence = objectness * maxClassScore;
            if (confidence < config_.confidenceThreshold) continue;
            
            // Check if class is in target classes (if specified)
            if (!config_.targetClasses.empty()) {
                std::string className = (static_cast<size_t>(bestClassId) < classNames_.size()) ? 
                                       classNames_[bestClassId] : "unknown";  // FIXED: Cast to size_t
                
                bool isTargetClass = std::find(config_.targetClasses.begin(), 
                                             config_.targetClasses.end(), 
                                             className) != config_.targetClasses.end();
                if (!isTargetClass) continue;
            }
            
            // Extract bounding box
            float centerX = row[0] * xFactor;
            float centerY = row[1] * yFactor;
            float width = row[2] * xFactor;
            float height = row[3] * yFactor;
            
            int left = static_cast<int>(centerX - width / 2);
            int top = static_cast<int>(centerY - height / 2);
            
            // Clamp to image bounds
            left = std::max(0, std::min(left, imageSize.width - 1));
            top = std::max(0, std::min(top, imageSize.height - 1));
            int right = std::max(0, std::min(static_cast<int>(centerX + width / 2), imageSize.width - 1));
            int bottom = std::max(0, std::min(static_cast<int>(centerY + height / 2), imageSize.height - 1));
            
            classIds.push_back(bestClassId);
            confidences.push_back(confidence);
            boxes.push_back(cv::Rect(left, top, right - left, bottom - top));
        }
    }
    
    // Apply Non-Maximum Suppression
    std::vector<int> indices;
    cv::dnn::NMSBoxes(boxes, confidences, config_.confidenceThreshold, 
                      config_.nmsThreshold, indices);
    
    // Create final detections
    detections.reserve(indices.size());
    for (int idx : indices) {
        Detection det;
        det.boundingBox = boxes[idx];
        det.confidence = confidences[idx];
        det.classId = classIds[idx];
        det.className = (static_cast<size_t>(classIds[idx]) < classNames_.size()) ? 
                       classNames_[classIds[idx]] : "unknown";  // FIXED: Cast to size_t
        detections.push_back(det);
    }
    
    return detections;
}

} // namespace Core
} // namespace YoloApp