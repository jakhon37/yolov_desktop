
// src/ui/main_window.h
#pragma once

#include "../core/detector.h"
#include "../workers/detection_worker.h"
#include "results_widget.h"
#include "image_viewer.h"
#include <QMainWindow>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QSplitter>
#include <QGroupBox>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QProgressBar>
#include <QStatusBar>
#include <QMenuBar>
#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>
#include <memory>

namespace YoloApp {
namespace UI {

/**
 * @brief Main application window
 */
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

protected:
    void closeEvent(QCloseEvent* event) override;

private slots:
    // Model management
    void onLoadModel();
    void onModelSettings();
    
    // Folder operations
    void onSelectFolder();
    void onStartProcessing();
    void onStopProcessing();
    
    // Worker signals
    void onScanningStarted(int totalFolders);
    void onFolderScanned(QString folderName, int imageCount);
    void onProcessingStarted(int totalImages);
    void onImageProcessed(QString imagePath, int detectionCount);
    void onFolderCompleted(QString folderName, int totalDetections);
    void onProcessingCompleted(Core::ProcessingStats stats);
    void onProcessingError(QString error);
    
    // UI interactions
    void onFolderSelected(int folderIndex);
    void onExportResults();
    void onRefreshResults();
    
    // Menu actions
    void onAbout();
    void onExit();

private:
    void setupUI();
    void setupMenuBar();
    void setupMainControls();
    void setupContentArea();
    void setupStatusBar();
    void connectSignals();
    
    void loadSettings();
    void saveSettings();
    
    void updateModelStatus();
    void updateFolderStatus();
    void updateProcessingControls();
    
    bool validateConfiguration();
    void showModelSettingsDialog();
    
    // Core components
    std::shared_ptr<Core::YoloDetector> detector_;
    std::unique_ptr<Workers::DetectionWorker> worker_;
    
    // UI components
    QWidget* centralWidget_;
    QVBoxLayout* mainLayout_;
    QSplitter* mainSplitter_;
    
    // Controls
    QGroupBox* modelGroup_;
    QGroupBox* folderGroup_;
    QGroupBox* processingGroup_;
    
    QPushButton* loadModelButton_;
    QPushButton* modelSettingsButton_;
    QLabel* modelStatusLabel_;
    QLineEdit* modelPathEdit_;
    
    QPushButton* selectFolderButton_;
    QLabel* folderStatusLabel_;
    QLineEdit* folderPathEdit_;
    
    QPushButton* startButton_;
    QPushButton* stopButton_;
    QProgressBar* progressBar_;
    QLabel* progressLabel_;
    
    // Content area
    ResultsWidget* resultsWidget_;
    ImageViewer* imageViewer_;
    
    // Status
    QLabel* statusLabel_;
    QProgressBar* statusProgress_;
    
    // Settings
    QSettings* settings_;
    QString lastModelPath_;
    QString lastFolderPath_;
    Core::DetectionConfig detectionConfig_;
    
    // State
    bool processingActive_;
};

} // namespace UI
} // namespace YoloApp