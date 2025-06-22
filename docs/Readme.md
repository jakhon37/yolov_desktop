# YOLO Object Detection Desktop Application

A modern, extensible C++ desktop application for object detection using YOLO models. Built with Qt6 and OpenCV, it provides a user-friendly interface for batch processing images and visualizing detection results.

![Application Screenshot](docs/screenshot.png)

## Features

- **Multi-format Model Support**: ONNX, Darknet (.weights + .cfg), TensorFlow (.pb)
- **Batch Processing**: Process single folders or nested folder structures
- **Real-time Visualization**: View detection results with bounding boxes and confidence scores
- **Configurable Detection**: Adjust confidence thresholds, NMS settings, and input dimensions
- **Detailed Metadata**: Comprehensive image and detection information
- **Export Results**: Save processed images and detection data
- **Cross-platform**: Windows and Linux support

## Project Structure

```
YOLODetectionApp/
├── src/
│   ├── core/                   # Core detection engine
│   │   ├── types.h            # Data structures and types
│   │   ├── config.h           # Configuration constants
│   │   ├── detector.h/.cpp    # YOLO detection interface and implementation
│   ├── processing/            # Image and folder processing
│   │   ├── folder_scanner.h/.cpp      # Folder scanning utilities
│   │   ├── image_processor.h/.cpp     # Image processing and annotation
│   ├── workers/               # Background processing
│   │   ├── detection_worker.h/.cpp    # Multi-threaded detection worker
│   ├── ui/                    # User interface components
│   │   ├── main_window.h/.cpp         # Main application window
│   │   ├── results_widget.h/.cpp      # Results table widget
│   │   ├── image_viewer.h/.cpp        # Image display widget
│   └── main.cpp               # Application entry point
├── CMakeLists.txt             # Build configuration
├── setup_ubuntu.sh            # Ubuntu setup script
├── setup_windows.bat          # Windows setup script
├── docs/                      # Documentation
└── examples/                  # Example images and models
```

## Requirements

### Ubuntu/Linux
- Ubuntu 20.04+ (or equivalent)
- CMake 3.16+
- GCC 9+ or Clang 10+
- Qt6 development packages
- OpenCV 4.0+ with DNN module

### Windows
- Windows 10+
- Visual Studio 2019/2022 with C++ tools
- CMake 3.16+
- Qt6 (6.2+)
- OpenCV 4.0+

## Installation

### Quick Setup - Ubuntu

```bash
# Clone the repository
git clone https://github.com/yourusername/YOLODetectionApp.git
cd YOLODetectionApp

# Run the setup script
chmod +x setup_ubuntu.sh
./setup_ubuntu.sh

# The application will be built in build/bin/YOLODetectionApp
```

### Quick Setup - Windows

```batch
# Clone the repository
git clone https://github.com/yourusername/YOLODetectionApp.git
cd YOLODetectionApp

# Run the setup script
setup_windows.bat

# The application will be built in build\bin\Release\YOLODetectionApp.exe
```

### Manual Installation

#### Ubuntu/Linux

1. **Install Dependencies**:
```bash
sudo apt update
sudo apt install -y build-essential cmake git pkg-config
sudo apt install -y qt6-base-dev qt6-base-dev-tools libqt6widgets6
sudo apt install -y libopencv-dev libopencv-contrib-dev
```

2. **Build the Application**:
```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
```

#### Windows

