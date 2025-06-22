// src/workers/detection_worker.h
#pragma once

#include "../core/types.h"
#include "../core/detector.h"
#include "../processing/folder_scanner.h"
#include "../processing/image_processor.h"
#include <QThread>
#include <QMutex>
#include <memory>
#include <atomic>

namespace YoloApp {
namespace Workers {

/**
 * @brief Background worker for processing image detection
 */
class DetectionWorker : public QThread {
    Q_OBJECT

public:
    explicit DetectionWorker(QObject* parent = nullptr);
    ~DetectionWorker() override;
    
    /**
     * @brief Start processing images in the specified folder
     */
    void startProcessing(const std::string& rootPath, 
                        std::shared_ptr<Core::IDetector> detector,
                        bool recursive = true);
    
    /**
     * @brief Request cancellation of current processing
     */
    void requestCancellation();
    
    /**
     * @brief Check if worker is currently processing
     */
    bool isProcessing() const;
    
    /**
     * @brief Get current processing statistics
     */
    Core::ProcessingStats getStats() const;
    
    /**
     * @brief Get results (thread-safe)
     */
    std::vector<Core::FolderResult> getResults() const;

signals:
    void scanningStarted(int totalFolders);
    void folderScanned(QString folderName, int imageCount);
    void processingStarted(int totalImages);
    void imageProcessed(QString imagePath, int detectionCount);
    void folderCompleted(QString folderName, int totalDetections);
    void processingCompleted(Core::ProcessingStats stats);
    void errorOccurred(QString error);

protected:
    void run() override;

private:
    std::string rootPath_;
    std::shared_ptr<Core::IDetector> detector_;
    bool recursive_;
    std::atomic<bool> cancellationRequested_;
    std::atomic<bool> processing_;
    
    mutable QMutex resultsMutex_;
    std::vector<Core::FolderResult> results_;
    Core::ProcessingStats stats_;
    
    void performScanning();
    void performProcessing();
    void updateStats();
};

} // namespace Workers
} // namespace YoloApp