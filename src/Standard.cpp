#include "Standard.hpp"

#include <algorithm>
#include <array>
#include <cstdint>
#include <cctype>
#include <fstream>
#include <cmath>
#include <iostream>
#include <iterator>
#include <limits>
#include <vector>

#include <opencv2/imgcodecs.hpp>

namespace {
constexpr std::array<unsigned char, 8> kMagic{'M', 'Y', 'S', 'T', 'I', 'C', '0', '1'};
constexpr std::size_t kHeaderSize = kMagic.size() + sizeof(std::uint64_t);

bool has_png_extension(const std::string& path) {
    if (path.size() < 4) {
        return false;
    }
    std::string extension = path.substr(path.size() - 4);
    std::transform(extension.begin(), extension.end(), extension.begin(),
                   [](unsigned char character) { return static_cast<char>(std::tolower(character)); });
    return extension == ".png";
}

void append_u64_be(std::vector<unsigned char>& output, std::uint64_t value) {
    for (int shift = 56; shift >= 0; shift -= 8) {
        output.push_back(static_cast<unsigned char>((value >> shift) & 0xffU));
    }
}

std::uint64_t read_u64_be(const unsigned char* input) {
    std::uint64_t value = 0;
    for (std::size_t i = 0; i < sizeof(value); ++i) {
        value = (value << 8U) | input[i];
    }
    return value;
}

void swap_red_blue(cv::Mat& image) {
    for (int row = 0; row < image.rows; ++row) {
        unsigned char* pixels = image.ptr<unsigned char>(row);
        for (int col = 0; col < image.cols; ++col) {
            std::swap(pixels[col * 3], pixels[col * 3 + 2]);
        }
    }
}
}

bool MysticStandard::binary_file_to_image(const std::string& binary_file_path,
                                           const std::string& output_image_path) {
    if (!has_png_extension(output_image_path)) {
        std::cerr << "Output must use the lossless .png format.\n";
        return false;
    }
    std::ifstream input(binary_file_path, std::ios::binary);
    if (!input) {
        std::cerr << "Could not open input file: " << binary_file_path << '\n';
        return false;
    }

    std::vector<unsigned char> file_data((std::istreambuf_iterator<char>(input)), {});
    std::vector<unsigned char> encoded(kMagic.begin(), kMagic.end());
    encoded.reserve(kHeaderSize + file_data.size());
    append_u64_be(encoded, static_cast<std::uint64_t>(file_data.size()));
    encoded.insert(encoded.end(), file_data.begin(), file_data.end());

    const std::size_t pixel_count = (encoded.size() + 2U) / 3U;
    const auto width = static_cast<std::size_t>(std::ceil(std::sqrt(static_cast<double>(pixel_count))));
    const auto height = (pixel_count + width - 1U) / width;
    if (width > static_cast<std::size_t>(std::numeric_limits<int>::max()) ||
        height > static_cast<std::size_t>(std::numeric_limits<int>::max())) {
        std::cerr << "Input is too large for supported image dimensions.\n";
        return false;
    }
    encoded.resize(width * height * 3U, 0);
    cv::Mat image(static_cast<int>(height), static_cast<int>(width), CV_8UC3, encoded.data());
    swap_red_blue(image);  // encoded bytes are RGB; OpenCV stores BGR

    try {
        if (!cv::imwrite(output_image_path, image)) {
            std::cerr << "Could not save PNG image: " << output_image_path << '\n';
            return false;
        }
    } catch (const cv::Exception& error) {
        std::cerr << "Could not save image: " << error.what() << '\n';
        return false;
    }
    std::cout << "Image saved as " << output_image_path << '\n';
    return true;
}

bool MysticStandard::image_to_binary_file(const std::string& image_file_path,
                                           const std::string& output_binary_file_path) {
    cv::Mat image;
    try {
        image = cv::imread(image_file_path, cv::IMREAD_COLOR);
    } catch (const cv::Exception& error) {
        std::cerr << "Could not read image: " << error.what() << '\n';
        return false;
    }
    if (image.empty()) {
        std::cerr << "Could not open image: " << image_file_path << '\n';
        return false;
    }
    if (!image.isContinuous()) {
        image = image.clone();
    }
    swap_red_blue(image);  // normalize OpenCV's BGR bytes to the on-disk RGB order
    const std::size_t available = image.total() * image.elemSize();
    const unsigned char* bytes = image.ptr<unsigned char>(0);
    if (available < kHeaderSize || !std::equal(kMagic.begin(), kMagic.end(), bytes)) {
        std::cerr << "Image is not a MysticImage file (invalid header).\n";
        return false;
    }
    const std::uint64_t stored_length = read_u64_be(bytes + kMagic.size());
    if (stored_length > available - kHeaderSize) {
        std::cerr << "Image is corrupt: payload length exceeds its capacity.\n";
        return false;
    }

    std::ofstream output(output_binary_file_path, std::ios::binary);
    if (!output) {
        std::cerr << "Could not open output file: " << output_binary_file_path << '\n';
        return false;
    }
    output.write(reinterpret_cast<const char*>(bytes + kHeaderSize),
                 static_cast<std::streamsize>(stored_length));
    if (!output) {
        std::cerr << "Could not write output file: " << output_binary_file_path << '\n';
        return false;
    }
    std::cout << "Binary data saved as " << output_binary_file_path << '\n';
    return true;
}
