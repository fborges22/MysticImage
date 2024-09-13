#ifndef MYSTICSMOKE_HPP
#define MYSTICSMOKE_HPP

#include <string>
#include <vector>
#include <opencv2/opencv.hpp>

class Smoke {
public:
    Smoke() = default;
    static std::vector<unsigned char> read_binary_file(const std::string& binary_file_path);
    static void embed_data_in_image(const std::vector<unsigned char>& binary_data, cv::Mat& image);
    static void binary_file_to_image(const std::string& binary_file_path, const std::string& input_image_path, const std::string& output_image_path);
};

#endif // MYSTICSMOKE_HPP
