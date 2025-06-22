@echo off
REM Windows Setup Script for YOLO Detection Application
REM Prerequisites: Visual Studio with C++ tools, Qt6, OpenCV

echo === YOLO Detection App Setup for Windows ===
echo.

REM Check if vcpkg is available (recommended package manager for Windows)
where vcpkg >nul 2>nul
if %ERRORLEVEL% EQU 0 (
    echo Found vcpkg - using it to install dependencies...
    echo Installing OpenCV...
    vcpkg install opencv[contrib,dnn]:x64-windows
    echo Installing Qt6...
    vcpkg install qt6-base:x64-windows
    set CMAKE_TOOLCHAIN_FILE=%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake
) else (
    echo vcpkg not found. Please make sure you have:
    echo 1. Qt6 installed (download from qt.io)
    echo 2. OpenCV installed (download from opencv.org)
    echo 3. Set CMAKE_PREFIX_PATH to include Qt6 and OpenCV paths
    echo.
    echo Example:
    echo set CMAKE_PREFIX_PATH=C:\Qt\6.5.0\msvc2019_64;C:\opencv\build
    echo.
    pause
)

REM Create build directory
echo Creating build directory...
if not exist build mkdir build
cd build

REM Configure with CMake
echo Configuring project with CMake...
if defined CMAKE_TOOLCHAIN_FILE (
    cmake .. -DCMAKE_TOOLCHAIN_FILE=%CMAKE_TOOLCHAIN_FILE% -DCMAKE_BUILD_TYPE=Release
) else (
    cmake .. -DCMAKE_BUILD_TYPE=Release
)

if %ERRORLEVEL% NEQ 0 (
    echo CMake configuration failed!
    echo Please ensure Qt6 and OpenCV are properly installed and accessible.
    pause
    exit /b 1
)

REM Build the application
echo Building the application...
cmake --build . --config Release

REM Check if build was successful
if exist "bin\Release\YOLODetectionApp.exe" (
    echo.
    echo === BUILD SUCCESSFUL ===
    echo Application built successfully!
    echo Executable location: %CD%\bin\Release\YOLODetectionApp.exe
    echo.
    echo To run the application, double-click the executable or run:
    echo   bin\Release\YOLODetectionApp.exe
    echo.
    echo Make sure you have a YOLO model file ^(.onnx, .weights, etc.^) ready to load!
) else (
    echo.
    echo === BUILD FAILED ===
    echo Build failed. Please check the error messages above.
    echo Common issues:
    echo 1. Qt6 not found - install Qt6 and set CMAKE_PREFIX_PATH
    echo 2. OpenCV not found - install OpenCV and set CMAKE_PREFIX_PATH
    echo 3. Visual Studio C++ tools not installed
    pause
    exit /b 1
)

echo.
echo === SETUP COMPLETE ===
pause