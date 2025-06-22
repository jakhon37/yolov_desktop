// src/ui/main_window.cpp
#include "main_window.h"
#include "../core/config.h"
#include <QApplication>
#include <QCloseEvent>
#include <QStandardPaths>
#include <QDir>
#include <QTimer>
#include <QInputDialog>
#include <QCheckBox>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QDialog>

namespace YoloApp {
namespace UI {

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , detector_(std::make_shared<Core::YoloDetector>())
    , worker_(nullptr)
    , processingActive_(false) {
    
    // Initialize settings
    QString configPath = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    QDir().mkpath(configPath);
    settings_ = new QSettings(configPath + "/settings.ini", QSettings::IniFormat, this);
    
    setupUI();
    connectSignals();
    loadSettings();
    updateModelStatus();
    updateFolderStatus();
    updateProcessingControls();
}

MainWindow::~MainWindow() {
    if (worker_) {
        worker_->requestCancellation();
        worker_->quit();
        worker_->wait(3000);
    }
    saveSettings();
}

void MainWindow::setupUI() {
    setWindowTitle(QString("%1 v%2").arg(Config::APP_NAME, Config::APP_VERSION));
    setMinimumSize(Config::DEFAULT_WINDOW_WIDTH, Config::DEFAULT_WINDOW_HEIGHT);
    
    centralWidget_ = new QWidget();
    setCentralWidget(centralWidget_);
    mainLayout_ = new QVBoxLayout(centralWidget_);
    
    setupMenuBar();
    setupMainControls();
    setupContentArea();
    setupStatusBar();
}

void MainWindow::setupMenuBar() {
    QMenuBar* menuBar = this->menuBar();
    
    // File menu
    QMenu* fileMenu = menuBar->addMenu("&File");
    
    // FIXED: Use the correct Qt6 addAction syntax
    fileMenu->addAction("&Load Model...", QKeySequence::Open, this, &MainWindow::onLoadModel);
    fileMenu->addAction("&Select Folder...", QKeySequence("Ctrl+F"), this, &MainWindow::onSelectFolder);
    fileMenu->addSeparator();
    fileMenu->addAction("&Export Results...", QKeySequence("Ctrl+E"), this, &MainWindow::onExportResults);
    fileMenu->addSeparator();
    fileMenu->addAction("E&xit", QKeySequence::Quit, this, &MainWindow::onExit);
    
    // Settings menu
    QMenu* settingsMenu = menuBar->addMenu("&Settings");
    settingsMenu->addAction("&Model Settings...", this, &MainWindow::onModelSettings);
    
    // Help menu
    QMenu* helpMenu = menuBar->addMenu("&Help");
    helpMenu->addAction("&About", this, &MainWindow::onAbout);
}

void MainWindow::setupMainControls() {
    QHBoxLayout* controlsLayout = new QHBoxLayout();
    
    // Model controls
    modelGroup_ = new QGroupBox("Model Configuration");
    QVBoxLayout* modelLayout = new QVBoxLayout(modelGroup_);
    
    QHBoxLayout* modelButtonLayout = new QHBoxLayout();
    loadModelButton_ = new QPushButton("Load Model");
    modelSettingsButton_ = new QPushButton("Settings");
    modelSettingsButton_->setEnabled(false);
    
    modelButtonLayout->addWidget(loadModelButton_);
    modelButtonLayout->addWidget(modelSettingsButton_);
    modelButtonLayout->addStretch();
    
    modelPathEdit_ = new QLineEdit();
    modelPathEdit_->setReadOnly(true);
    modelPathEdit_->setPlaceholderText("No model loaded");
    
    modelStatusLabel_ = new QLabel("No model loaded");
    modelStatusLabel_->setStyleSheet(Config::WARNING_COLOR);
    
    modelLayout->addLayout(modelButtonLayout);
    modelLayout->addWidget(modelPathEdit_);
    modelLayout->addWidget(modelStatusLabel_);
    
    // Folder controls
    folderGroup_ = new QGroupBox("Input Folder");
    QVBoxLayout* folderLayout = new QVBoxLayout(folderGroup_);
    
    selectFolderButton_ = new QPushButton("Select Folder");
    folderPathEdit_ = new QLineEdit();
    folderPathEdit_->setReadOnly(true);
    folderPathEdit_->setPlaceholderText("No folder selected");
    
    folderStatusLabel_ = new QLabel("No folder selected");
    
    folderLayout->addWidget(selectFolderButton_);
    folderLayout->addWidget(folderPathEdit_);
    folderLayout->addWidget(folderStatusLabel_);
    
    // Processing controls
    processingGroup_ = new QGroupBox("Processing");
    QVBoxLayout* processingLayout = new QVBoxLayout(processingGroup_);
    
    QHBoxLayout* processingButtonLayout = new QHBoxLayout();
    startButton_ = new QPushButton("Start Detection");
    stopButton_ = new QPushButton("Stop");
    stopButton_->setEnabled(false);
    
    processingButtonLayout->addWidget(startButton_);
    processingButtonLayout->addWidget(stopButton_);
    processingButtonLayout->addStretch();
    
    progressBar_ = new QProgressBar();
    progressBar_->setVisible(false);
    
    progressLabel_ = new QLabel("Ready");
    
    processingLayout->addLayout(processingButtonLayout);
    processingLayout->addWidget(progressBar_);
    processingLayout->addWidget(progressLabel_);
    
    // Add groups to layout
    controlsLayout->addWidget(modelGroup_);
    controlsLayout->addWidget(folderGroup_);
    controlsLayout->addWidget(processingGroup_);
    controlsLayout->addStretch();
    
    mainLayout_->addLayout(controlsLayout);
}

void MainWindow::setupContentArea() {
    mainSplitter_ = new QSplitter(Qt::Horizontal);
    
    // Results widget
    resultsWidget_ = new ResultsWidget();
    resultsWidget_->setMinimumWidth(400);
    
    // Image viewer
    imageViewer_ = new ImageViewer();
    imageViewer_->setMinimumWidth(600);
    
    mainSplitter_->addWidget(resultsWidget_);
    mainSplitter_->addWidget(imageViewer_);
    mainSplitter_->setStretchFactor(0, 1);
    mainSplitter_->setStretchFactor(1, 2);
    
    mainLayout_->addWidget(mainSplitter_);
}

void MainWindow::setupStatusBar() {
    statusLabel_ = new QLabel("Ready");
    statusProgress_ = new QProgressBar();
    statusProgress_->setVisible(false);
    statusProgress_->setMaximumWidth(200);
    
    statusBar()->addWidget(statusLabel_);
    statusBar()->addPermanentWidget(statusProgress_);
}

void MainWindow::connectSignals() {
    // Model controls
    connect(loadModelButton_, &QPushButton::clicked, this, &MainWindow::onLoadModel);
    connect(modelSettingsButton_, &QPushButton::clicked, this, &MainWindow::onModelSettings);
    
    // Folder controls
    connect(selectFolderButton_, &QPushButton::clicked, this, &MainWindow::onSelectFolder);
    
    // Processing controls
    connect(startButton_, &QPushButton::clicked, this, &MainWindow::onStartProcessing);
    connect(stopButton_, &QPushButton::clicked, this, &MainWindow::onStopProcessing);
    
    // Results widget
    connect(resultsWidget_, &ResultsWidget::folderSelected, this, &MainWindow::onFolderSelected);
    connect(resultsWidget_, &ResultsWidget::exportRequested, this, &MainWindow::onExportResults);
    connect(resultsWidget_, &ResultsWidget::refreshRequested, this, &MainWindow::onRefreshResults);
}

// void MainWindow::onLoadModel() {
//     QString modelPath = QFileDialog::getOpenFileName(this,
//         "Select YOLO Model",
//         lastModelPath_.isEmpty() ? QDir::homePath() : QFileInfo(lastModelPath_).dir().path(),
//         "Model Files (*.onnx *.weights *.pt *.pb);;All Files (*)");
    
//     if (modelPath.isEmpty()) {
//         return;
//     }
    
//     // Check for config file for Darknet models
//     QString configPath;
//     if (modelPath.endsWith(".weights", Qt::CaseInsensitive)) {
//         QString configFile = QFileDialog::getOpenFileName(this,
//             "Select Config File (Optional)",
//             QFileInfo(modelPath).dir().path(),
//             "Config Files (*.cfg);;All Files (*)");
//         if (!configFile.isEmpty()) {
//             configPath = configFile;
//         }
//     }
    
//     // Check for custom class names
//     QString classesPath;
//     QMessageBox::StandardButton reply = QMessageBox::question(this,
//         "Class Names", "Do you want to load custom class names?",
//         QMessageBox::Yes | QMessageBox::No);
    
//     if (reply == QMessageBox::Yes) {
//         classesPath = QFileDialog::getOpenFileName(this,
//             "Select Class Names File",
//             QFileInfo(modelPath).dir().path(),
//             "Text Files (*.txt *.names);;All Files (*)");
//     }
    
//     // Load the model
//     bool success = detector_->loadModel(modelPath.toStdString(),
//                                        configPath.toStdString(),
//                                        classesPath.toStdString());
    
//     if (success) {
//         lastModelPath_ = modelPath;
//         modelPathEdit_->setText(modelPath);
//         updateModelStatus();
//         updateProcessingControls();
        
//         QMessageBox::information(this, "Success", "Model loaded successfully!");
//     } else {
//         QMessageBox::critical(this, "Error", "Failed to load model. Please check the file format and try again.");
//     }
// }

void MainWindow::loadModel() {
    QString filter = "Model Files (*.onnx *.weights);;ONNX Files (*.onnx);;Darknet Weights (*.weights);;All Files (*)";
    QString fileName = QFileDialog::getOpenFileName(this, "Open YOLO Model", "", filter);
    
    if (!fileName.isEmpty()) {
        std::string modelPath = fileName.toStdString();
        std::string configPath;
        
        // If it's a weights file, we need a config file too
        if (fileName.endsWith(".weights")) {
            QString cfgFilter = "Config Files (*.cfg);;All Files (*)";
            QString cfgFile = QFileDialog::getOpenFileName(this, "Open Config File", "", cfgFilter);
            if (cfgFile.isEmpty()) {
                QMessageBox::warning(this, "Warning", "Config file is required for .weights files");
                return;
            }
            configPath = cfgFile.toStdString();
        }
        
        if (detector->loadModel(modelPath, configPath)) {
            QMessageBox::information(this, "Success", "Model loaded successfully!");
            
            // Ask for class names file
            QString namesFilter = "Names Files (*.names *.txt);;All Files (*)";
            QString namesFile = QFileDialog::getOpenFileName(this, "Open Class Names File (Optional)", "", namesFilter);
            if (!namesFile.isEmpty()) {
                loadClassNames(namesFile.toStdString());
            }
        } else {
            QMessageBox::critical(this, "Error", "Failed to load model. Please check:\n"
                                                "1. Model file format is supported\n"
                                                "2. For ONNX: Model was exported with static shapes\n"
                                                "3. For Darknet: Both .weights and .cfg files are provided");
        }
    }
}

void MainWindow::onModelSettings() {
    showModelSettingsDialog();
}

void MainWindow::onSelectFolder() {
    QString folderPath = QFileDialog::getExistingDirectory(this,
        "Select Image Folder",
        lastFolderPath_.isEmpty() ? QDir::homePath() : lastFolderPath_);
    
    if (!folderPath.isEmpty()) {
        lastFolderPath_ = folderPath;
        folderPathEdit_->setText(folderPath);
        updateFolderStatus();
        updateProcessingControls();
    }
}

void MainWindow::onStartProcessing() {
    if (!validateConfiguration()) {
        return;
    }
    
    if (worker_) {
        worker_->requestCancellation();
        worker_->quit();
        worker_->wait(3000);
        worker_.reset();
    }
    
    worker_ = std::make_unique<Workers::DetectionWorker>(this);
    
    // Connect worker signals
    connect(worker_.get(), &Workers::DetectionWorker::scanningStarted,
            this, &MainWindow::onScanningStarted);
    connect(worker_.get(), &Workers::DetectionWorker::folderScanned,
            this, &MainWindow::onFolderScanned);
    connect(worker_.get(), &Workers::DetectionWorker::processingStarted,
            this, &MainWindow::onProcessingStarted);
    connect(worker_.get(), &Workers::DetectionWorker::imageProcessed,
            this, &MainWindow::onImageProcessed);
    connect(worker_.get(), &Workers::DetectionWorker::folderCompleted,
            this, &MainWindow::onFolderCompleted);
    connect(worker_.get(), &Workers::DetectionWorker::processingCompleted,
            this, &MainWindow::onProcessingCompleted);
    connect(worker_.get(), &Workers::DetectionWorker::errorOccurred,
            this, &MainWindow::onProcessingError);
    
    // Clear previous results
    resultsWidget_->clearResults();
    imageViewer_->clear();
    
    // Start processing
    worker_->startProcessing(lastFolderPath_.toStdString(), detector_, true);
    
    processingActive_ = true;
    updateProcessingControls();
}

void MainWindow::onStopProcessing() {
    if (worker_) {
        worker_->requestCancellation();
        progressLabel_->setText("Stopping...");
    }
}

void MainWindow::onScanningStarted(int totalFolders) {
    progressBar_->setVisible(true);
    progressBar_->setMaximum(totalFolders);
    progressBar_->setValue(0);
    progressLabel_->setText(QString("Scanning folders... (0/%1)").arg(totalFolders));
    
    statusProgress_->setVisible(true);
    statusProgress_->setMaximum(totalFolders);
    statusProgress_->setValue(0);
    statusLabel_->setText("Scanning for images...");
}

void MainWindow::onFolderScanned(QString folderName, int imageCount) {
    Q_UNUSED(imageCount)
    int current = progressBar_->value() + 1;
    progressBar_->setValue(current);
    progressLabel_->setText(QString("Scanning folders... (%1/%2)")
                           .arg(current).arg(progressBar_->maximum()));
    
    statusProgress_->setValue(current);
    statusLabel_->setText(QString("Scanned: %1").arg(folderName));
}

void MainWindow::onProcessingStarted(int totalImages) {
    progressBar_->setMaximum(totalImages);
    progressBar_->setValue(0);
    progressLabel_->setText(QString("Processing images... (0/%1)").arg(totalImages));
    
    statusProgress_->setMaximum(totalImages);
    statusProgress_->setValue(0);
    statusLabel_->setText("Processing images...");
}

void MainWindow::onImageProcessed(QString imagePath, int detectionCount) {
    Q_UNUSED(imagePath)
    Q_UNUSED(detectionCount)
    int current = progressBar_->value() + 1;
    progressBar_->setValue(current);
    progressLabel_->setText(QString("Processing images... (%1/%2)")
                           .arg(current).arg(progressBar_->maximum()));
    
    statusProgress_->setValue(current);
}

void MainWindow::onFolderCompleted(QString folderName, int totalDetections) {
    // Update results table with latest data
    if (worker_) {
        resultsWidget_->updateResults(worker_->getResults());
    }
    
    statusLabel_->setText(QString("Completed: %1 (%2 detections)")
                         .arg(folderName).arg(totalDetections));
}

void MainWindow::onProcessingCompleted(Core::ProcessingStats stats) {
    processingActive_ = false;
    updateProcessingControls();
    
    progressBar_->setVisible(false);
    statusProgress_->setVisible(false);
    
    QString summary = QString("Completed: %1 folders, %2 images, %3 detections in %4 seconds")
                     .arg(stats.processedFolders)
                     .arg(stats.processedImages)
                     .arg(stats.totalDetections)
                     .arg(stats.getElapsedSeconds(), 0, 'f', 1);
    
    progressLabel_->setText(summary);
    statusLabel_->setText("Processing completed");
    
    // Final update of results
    if (worker_) {
        resultsWidget_->updateResults(worker_->getResults());
    }
    
    QMessageBox::information(this, "Processing Complete", summary);
}

void MainWindow::onProcessingError(QString error) {
    QMessageBox::warning(this, "Processing Error", error);
    statusLabel_->setText("Error occurred during processing");
}

void MainWindow::onFolderSelected(int folderIndex) {
    if (worker_) {
        auto results = worker_->getResults();
        if (folderIndex >= 0 && folderIndex < static_cast<int>(results.size())) {
            imageViewer_->displayFolder(results[folderIndex]);
        }
    }
}

void MainWindow::onExportResults() {
    // Implementation for exporting results to CSV/JSON
    QMessageBox::information(this, "Export", "Export functionality to be implemented");
}

void MainWindow::onRefreshResults() {
    if (worker_) {
        resultsWidget_->updateResults(worker_->getResults());
    }
}

void MainWindow::onAbout() {
    QString aboutText = QString(
        "<h3>%1 v%2</h3>"
        "<p>A desktop application for object detection using YOLO models.</p>"
        "<p>Built with Qt and OpenCV</p>"
        "<p>Features:</p>"
        "<ul>"
        "<li>Support for ONNX, Darknet, and TensorFlow models</li>"
        "<li>Batch processing of image folders</li>"
        "<li>Real-time detection visualization</li>"
        "<li>Detailed metadata and statistics</li>"
        "</ul>"
    ).arg(Config::APP_NAME, Config::APP_VERSION);
    
    QMessageBox::about(this, "About", aboutText);
}

void MainWindow::onExit() {
    close();
}

void MainWindow::closeEvent(QCloseEvent* event) {
    if (processingActive_) {
        QMessageBox::StandardButton reply = QMessageBox::question(this,
            "Exit", "Processing is in progress. Do you want to stop and exit?",
            QMessageBox::Yes | QMessageBox::No);
        
        if (reply == QMessageBox::No) {
            event->ignore();
            return;
        }
        
        if (worker_) {
            worker_->requestCancellation();
        }
    }
    
    saveSettings();
    event->accept();
}

void MainWindow::loadSettings() {
    // Window geometry
    restoreGeometry(settings_->value("geometry").toByteArray());
    restoreState(settings_->value("windowState").toByteArray());
    
    // Paths
    lastModelPath_ = settings_->value("lastModelPath").toString();
    lastFolderPath_ = settings_->value("lastFolderPath").toString();
    
    // Detection configuration
    detectionConfig_.confidenceThreshold = settings_->value("confidenceThreshold", 0.5f).toFloat();
    detectionConfig_.nmsThreshold = settings_->value("nmsThreshold", 0.4f).toFloat();
    detectionConfig_.inputWidth = settings_->value("inputWidth", 640).toInt();
    detectionConfig_.inputHeight = settings_->value("inputHeight", 640).toInt();
    
    // Update UI
    if (!lastModelPath_.isEmpty()) {
        modelPathEdit_->setText(lastModelPath_);
    }
    if (!lastFolderPath_.isEmpty()) {
        folderPathEdit_->setText(lastFolderPath_);
    }
    
    detector_->setConfig(detectionConfig_);
}

void MainWindow::saveSettings() {
    // Window geometry
    settings_->setValue("geometry", saveGeometry());
    settings_->setValue("windowState", saveState());
    
    // Paths
    settings_->setValue("lastModelPath", lastModelPath_);
    settings_->setValue("lastFolderPath", lastFolderPath_);
    
    // Detection configuration
    settings_->setValue("confidenceThreshold", detectionConfig_.confidenceThreshold);
    settings_->setValue("nmsThreshold", detectionConfig_.nmsThreshold);
    settings_->setValue("inputWidth", detectionConfig_.inputWidth);
    settings_->setValue("inputHeight", detectionConfig_.inputHeight);
}

void MainWindow::updateModelStatus() {
    if (detector_->isLoaded()) {
        modelStatusLabel_->setText("Model loaded and ready");
        modelStatusLabel_->setStyleSheet(Config::SUCCESS_COLOR);
        modelSettingsButton_->setEnabled(true);
    } else {
        modelStatusLabel_->setText("No model loaded");
        modelStatusLabel_->setStyleSheet(Config::WARNING_COLOR);
        modelSettingsButton_->setEnabled(false);
    }
}

void MainWindow::updateFolderStatus() {
    if (!lastFolderPath_.isEmpty()) {
        QDir dir(lastFolderPath_);
        if (dir.exists()) {
            folderStatusLabel_->setText("Folder selected and accessible");
        } else {
            folderStatusLabel_->setText("Selected folder does not exist");
        }
    } else {
        folderStatusLabel_->setText("No folder selected");
    }
}

void MainWindow::updateProcessingControls() {
    bool canStart = detector_->isLoaded() && !lastFolderPath_.isEmpty() && !processingActive_;
    
    startButton_->setEnabled(canStart);
    stopButton_->setEnabled(processingActive_);
    loadModelButton_->setEnabled(!processingActive_);
    selectFolderButton_->setEnabled(!processingActive_);
}

bool MainWindow::validateConfiguration() {
    if (!detector_->isLoaded()) {
        QMessageBox::warning(this, "Configuration Error", "No model loaded. Please load a YOLO model first.");
        return false;
    }
    
    if (lastFolderPath_.isEmpty()) {
        QMessageBox::warning(this, "Configuration Error", "No folder selected. Please select an input folder.");
        return false;
    }
    
    QDir dir(lastFolderPath_);
    if (!dir.exists()) {
        QMessageBox::warning(this, "Configuration Error", "Selected folder does not exist.");
        return false;
    }
    
    return true;
}

void MainWindow::showModelSettingsDialog() {
    QDialog dialog(this);
    dialog.setWindowTitle("Model Settings");
    dialog.setModal(true);
    
    QFormLayout* layout = new QFormLayout(&dialog);
    
    // Confidence threshold
    QDoubleSpinBox* confidenceSpinBox = new QDoubleSpinBox();
    confidenceSpinBox->setRange(0.1, 1.0);
    confidenceSpinBox->setSingleStep(0.1);
    confidenceSpinBox->setDecimals(2);
    confidenceSpinBox->setValue(detectionConfig_.confidenceThreshold);
    layout->addRow("Confidence Threshold:", confidenceSpinBox);
    
    // NMS threshold
    QDoubleSpinBox* nmsSpinBox = new QDoubleSpinBox();
    nmsSpinBox->setRange(0.1, 1.0);
    nmsSpinBox->setSingleStep(0.1);
    nmsSpinBox->setDecimals(2);
    nmsSpinBox->setValue(detectionConfig_.nmsThreshold);
    layout->addRow("NMS Threshold:", nmsSpinBox);
    
    // Input dimensions
    QSpinBox* widthSpinBox = new QSpinBox();
    widthSpinBox->setRange(128, 1280);
    widthSpinBox->setSingleStep(32);
    widthSpinBox->setValue(detectionConfig_.inputWidth);
    layout->addRow("Input Width:", widthSpinBox);
    
    QSpinBox* heightSpinBox = new QSpinBox();
    heightSpinBox->setRange(128, 1280);
    heightSpinBox->setSingleStep(32);
    heightSpinBox->setValue(detectionConfig_.inputHeight);
    layout->addRow("Input Height:", heightSpinBox);
    
    // Dialog buttons
    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    layout->addRow(buttonBox);
    
    if (dialog.exec() == QDialog::Accepted) {
        detectionConfig_.confidenceThreshold = static_cast<float>(confidenceSpinBox->value());
        detectionConfig_.nmsThreshold = static_cast<float>(nmsSpinBox->value());
        detectionConfig_.inputWidth = widthSpinBox->value();
        detectionConfig_.inputHeight = heightSpinBox->value();
        
        detector_->setConfig(detectionConfig_);
    }
}

} // namespace UI
} // namespace YoloApp

// #include "main_window.moc"