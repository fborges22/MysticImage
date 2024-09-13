#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <cstring>
#include <opencv2/opencv.hpp>
#include "Standard.hpp"

void MysticStandard::binary_file_to_image(const std::string& binary_file_path, const std::string& output_image_path) {
    // Read the binary file
    std::ifstream binary_file(binary_file_path, std::ios::binary);
    if (!binary_file) {
        std::cerr << "Could not open the binary file: " << binary_file_path << std::endl;
        return;
    }

    std::vector<unsigned char> binary_data((std::istreambuf_iterator<char>(binary_file)), std::istreambuf_iterator<char>());
    binary_file.close();

    // Determine the dimensions of the image
    size_t data_length = binary_data.size();
    size_t pixel_count = data_length / 3 + (data_length % 3 ? 1 : 0);
    int width = static_cast<int>(std::sqrt(pixel_count));
    int height = static_cast<int>(std::ceil(static_cast<double>(pixel_count) / width));

    // Pad the binary data to fit the image dimensions
    binary_data.resize(width * height * 3, 0);

    // Create an OpenCV image from the binary data
    cv::Mat image(height, width, CV_8UC3, binary_data.data());

    // Save the image
    if (!cv::imwrite(output_image_path, image)) {
        std::cerr << "Could not save the image to: " << output_image_path << std::endl;
    } else {
        std::cout << "Image saved as " << output_image_path << std::endl;
    }
}

void MysticStandard::image_to_binary_file(const std::string& image_file_path, const std::string& output_binary_file_path) {
    // Read the image file
    cv::Mat image = cv::imread(image_file_path, cv::IMREAD_COLOR);
    if (image.empty()) {
        std::cerr << "Could not open or find the image: " << image_file_path << std::endl;
        return;
    }

    // Flatten the image data to a 1D array
    std::vector<unsigned char> binary_data(image.begin<unsigned char>(), image.end<unsigned char>());

    // Write the binary data to the output file
    std::ofstream binary_file(output_binary_file_path, std::ios::binary);
    if (!binary_file) {
        std::cerr << "Could not open the output binary file: " << output_binary_file_path << std::endl;
        return;
    }

    binary_file.write(reinterpret_cast<char*>(binary_data.data()), binary_data.size());
    binary_file.close();

    std::cout << "Binary data saved as " << output_binary_file_path << std::endl;
}