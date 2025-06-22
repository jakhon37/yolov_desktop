
// src/core/config.h
#pragma once

namespace YoloApp {
namespace Config {

// Application constants
constexpr const char* APP_NAME = "YOLO Object Detection";
constexpr const char* APP_VERSION = "1.0.0";
constexpr int DEFAULT_WINDOW_WIDTH = 1400;
constexpr int DEFAULT_WINDOW_HEIGHT = 800;

// Supported image formats
const std::vector<std::string> SUPPORTED_EXTENSIONS = {
    ".jpg", ".jpeg", ".png", ".bmp", ".tiff", ".tif", ".webp"
};

// Default COCO class names
const std::vector<std::string> COCO_CLASSES = {
    "person", "bicycle", "car", "motorcycle", "airplane", "bus", "train", "truck",
    "boat", "traffic light", "fire hydrant", "stop sign", "parking meter", "bench",
    "bird", "cat", "dog", "horse", "sheep", "cow", "elephant", "bear", "zebra",
    "giraffe", "backpack", "umbrella", "handbag", "tie", "suitcase", "frisbee",
    "skis", "snowboard", "sports ball", "kite", "baseball bat", "baseball glove",
    "skateboard", "surfboard", "tennis racket", "bottle", "wine glass", "cup",
    "fork", "knife", "spoon", "bowl", "banana", "apple", "sandwich", "orange",
    "broccoli", "carrot", "hot dog", "pizza", "donut", "cake", "chair", "couch",
    "potted plant", "bed", "dining table", "toilet", "tv", "laptop", "mouse",
    "remote", "keyboard", "cell phone", "microwave", "oven", "toaster", "sink",
    "refrigerator", "book", "clock", "vase", "scissors", "teddy bear", "hair drier",
    "toothbrush"
};

// UI Colors
constexpr const char* SUCCESS_COLOR = "color: green; font-weight: bold;";
constexpr const char* ERROR_COLOR = "color: red; font-weight: bold;";
constexpr const char* WARNING_COLOR = "color: orange; font-weight: bold;";

} // namespace Config
} // namespace YoloApp
