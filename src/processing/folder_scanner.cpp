
// src/processing/folder_scanner.cpp
#include "folder_scanner.h"
#include "../core/config.h"
#include <filesystem>
#include <algorithm>

namespace fs = std::filesystem;

namespace YoloApp {
namespace Processing {

std::vector<Core::FolderResult> FolderScanner::scanForImages(const std::string& rootPath, 
                                                            bool recursive) {
    std::vector<Core::FolderResult> results;
    
    if (!fs::exists(rootPath) || !fs::is_directory(rootPath)) {
        return results;
    }
    
    std::vector<std::string> foldersToScan;
    
    // Check if root folder itself contains images
    if (folderContainsImages(rootPath)) {
        foldersToScan.push_back(rootPath);
    }
    
    // If recursive, scan subdirectories
    if (recursive) {
        try {
            for (const auto& entry : fs::recursive_directory_iterator(rootPath)) {
                if (entry.is_directory() && folderContainsImages(entry.path().string())) {
                    // Avoid adding the root path twice
                    if (entry.path().string() != rootPath) {
                        foldersToScan.push_back(entry.path().string());
                    }
                }
            }
        } catch (const fs::filesystem_error&) {
            // Handle permission errors gracefully
        }
    }
    
    // Process each folder
    int totalFolders = static_cast<int>(foldersToScan.size());
    for (int i = 0; i < totalFolders; ++i) {
        const std::string& folderPath = foldersToScan[i];
        
        if (progressCallback_) {
            progressCallback_(i, totalFolders, folderPath);
        }
        
        Core::FolderResult folderResult(folderPath);
        scanSingleFolder(folderPath, folderResult);
        
        if (!folderResult.images.empty()) {
            results.push_back(std::move(folderResult));
        }
    }
    
    if (progressCallback_) {
        progressCallback_(totalFolders, totalFolders, "");
    }
    
    return results;
}

void FolderScanner::setProgressCallback(ProgressCallback callback) {
    progressCallback_ = callback;
}

bool FolderScanner::isImageFile(const std::string& filePath) {
    std::string extension = fs::path(filePath).extension().string();
    std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
    
    return std::find(Config::SUPPORTED_EXTENSIONS.begin(), 
                    Config::SUPPORTED_EXTENSIONS.end(), 
                    extension) != Config::SUPPORTED_EXTENSIONS.end();
}

bool FolderScanner::folderContainsImages(const std::string& folderPath) {
    try {
        for (const auto& entry : fs::directory_iterator(folderPath)) {
            if (entry.is_regular_file() && isImageFile(entry.path().string())) {
                return true;
            }
        }
    } catch (const fs::filesystem_error&) {
        return false;
    }
    return false;
}

void FolderScanner::scanSingleFolder(const std::string& folderPath, Core::FolderResult& result) {
    std::vector<std::string> imageFiles = getImageFiles(folderPath);
    
    result.images.reserve(imageFiles.size());
    for (const std::string& imagePath : imageFiles) {
        auto imageResult = std::make_shared<Core::ImageResult>(imagePath);
        result.images.push_back(imageResult);
    }
    
    result.updateCounts();
}

std::vector<std::string> FolderScanner::getImageFiles(const std::string& folderPath) {
    std::vector<std::string> imageFiles;
    
    try {
        for (const auto& entry : fs::directory_iterator(folderPath)) {
            if (entry.is_regular_file() && isImageFile(entry.path().string())) {
                imageFiles.push_back(entry.path().string());
            }
        }
        
        // Sort files for consistent ordering
        std::sort(imageFiles.begin(), imageFiles.end());
        
    } catch (const fs::filesystem_error&) {
        // Handle permission errors gracefully
    }
    
    return imageFiles;
}

} // namespace Processing
} // namespace YoloApp