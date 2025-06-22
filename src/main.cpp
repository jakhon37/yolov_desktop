// src/main.cpp
#include "ui/main_window.h"
#include "core/config.h"
#include <QApplication>
#include <QStyleFactory>
#include <QDir>
#include <QStandardPaths>
#include <QMessageBox>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <iostream>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    
    // Set application properties
    app.setApplicationName(YoloApp::Config::APP_NAME);
    app.setApplicationVersion(YoloApp::Config::APP_VERSION);
    app.setOrganizationName("YoloApp");
    app.setOrganizationDomain("yoloapp.org");
    
    // Command line argument parsing
    QCommandLineParser parser;
    parser.setApplicationDescription("YOLO Object Detection Desktop Application");
    parser.addHelpOption();
    parser.addVersionOption();
    
    QCommandLineOption modelOption(QStringList() << "m" << "model",
                                  "Load model file on startup", "model");
    parser.addOption(modelOption);
    
    QCommandLineOption folderOption(QStringList() << "f" << "folder",
                                   "Select input folder on startup", "folder");
    parser.addOption(folderOption);
    
    QCommandLineOption autoStartOption(QStringList() << "a" << "auto",
                                      "Start processing automatically");
    parser.addOption(autoStartOption);
    
    parser.process(app);
    
    // Set application style
    app.setStyle(QStyleFactory::create("Fusion"));
    
    // Apply dark theme (optional)
    QPalette darkPalette;
    darkPalette.setColor(QPalette::Window, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::WindowText, Qt::white);
    darkPalette.setColor(QPalette::Base, QColor(25, 25, 25));
    darkPalette.setColor(QPalette::AlternateBase, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::ToolTipBase, Qt::white);
    darkPalette.setColor(QPalette::ToolTipText, Qt::white);
    darkPalette.setColor(QPalette::Text, Qt::white);
    darkPalette.setColor(QPalette::Button, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::ButtonText, Qt::white);
    darkPalette.setColor(QPalette::BrightText, Qt::red);
    darkPalette.setColor(QPalette::Link, QColor(42, 130, 218));
    darkPalette.setColor(QPalette::Highlight, QColor(42, 130, 218));
    darkPalette.setColor(QPalette::HighlightedText, Qt::black);
    app.setPalette(darkPalette);
    
    // Create and show main window
    YoloApp::UI::MainWindow window;
    window.show();
    
    // Process command line options after window is shown
    if (parser.isSet(modelOption)) {
        QString modelPath = parser.value(modelOption);
        if (QFile::exists(modelPath)) {
            // Load model (would need to add public method to MainWindow)
            std::cout << "Loading model: " << modelPath.toStdString() << std::endl;
        } else {
            QMessageBox::warning(&window, "Error", "Model file not found: " + modelPath);
        }
    }
    
    if (parser.isSet(folderOption)) {
        QString folderPath = parser.value(folderOption);
        if (QDir(folderPath).exists()) {
            // Select folder (would need to add public method to MainWindow)
            std::cout << "Selecting folder: " << folderPath.toStdString() << std::endl;
        } else {
            QMessageBox::warning(&window, "Error", "Folder not found: " + folderPath);
        }
    }
    
    return app.exec();
}