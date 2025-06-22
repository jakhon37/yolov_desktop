// src/ui/results_widget.cpp
#include "results_widget.h"
#include "../core/config.h"
#include <QHeaderView>
#include <QAbstractItemView>

namespace YoloApp {
namespace UI {

ResultsWidget::ResultsWidget(QWidget* parent)
    : QWidget(parent)
    , selectedFolderIndex_(-1) {
    setupUI();
}

void ResultsWidget::setupUI() {
    mainLayout_ = new QVBoxLayout(this);
    
    // Top controls
    topLayout_ = new QHBoxLayout();
    summaryLabel_ = new QLabel("No results");
    summaryLabel_->setStyleSheet("font-weight: bold;");
    
    topLayout_->addWidget(summaryLabel_);
    topLayout_->addStretch();
    
    // Bottom controls
    bottomLayout_ = new QHBoxLayout();
    exportButton_ = new QPushButton("Export Results");
    refreshButton_ = new QPushButton("Refresh");
    progressBar_ = new QProgressBar();
    progressBar_->setVisible(false);
    
    bottomLayout_->addWidget(exportButton_);
    bottomLayout_->addWidget(refreshButton_);
    bottomLayout_->addStretch();
    bottomLayout_->addWidget(progressBar_);
    
    // Results table
    setupTable();
    
    mainLayout_->addLayout(topLayout_);
    mainLayout_->addWidget(resultsTable_);
    mainLayout_->addLayout(bottomLayout_);
    
    connect(exportButton_, &QPushButton::clicked, this, &ResultsWidget::onExportClicked);
    connect(refreshButton_, &QPushButton::clicked, this, &ResultsWidget::onRefreshClicked);
}

void ResultsWidget::setupTable() {
    resultsTable_ = new QTableWidget();
    resultsTable_->setColumnCount(4);
    
    QStringList headers = {"Folder", "Images", "Detections", "Status"};
    resultsTable_->setHorizontalHeaderLabels(headers);
    
    // Configure table
    resultsTable_->setSelectionBehavior(QAbstractItemView::SelectRows);
    resultsTable_->setAlternatingRowColors(true);
    resultsTable_->setSortingEnabled(true);
    
    // Set column widths
    QHeaderView* header = resultsTable_->horizontalHeader();
    header->setStretchLastSection(true);
    header->resizeSection(0, 200);  // Folder name
    header->resizeSection(1, 80);   // Image count
    header->resizeSection(2, 100);  // Detection count
    
    connect(resultsTable_, &QTableWidget::itemSelectionChanged, 
            this, &ResultsWidget::onTableSelectionChanged);
}

void ResultsWidget::updateResults(const std::vector<Core::FolderResult>& results) {
    results_ = results;
    
    resultsTable_->setRowCount(static_cast<int>(results.size()));
    
    for (int i = 0; i < static_cast<int>(results.size()); ++i) {
        const auto& folder = results[i];
        
        resultsTable_->setItem(i, 0, new QTableWidgetItem(QString::fromStdString(folder.folderName)));
        resultsTable_->setItem(i, 1, new QTableWidgetItem(QString::number(folder.imageCount)));
        resultsTable_->setItem(i, 2, new QTableWidgetItem(QString::number(folder.totalDetections)));
        resultsTable_->setItem(i, 3, new QTableWidgetItem(folder.processed ? "Completed" : "Processing..."));
        
        // Set row data for easy access
        resultsTable_->item(i, 0)->setData(Qt::UserRole, i);
    }
    
    updateSummary();
}

void ResultsWidget::clearResults() {
    results_.clear();
    resultsTable_->setRowCount(0);
    selectedFolderIndex_ = -1;
    updateSummary();
}

void ResultsWidget::setProgress(int current, int total) {
    if (total > 0) {
        progressBar_->setVisible(true);
        progressBar_->setMaximum(total);
        progressBar_->setValue(current);
    } else {
        progressBar_->setVisible(false);
    }
}

int ResultsWidget::getSelectedFolderIndex() const {
    return selectedFolderIndex_;
}

void ResultsWidget::onTableSelectionChanged() {
    QList<QTableWidgetItem*> selected = resultsTable_->selectedItems();
    if (!selected.isEmpty()) {
        int row = selected.first()->row();
        selectedFolderIndex_ = resultsTable_->item(row, 0)->data(Qt::UserRole).toInt();
        emit folderSelected(selectedFolderIndex_);
    }
}

void ResultsWidget::onExportClicked() {
    emit exportRequested();
}

void ResultsWidget::onRefreshClicked() {
    emit refreshRequested();
}

void ResultsWidget::updateSummary() {
    if (results_.empty()) {
        summaryLabel_->setText("No results");
        return;
    }
    
    int totalImages = 0;
    int totalDetections = 0;
    int completedFolders = 0;
    
    for (const auto& folder : results_) {
        totalImages += folder.imageCount;
        totalDetections += folder.totalDetections;
        if (folder.processed) {
            completedFolders++;
        }
    }
    
    QString summary = QString("Folders: %1/%2 | Images: %3 | Detections: %4")
                     .arg(completedFolders)
                     .arg(results_.size())
                     .arg(totalImages)
                     .arg(totalDetections);
    
    summaryLabel_->setText(summary);
}

} // namespace UI
} // namespace YoloApp
