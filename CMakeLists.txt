
cmake_minimum_required(VERSION 3.16)
project(YOLODetectionApp)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(Qt6 REQUIRED COMPONENTS Core Widgets)
find_package(OpenCV REQUIRED)

# Enable automatic MOC, UIC, and RCC processing
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Include directories
include_directories(src)

# Source files
set(SOURCES
    src/main.cpp
    src/core/detector.cpp
    src/processing/folder_scanner.cpp
    src/processing/image_processor.cpp
    src/workers/detection_worker.cpp
    src/ui/main_window.cpp
    src/ui/results_widget.cpp
    src/ui/image_viewer.cpp
)

# Header files
set(HEADERS
    src/core/types.h
    src/core/config.h
    src/core/detector.h
    src/processing/folder_scanner.h
    src/processing/image_processor.h
    src/workers/detection_worker.h
    src/ui/main_window.h
    src/ui/results_widget.h
    src/ui/image_viewer.h
)

# Add executable
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# Link libraries
target_link_libraries(${PROJECT_NAME}
    Qt6::Core
    Qt6::Widgets
    ${OpenCV_LIBS}
)

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE
    ${OpenCV_INCLUDE_DIRS}
    src
)

# Compiler-specific options
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4)
    # Enable parallel compilation
    target_compile_options(${PROJECT_NAME} PRIVATE /MP)
    # Set UTF-8 encoding
    target_compile_options(${PROJECT_NAME} PRIVATE /utf-8)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE 
        -Wall -Wextra -Wpedantic
        -pthread
    )
endif()

# Enable C++17 filesystem
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS "9.0")
    target_link_libraries(${PROJECT_NAME} stdc++fs)
endif()

# Set output directory
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# Copy models and example files to output directory (if they exist)
if(EXISTS "${CMAKE_SOURCE_DIR}/models")
    file(COPY "${CMAKE_SOURCE_DIR}/models" DESTINATION "${CMAKE_BINARY_DIR}/bin/")
endif()

if(EXISTS "${CMAKE_SOURCE_DIR}/examples")
    file(COPY "${CMAKE_SOURCE_DIR}/examples" DESTINATION "${CMAKE_BINARY_DIR}/bin/")
endif()

# Install target
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
)

# Install additional files
install(DIRECTORY "${CMAKE_BINARY_DIR}/bin/models" DESTINATION bin OPTIONAL)
install(DIRECTORY "${CMAKE_BINARY_DIR}/bin/examples" DESTINATION bin OPTIONAL)

# Create package
set(CPACK_PACKAGE_NAME "YOLODetectionApp")
set(CPACK_PACKAGE_VERSION ${CMAKE_PROJECT_VERSION})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "YOLO Object Detection Desktop Application")
set(CPACK_PACKAGE_VENDOR "YoloApp")

if(WIN32)
    set(CPACK_GENERATOR "NSIS")
    set(CPACK_NSIS_DISPLAY_NAME "YOLO Detection App")
    set(CPACK_NSIS_PACKAGE_NAME "YOLODetectionApp")
    set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)
else()
    set(CPACK_GENERATOR "DEB;RPM;TGZ")
    set(CPACK_DEBIAN_PACKAGE_DEPENDS "libqt6widgets6, libopencv-dev")
endif()

include(CPack)