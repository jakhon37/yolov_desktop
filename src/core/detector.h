
// src/core/detector.h
#pragma once

#include "types.h"
#include <opencv2/dnn.hpp>
#include <memory>

namespace YoloApp {
namespace Core {

/**
 * @brief Interface for object detection engines
 */
class IDetector {
public:
    virtual ~IDetector() = default;
    
    virtual bool loadModel(const std::string& modelPath, 
                          const std::string& configPath = "",
                          const std::string& classesPath = "") = 0;
    
    virtual std::vector<Detection> detectObjects(const cv::Mat& image) = 0;
    
    virtual void setConfig(const DetectionConfig& config) = 0;
    virtual DetectionConfig getConfig() const = 0;
    
    virtual bool isLoaded() const = 0;
    virtual std::string getModelInfo() const = 0;
};

/**
 * @brief YOLO object detection implementation
 */
class YoloDetector : public IDetector {
public:
    YoloDetector();
    ~YoloDetector() override = default;
    
    bool loadModel(const std::string& modelPath, 
                   const std::string& configPath = "",
                   const std::string& classesPath = "") override;
    
                       bool loadModel(const std::string& modelPath, const std::string& configPath = "");
    bool loadONNXModel(const std::string& modelPath);
    bool loadDarknetModel(const std::string& weightsPath, const std::string& configPath);
    std::vector<Detection> detect(const cv::Mat& image);
    void setConfidenceThreshold(float threshold) { confidenceThreshold = threshold; }
    void setNMSThreshold(float threshold) { nmsThreshold = threshold; }
    void setInputSize(int width, int height) { inputWidth = width; inputHeight = height; }

    
    std::vector<Detection> detectObjects(const cv::Mat& image) override;
    
    void setConfig(const DetectionConfig& config) override;
    DetectionConfig getConfig() const override;
    
    bool isLoaded() const override;
    std::string getModelInfo() const override;

private:
    cv::dnn::Net network_;
    std::vector<std::string> classNames_;
    std::vector<std::string> outputNames_;
    DetectionConfig config_;
    std::string modelPath_;
    bool loaded_;
    
    void loadClassNames(const std::string& classesPath);
    std::vector<Detection> postProcessDetections(
        const std::vector<cv::Mat>& outputs, 
        const cv::Size& imageSize);
};

} // namespace Core
} // namespace YoloApp