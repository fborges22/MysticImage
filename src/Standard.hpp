#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <cstring>
#include <opencv2/opencv.hpp>

namespace MysticStandard {
    void binary_file_to_image(const std::string& binary_file_path, const std::string& output_image_path);
    void image_to_binary_file(const std::string& image_file_path, const std::string& output_binary_file_path);
}