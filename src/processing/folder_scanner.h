// src/processing/folder_scanner.h
#pragma once

#include "../core/types.h"
#include <string>
#include <vector>
#include <functional>

namespace YoloApp {
namespace Processing {

/**
 * @brief Scans folders for images and organizes them
 */
class FolderScanner {
public:
    using ProgressCallback = std::function<void(int current, int total, const std::string& currentPath)>;
    
    FolderScanner() = default;
    ~FolderScanner() = default;
    
    /**
     * @brief Scan a root path for images
     * @param rootPath Root directory to scan
     * @param recursive Whether to scan subdirectories
     * @return Vector of folder results containing image paths
     */
    std::vector<Core::FolderResult> scanForImages(const std::string& rootPath, 
                                                 bool recursive = true);
    
    /**
     * @brief Set progress callback for scanning operations
     */
    void setProgressCallback(ProgressCallback callback);
    
    /**
     * @brief Check if a file is a supported image format
     */
    static bool isImageFile(const std::string& filePath);
    
    /**
     * @brief Check if a folder contains images
     */
    static bool folderContainsImages(const std::string& folderPath);

private:
    ProgressCallback progressCallback_;
    
    void scanSingleFolder(const std::string& folderPath, Core::FolderResult& result);
    std::vector<std::string> getImageFiles(const std::string& folderPath);
};
} // namespace Processing
} // namespace YoloApp