1. **Install Dependencies**:
   - Install Visual Studio 2019/2022 with C++ development tools
   - Install Qt6 from [qt.io](https://www.qt.io/download)
   - Install OpenCV from [opencv.org](https://opencv.org/releases/) or use vcpkg:
     ```cmd
     vcpkg install opencv[contrib,dnn]:x64-windows
     vcpkg install qt6-base:x64-windows
     ```

2. **Build the Application**:
```cmd
mkdir build && cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake
cmake --build . --config Release
```

## Usage

### Getting Started

1. **Launch the Application**:
   ```bash
   # Linux
   ./build/bin/YOLODetectionApp
   
   # Windows
   build\bin\Release\YOLODetectionApp.exe
   ```

2. **Load a YOLO Model**:
   - Click "Load Model" button
   - Select your YOLO model file (.onnx, .weights, .pt, .pb)
   - For Darknet models (.weights), you may need to provide a .cfg file
   - Optionally load custom class names (.txt file)

3. **Select Input Folder**:
   - Click "Select Folder" button
   - Choose a folder containing images or subfolders with images
   - Supported formats: JPG, PNG, BMP, TIFF, WebP

4. **Configure Detection Settings** (Optional):
   - Click "Settings" next to the model controls
   - Adjust confidence threshold (default: 0.5)
   - Adjust NMS threshold (default: 0.4)
   - Set input dimensions (default: 640x640)

5. **Start Processing**:
   - Click "Start Detection"
   - Monitor progress in the progress bar
   - View results in real-time as folders are processed

### Command Line Options

```bash
# Load model and folder automatically
./YOLODetectionApp --model /path/to/model.onnx --folder /path/to/images

# Show help
./YOLODetectionApp --help
```

### Viewing Results

- **Results Table**: Shows processed folders with image counts and detection statistics
- **Image Viewer**: Click on a folder row to view images with detection annotations
- **Zoom Controls**: Use the slider to zoom in/out on images
- **Toggle Annotations**: Show/hide detection bounding boxes
- **Metadata Panel**: View detailed information about selected images

## Supported YOLO Models

### ONNX Models (Recommended)
- YOLOv5 (official PyTorch export)
- YOLOv8/YOLOv9 (Ultralytics)
- YOLOv10
- Any YOLO model exported to ONNX format

### Darknet Models
- YOLOv3/YOLOv4 (.weights + .cfg files)
- Custom Darknet models

### TensorFlow Models
- YOLOv5 TensorFlow exports
- Custom TensorFlow YOLO models

### Where to Get Models

1. **YOLOv5**: Download from [Ultralytics releases](https://github.com/ultralytics/yolov5/releases)
2. **YOLOv8**: Use [Ultralytics pip package](https://github.com/ultralytics/ultralytics)
3. **Official Models**: [YOLO official repositories](https://github.com/WongKinYiu/yolov7)

## Configuration

### Detection Parameters

- **Confidence Threshold**: Minimum confidence for detections (0.1-1.0)
- **NMS Threshold**: Non-Maximum Suppression overlap threshold (0.1-1.0)
- **Input Size**: Model input dimensions (typically 640x640 for modern YOLO)

### Class Filtering

Create a text file with class names (one per line) to filter detections:
```
person
car
bicycle
```

## Development

### Architecture Overview

The application follows a modular architecture with clear separation of concerns:

- **Core**: Detection engine and data structures
- **Processing**: Image and folder processing utilities
- **Workers**: Background threading for non-blocking UI
- **UI**: Qt-based user interface components

### Key Classes

- `YoloDetector`: Main detection engine implementing `IDetector` interface
- `DetectionWorker`: QThread-based worker for background processing
- `MainWindow`: Primary UI coordinator
- `ImageProcessor`: Handles image loading, annotation, and metadata
- `FolderScanner`: Recursive folder scanning with progress callbacks

### Adding New Features

1. **New Model Formats**: Extend `YoloDetector::loadModel()`
2. **Export Formats**: Implement in `MainWindow::onExportResults()`
3. **UI Components**: Add new widgets in the `ui/` namespace
4. **Processing Options**: Extend `DetectionConfig` structure

### Building with Different Qt/OpenCV Versions

```bash
# Specify custom Qt installation
cmake .. -DCMAKE_PREFIX_PATH=/path/to/qt6

# Specify custom OpenCV installation
cmake .. -DOpenCV_DIR=/path/to/opencv/build

# Use specific versions
cmake .. -DCMAKE_PREFIX_PATH="/path/to/qt6;/path/to/opencv"
```

## Troubleshooting

### Common Issues

1. **Model Loading Fails**:
   - Verify model file format and integrity
   - Check if model requires specific input dimensions
   - Ensure OpenCV DNN module supports the model format

2. **Qt6 Not Found**:
   ```bash
   # Ubuntu: Install Qt6 development packages
   sudo apt install qt6-base-dev
   
   # Set CMAKE_PREFIX_PATH
   export CMAKE_PREFIX_PATH=/usr/lib/x86_64-linux-gnu/cmake/Qt6
   ```

3. **OpenCV DNN Module Missing**:
   ```bash
   # Rebuild OpenCV with DNN support
   cmake -DOPENCV_EXTRA_MODULES_PATH=opencv_contrib/modules \
         -DBUILD_opencv_dnn=ON \
         opencv/
   ```

4. **Performance Issues**:
   - Use GPU acceleration if available (CUDA/OpenVINO)
   - Reduce input image resolution
   - Lower confidence thresholds
   - Process fewer images simultaneously

### Debug Mode

Build with debug symbols for troubleshooting:
```bash
cmake .. -DCMAKE_BUILD_TYPE=Debug
```

### Logging

Set environment variable for verbose output:
```bash
export QT_LOGGING_RULES="*.debug=true"
./YOLODetectionApp
```

## Contributing

1. Fork the repository
2. Create a feature branch: `git checkout -b feature-name`
3. Make your changes with proper documentation
4. Add tests if applicable
5. Submit a pull request

### Code Style

- Follow modern C++17 practices
- Use Qt conventions for UI code
- Document public interfaces
- Include error handling
- Write unit tests for core functionality

## License

This project is licensed under the MIT License. See [LICENSE](LICENSE) file for details.

## Acknowledgments

- [Ultralytics](https://ultralytics.com/) for YOLOv5/v8 models
- [OpenCV](https://opencv.org/) for computer vision functionality
- [Qt](https://www.qt.io/) for the cross-platform GUI framework
- YOLO authors for the groundbreaking object detection algorithm

## Support

- **Issues**: Report bugs on [GitHub Issues](https://github.com/yourusername/YOLODetectionApp/issues)
- **Discussions**: Join [GitHub Discussions](https://github.com/yourusername/YOLODetectionApp/discussions)
- **Documentation**: Check the [Wiki](https://github.com/yourusername/YOLODetectionApp/wiki)

---

**Happy Detecting! 🎯**