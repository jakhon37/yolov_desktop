
// src/ui/image_viewer.cpp
#include "image_viewer.h"
#include "../processing/image_processor.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QPixmap>
#include <QApplication>

namespace YoloApp {
namespace UI {

ImageViewer::ImageViewer(QWidget* parent)
    : QWidget(parent)
    , currentImageIndex_(-1)
    , showAnnotations_(true)
    , currentZoom_(1.0) {
    setupUI();
}

void ImageViewer::setupUI() {
    mainLayout_ = new QVBoxLayout(this);
    
    // Controls
    controlsLayout_ = new QHBoxLayout();
    
    imageSelector_ = new QComboBox();
    imageSelector_->setMinimumWidth(200);
    
    zoomSlider_ = new QSlider(Qt::Horizontal);
    zoomSlider_->setRange(10, 500);  // 10% to 500%
    zoomSlider_->setValue(100);      // 100%
    zoomSlider_->setMinimumWidth(150);
    
    zoomLabel_ = new QLabel("100%");
    zoomLabel_->setMinimumWidth(50);
    
    toggleAnnotationsButton_ = new QPushButton("Hide Annotations");
    saveButton_ = new QPushButton("Save Image");
    
    controlsLayout_->addWidget(new QLabel("Image:"));
    controlsLayout_->addWidget(imageSelector_);
    controlsLayout_->addStretch();
    controlsLayout_->addWidget(new QLabel("Zoom:"));
    controlsLayout_->addWidget(zoomSlider_);
    controlsLayout_->addWidget(zoomLabel_);
    controlsLayout_->addWidget(toggleAnnotationsButton_);
    controlsLayout_->addWidget(saveButton_);
    
    // Content
    contentLayout_ = new QHBoxLayout();
    
    // Image display
    imageScrollArea_ = new QScrollArea();
    imageLabel_ = new QLabel();
    imageLabel_->setAlignment(Qt::AlignCenter);
    imageLabel_->setMinimumSize(400, 300);
    imageLabel_->setText("Select a folder to view images");
    imageLabel_->setStyleSheet("border: 1px solid gray; background-color: #f0f0f0;");
    
    imageScrollArea_->setWidget(imageLabel_);
    imageScrollArea_->setWidgetResizable(false);
    
    // Metadata
    metadataText_ = new QTextEdit();
    metadataText_->setReadOnly(true);
    metadataText_->setMaximumWidth(300);
    metadataText_->setMinimumWidth(250);
    
    contentLayout_->addWidget(imageScrollArea_, 3);
    contentLayout_->addWidget(metadataText_, 1);
    
    mainLayout_->addLayout(controlsLayout_);
    mainLayout_->addLayout(contentLayout_);
    
    // Connect signals
    connect(imageSelector_, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ImageViewer::onImageSelectionChanged);
    connect(zoomSlider_, &QSlider::valueChanged, this, &ImageViewer::onZoomChanged);
    connect(toggleAnnotationsButton_, &QPushButton::clicked, this, &ImageViewer::onToggleAnnotations);
    connect(saveButton_, &QPushButton::clicked, this, &ImageViewer::onSaveImage);
    
    // Initial state
    imageSelector_->setEnabled(false);
    saveButton_->setEnabled(false);
}

void ImageViewer::displayFolder(const Core::FolderResult& folderResult) {
    currentFolder_ = folderResult;
    currentImageIndex_ = -1;
    
    // Populate image selector
    imageSelector_->clear();
    for (int i = 0; i < static_cast<int>(folderResult.images.size()); ++i) {
        const auto& img = folderResult.images[i];
        QString itemText = QString("Image %1").arg(i + 1);
        if (img && img->processed) {
            itemText += QString(" (%1 detections)").arg(img->getDetectionCount());
        }
        imageSelector_->addItem(itemText);
    }
    
    imageSelector_->setEnabled(!folderResult.images.empty());
    
    // Select first image if available
    if (!folderResult.images.empty()) {
        imageSelector_->setCurrentIndex(0);
        onImageSelectionChanged(0);
    } else {
        clear();
    }
}

void ImageViewer::clear() {
    imageLabel_->clear();
    imageLabel_->setText("Select a folder to view images");
    metadataText_->clear();
    imageSelector_->clear();
    imageSelector_->setEnabled(false);
    saveButton_->setEnabled(false);
    currentImageIndex_ = -1;
}

void ImageViewer::onImageSelectionChanged(int index) {
    if (index < 0 || index >= static_cast<int>(currentFolder_.images.size())) {
        return;
    }
    
    currentImageIndex_ = index;
    updateImageDisplay();
    updateMetadata();
    saveButton_->setEnabled(true);
    
    emit imageClicked(index);
}

void ImageViewer::onZoomChanged(int value) {
    currentZoom_ = value / 100.0;
    zoomLabel_->setText(QString("%1%").arg(value));
    updateImageDisplay();
}

void ImageViewer::onToggleAnnotations(bool show) {
    Q_UNUSED(show)
    showAnnotations_ = !showAnnotations_;
    toggleAnnotationsButton_->setText(showAnnotations_ ? "Hide Annotations" : "Show Annotations");
    updateImageDisplay();
}

void ImageViewer::onSaveImage() {
    if (currentImageIndex_ < 0 || currentImageIndex_ >= static_cast<int>(currentFolder_.images.size())) {
        return;
    }
    
    const auto& imageResult = currentFolder_.images[currentImageIndex_];
    if (!imageResult || !imageResult->processed) {
        QMessageBox::warning(this, "Warning", "Image not yet processed.");
        return;
    }
    
    QString defaultName = QString("detection_result_%1.jpg").arg(currentImageIndex_ + 1);
    QString fileName = QFileDialog::getSaveFileName(this, "Save Image", defaultName,
                                                   "JPEG Files (*.jpg);;PNG Files (*.png);;All Files (*)");
    
    if (!fileName.isEmpty()) {
        cv::Mat imageToSave = showAnnotations_ ? imageResult->annotatedImage : imageResult->originalImage;
        if (!imageToSave.empty()) {
            if (cv::imwrite(fileName.toStdString(), imageToSave)) {
                QMessageBox::information(this, "Success", "Image saved successfully.");
            } else {
                QMessageBox::warning(this, "Error", "Failed to save image.");
            }
        }
    }
}

void ImageViewer::updateImageDisplay() {
    if (currentImageIndex_ < 0 || currentImageIndex_ >= static_cast<int>(currentFolder_.images.size())) {
        return;
    }
    
    const auto& imageResult = currentFolder_.images[currentImageIndex_];
    if (!imageResult || !imageResult->processed) {
        imageLabel_->setText("Image processing...");
        return;
    }
    
    cv::Mat displayImage = showAnnotations_ ? imageResult->annotatedImage : imageResult->originalImage;
    
    if (displayImage.empty()) {
        imageLabel_->setText("Failed to load image");
        return;
    }
    
    // Apply zoom
    if (currentZoom_ != 1.0) {
        displayImage = scaleImage(displayImage, currentZoom_);
    }
    
    QPixmap pixmap = matToQPixmap(displayImage);
    imageLabel_->setPixmap(pixmap);
    imageLabel_->resize(pixmap.size());
}

void ImageViewer::updateMetadata() {
    if (currentImageIndex_ < 0 || currentImageIndex_ >= static_cast<int>(currentFolder_.images.size())) {
        metadataText_->clear();
        return;
    }
    
    const auto& imageResult = currentFolder_.images[currentImageIndex_];
    if (!imageResult) {
        metadataText_->setText("No image data available");
        return;
    }
    
    if (!imageResult->processed) {
        metadataText_->setText("Processing image...");
        return;
    }
    
    metadataText_->setText(QString::fromStdString(imageResult->metadata));
}

QPixmap ImageViewer::matToQPixmap(const cv::Mat& mat) {
    if (mat.empty()) {
        return QPixmap();
    }
    
    QImage qimg;
    
    if (mat.type() == CV_8UC1) {
        qimg = QImage(mat.data, mat.cols, mat.rows, mat.step, QImage::Format_Grayscale8);
    } else if (mat.type() == CV_8UC3) {
        cv::Mat rgbMat;
        cv::cvtColor(mat, rgbMat, cv::COLOR_BGR2RGB);
        qimg = QImage(rgbMat.data, rgbMat.cols, rgbMat.rows, rgbMat.step, QImage::Format_RGB888);
    } else if (mat.type() == CV_8UC4) {
        cv::Mat rgbaMat;
        cv::cvtColor(mat, rgbaMat, cv::COLOR_BGRA2RGBA);
        qimg = QImage(rgbaMat.data, rgbaMat.cols, rgbaMat.rows, rgbaMat.step, QImage::Format_RGBA8888);
    } else {
        return QPixmap();
    }
    
    return QPixmap::fromImage(qimg);
}

cv::Mat ImageViewer::scaleImage(const cv::Mat& image, double scale) {
    if (scale == 1.0) {
        return image;
    }
    
    cv::Mat scaled;
    cv::Size newSize(static_cast<int>(image.cols * scale), static_cast<int>(image.rows * scale));
    cv::resize(image, scaled, newSize, 0, 0, cv::INTER_LINEAR);
    return scaled;
}

} // namespace UI
} // namespace YoloApp