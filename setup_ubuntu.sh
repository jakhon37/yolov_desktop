#!/bin/bash

# Ubuntu/Linux Setup Script for YOLO Detection Application
# Run this script to install dependencies and build the application

echo "=== YOLO Detection App Setup for Ubuntu/Linux ==="

# Update package list
echo "Updating package list..."
sudo apt update

# Install build essentials
echo "Installing build tools..."
sudo apt install -y build-essential cmake git pkg-config

# Install Qt6 development packages
echo "Installing Qt6 development packages..."
sudo apt install -y qt6-base-dev qt6-base-dev-tools libqt6widgets6

# Install OpenCV development packages
echo "Installing OpenCV development packages..."
sudo apt install -y libopencv-dev libopencv-contrib-dev

# Additional dependencies that might be needed
echo "Installing additional dependencies..."
sudo apt install -y \
    libgtk-3-dev \
    libavcodec-dev \
    libavformat-dev \
    libswscale-dev \
    libv4l-dev \
    libxvidcore-dev \
    libx264-dev \
    libjpeg-dev \
    libpng-dev \
    libtiff-dev \
    libatlas-base-dev \
    gfortran

# Create build directory
echo "Creating build directory..."
mkdir -p build
cd build

# Configure with CMake
echo "Configuring project with CMake..."
cmake .. -DCMAKE_BUILD_TYPE=Release

# Build the application
echo "Building the application..."
make -j$(nproc)

# Check if build was successful
if [ -f "./bin/YOLODetectionApp" ]; then
    echo "=== BUILD SUCCESSFUL ==="
    echo "Application built successfully!"
    echo "Executable location: $(pwd)/bin/YOLODetectionApp"
    echo ""
    echo "To run the application:"
    echo "  cd $(pwd)"
    echo "  ./bin/YOLODetectionApp"
    echo ""
    echo "Make sure you have a YOLO model file (.onnx, .weights, etc.) ready to load!"
else
    echo "=== BUILD FAILED ==="
    echo "Build failed. Please check the error messages above."
    exit 1
fi

echo ""
echo "=== SETUP COMPLETE ==="