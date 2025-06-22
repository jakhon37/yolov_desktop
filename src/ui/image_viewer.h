
// src/ui/image_viewer.h
#pragma once

#include "../core/types.h"
#include <QWidget>
#include <QScrollArea>
#include <QLabel>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QPushButton>
#include <QSlider>
#include <QComboBox>
#include <QPixmap>
#include <memory>

namespace YoloApp {
namespace UI {

/**
 * @brief Widget for displaying images with detection results
 */
class ImageViewer : public QWidget {
    Q_OBJECT

public:
    explicit ImageViewer(QWidget* parent = nullptr);
    ~ImageViewer() override = default;
    
    /**
     * @brief Display images from a folder result
     */
    void displayFolder(const Core::FolderResult& folderResult);
    
    /**
     * @brief Clear the display
     */
    void clear();

signals:
    void imageClicked(int imageIndex);

private slots:
    void onImageSelectionChanged(int index);
    void onZoomChanged(int value);
    void onToggleAnnotations(bool show);
    void onSaveImage();

private:
    void setupUI();
    void updateImageDisplay();
    void updateMetadata();
    QPixmap matToQPixmap(const cv::Mat& mat);
    cv::Mat scaleImage(const cv::Mat& image, double scale);
    
    QVBoxLayout* mainLayout_;
    QHBoxLayout* controlsLayout_;
    QHBoxLayout* contentLayout_;
    
    // Controls
    QComboBox* imageSelector_;
    QSlider* zoomSlider_;
    QPushButton* toggleAnnotationsButton_;
    QPushButton* saveButton_;
    QLabel* zoomLabel_;
    
    // Display
    QScrollArea* imageScrollArea_;
    QLabel* imageLabel_;
    QTextEdit* metadataText_;
    
    // Data
    Core::FolderResult currentFolder_;
    int currentImageIndex_;
    bool showAnnotations_;
    double currentZoom_;
};
} // namespace UI
} // namespace YoloApp