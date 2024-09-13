#include "Smoke.hpp"
#include <iostream>
#include <fstream>
#include <opencv2/opencv.hpp>

// Method to read a binary file and return its contents as a vector of bytes
std::vector<unsigned char> Smoke::read_binary_file(const std::string& binary_file_path) {
    std::ifstream binary_file(binary_file_path, std::ios::binary);
    if (!binary_file) {
        std::cerr << "Could not open the binary file: " << binary_file_path << std::endl;
        return {};
    }

    // Read the binary data into a vector
    return std::vector<unsigned char>(std::istreambuf_iterator<char>(binary_file), {});
}

// Method to embed binary data into the least significant bits of an image
void Smoke::embed_data_in_image(const std::vector<unsigned char>& binary_data, cv::Mat& image) {
    int rows = image.rows;
    int cols = image.cols * image.channels(); // Consider all color channels
    int total_pixels = rows * cols;

    if (binary_data.size() * 8 > total_pixels) {
        std::cerr << "Error: The binary data is too large to fit in the image." << std::endl;
        return;
    }

    int data_index = 0;
    int bit_index = 0;

    // Loop over every pixel in the image
    for (int row = 0; row < rows; ++row) {
        for (int col = 0; col < cols; ++col) {
            // Access the pixel value (unsigned char)
            unsigned char& pixel = image.at<unsigned char>(row, col);

            // Embed the binary data's bit in the least significant bit of the pixel
            if (data_index < binary_data.size()) {
                unsigned char bit_to_embed = (binary_data[data_index] >> (7 - bit_index)) & 1;
                pixel = (pixel & 0xFE) | bit_to_embed;

                bit_index++;
                if (bit_index == 8) {
                    bit_index = 0;
                    data_index++;
                }
            }
        }
    }
}

// Method to create a new image with embedded binary data
void Smoke::binary_file_to_image(const std::string& binary_file_path, const std::string& input_image_path, const std::string& output_image_path) {
    // Read the input image
    cv::Mat image = cv::imread(input_image_path, cv::IMREAD_COLOR);
    if (image.empty()) {
        std::cerr << "Could not open or find the image: " << input_image_path << std::endl;
        return;
    }

    // Convert the image to a type suitable for modification (8-bit single channel per pixel)
    cv::cvtColor(image, image, cv::COLOR_BGR2GRAY);

    // Read the binary file data
    std::vector<unsigned char> binary_data = read_binary_file(binary_file_path);
    if (binary_data.empty()) {
        std::cerr << "Error reading binary file." << std::endl;
        return;
    }

    // Embed the binary data into the image
    embed_data_in_image(binary_data, image);

    // Save the modified image with embedded data
    if (!cv::imwrite(output_image_path, image)) {
        std::cerr << "Could not write the image to file: " << output_image_path << std::endl;
    } else {
        std::cout << "Successfully created the output image with embedded data: " << output_image_path << std::endl;
    }
}
