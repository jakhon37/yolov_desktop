
// src/workers/detection_worker.cpp
#include "detection_worker.h"
#include <QMutexLocker>
#include <chrono>

namespace YoloApp {
namespace Workers {

DetectionWorker::DetectionWorker(QObject* parent)
    : QThread(parent)
    , recursive_(true)
    , cancellationRequested_(false)
    , processing_(false) {
}

DetectionWorker::~DetectionWorker() {
    requestCancellation();
    if (isRunning()) {
        quit();
        wait(5000); // Wait up to 5 seconds
        if (isRunning()) {
            terminate();
            wait(1000);
        }
    }
}

void DetectionWorker::startProcessing(const std::string& rootPath, 
                                     std::shared_ptr<Core::IDetector> detector,
                                     bool recursive) {
    if (isRunning()) {
        return; // Already processing
    }
    
    rootPath_ = rootPath;
    detector_ = detector;
    recursive_ = recursive;
    cancellationRequested_ = false;
    processing_ = true;
    
    {
        QMutexLocker locker(&resultsMutex_);
        results_.clear();
        stats_ = Core::ProcessingStats();
    }
    
    start();
}

void DetectionWorker::requestCancellation() {
    cancellationRequested_ = true;
}

bool DetectionWorker::isProcessing() const {
    return processing_;
}

Core::ProcessingStats DetectionWorker::getStats() const {
    QMutexLocker locker(&resultsMutex_);
    return stats_;
}

std::vector<Core::FolderResult> DetectionWorker::getResults() const {
    QMutexLocker locker(&resultsMutex_);
    return results_;
}

void DetectionWorker::run() {
    try {
        stats_.start();
        performScanning();
        
        if (!cancellationRequested_) {
            performProcessing();
        }
        
        stats_.finish();
        emit processingCompleted(stats_);
        
    } catch (const std::exception& e) {
        emit errorOccurred(QString::fromStdString(e.what()));
    }
    
    processing_ = false;
}

void DetectionWorker::performScanning() {
    if (!detector_ || !detector_->isLoaded()) {
        throw std::runtime_error("Detector not loaded");
    }
    
    Processing::FolderScanner scanner;
    
    // Set up progress callback
    scanner.setProgressCallback([this](int current, int total, const std::string& currentPath) {
        if (cancellationRequested_) return;
        
        if (current == 0 && total > 0) {
            emit scanningStarted(total);
        }
        
        if (!currentPath.empty()) {
            emit folderScanned(QString::fromStdString(currentPath), 0);
        }
    });
    
    // Scan for folders containing images
    std::vector<Core::FolderResult> scannedResults = scanner.scanForImages(rootPath_, recursive_);
    
    if (cancellationRequested_) {
        return;
    }
    
    // Update results and stats
    {
        QMutexLocker locker(&resultsMutex_);
        results_ = std::move(scannedResults);
        stats_.totalFolders = static_cast<int>(results_.size());
        
        // Count total images
        for (const auto& folder : results_) {
            stats_.totalImages += folder.imageCount;
        }
    }
    
    emit processingStarted(stats_.totalImages);
}

void DetectionWorker::performProcessing() {
    QMutexLocker locker(&resultsMutex_);
    
    for (auto& folderResult : results_) {
        if (cancellationRequested_) {
            break;
        }
        
        // Process each image in the folder
        for (auto& imageResult : folderResult.images) {
            if (cancellationRequested_) {
                break;
            }
            
            // Unlock mutex during processing to allow UI updates
            locker.unlock();
            
            try {
                Processing::ImageProcessor::processImageResult(imageResult, *detector_);
                emit imageProcessed(QString::fromStdString(imageResult->imagePath), 
                                  imageResult->getDetectionCount());
                
            } catch (const std::exception& e) {
                // Log error but continue processing
                emit errorOccurred(QString("Error processing %1: %2")
                                  .arg(QString::fromStdString(imageResult->imagePath))
                                  .arg(QString::fromStdString(e.what())));
            }
            
            // Relock and update stats
            locker.relock();
            stats_.processedImages++;
            stats_.totalDetections += imageResult->getDetectionCount();
        }
        
        // Update folder statistics
        folderResult.updateCounts();
        folderResult.processed = true;
        stats_.processedFolders++;
        
        emit folderCompleted(QString::fromStdString(folderResult.folderName), 
                           folderResult.totalDetections);
    }
}

} // namespace Workers
} // namespace YoloApp

// #include "detection_worker.moc"