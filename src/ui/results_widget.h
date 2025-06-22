// src/ui/results_widget.h
#pragma once

#include "../core/types.h"
#include <QWidget>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QProgressBar>
#include <QHeaderView>
#include <memory>

namespace YoloApp {
namespace UI {

/**
 * @brief Widget for displaying detection results in a table
 */
class ResultsWidget : public QWidget {
    Q_OBJECT

public:
    explicit ResultsWidget(QWidget* parent = nullptr);
    ~ResultsWidget() override = default;
    
    /**
     * @brief Update results display
     */
    void updateResults(const std::vector<Core::FolderResult>& results);
    
    /**
     * @brief Clear all results
     */
    void clearResults();
    
    /**
     * @brief Set processing progress
     */
    void setProgress(int current, int total);
    
    /**
     * @brief Get currently selected folder index
     */
    int getSelectedFolderIndex() const;

signals:
    void folderSelected(int folderIndex);
    void exportRequested();
    void refreshRequested();

private slots:
    void onTableSelectionChanged();
    void onExportClicked();
    void onRefreshClicked();

private:
    void setupUI();
    void setupTable();
    void updateSummary();
    
    QVBoxLayout* mainLayout_;
    QHBoxLayout* topLayout_;
    QHBoxLayout* bottomLayout_;
    
    QLabel* summaryLabel_;
    QPushButton* exportButton_;
    QPushButton* refreshButton_;
    QProgressBar* progressBar_;
    QTableWidget* resultsTable_;
    
    std::vector<Core::FolderResult> results_;
    int selectedFolderIndex_;
};

} // namespace UI
} // namespace YoloApp  