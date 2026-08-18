#include "Smoke.hpp"

#include <algorithm>
#include <array>
#include <cstdint>
#include <cctype>
#include <fstream>
#include <iostream>
#include <iterator>

#include <opencv2/imgcodecs.hpp>

namespace {
constexpr std::array<unsigned char, 8> kMagic{'M', 'Y', 'S', 'T', 'E', 'G', '0', '1'};
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

std::vector<unsigned char> extract_bytes(const cv::Mat& image, std::size_t byte_count) {
    std::vector<unsigned char> result(byte_count, 0);
    std::size_t bit_offset = 0;
    for (int row = 0; row < image.rows && bit_offset < byte_count * 8U; ++row) {
        const unsigned char* pixels = image.ptr<unsigned char>(row);
        const std::size_t row_bytes = static_cast<std::size_t>(image.cols) * image.elemSize();
        for (std::size_t col = 0; col < row_bytes && bit_offset < byte_count * 8U; ++col, ++bit_offset) {
            result[bit_offset / 8U] |= static_cast<unsigned char>(
                (pixels[col] & 1U) << (7U - (bit_offset % 8U)));
        }
    }
    return result;
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

bool Smoke::embed_data_in_image(const std::vector<unsigned char>& binary_data, cv::Mat& image) {
    const std::size_t capacity = image.total() * image.elemSize();
    if (binary_data.size() > capacity / 8U) {
        std::cerr << "Payload needs " << binary_data.size() * 8U
                  << " carrier bytes, but only " << capacity << " are available.\n";
        return false;
    }

    std::size_t bit_offset = 0;
    for (int row = 0; row < image.rows && bit_offset < binary_data.size() * 8U; ++row) {
        unsigned char* pixels = image.ptr<unsigned char>(row);
        const std::size_t row_bytes = static_cast<std::size_t>(image.cols) * image.elemSize();
        for (std::size_t col = 0; col < row_bytes && bit_offset < binary_data.size() * 8U;
             ++col, ++bit_offset) {
            const unsigned char bit = static_cast<unsigned char>(
                (binary_data[bit_offset / 8U] >> (7U - (bit_offset % 8U))) & 1U);
            pixels[col] = static_cast<unsigned char>((pixels[col] & 0xfeU) | bit);
        }
    }
    return true;
}

bool Smoke::binary_file_to_image(const std::string& binary_file_path,
                                 const std::string& input_image_path,
                                 const std::string& output_image_path) {
    if (!has_png_extension(output_image_path)) {
        std::cerr << "Steganographic output must use the lossless .png format.\n";
        return false;
    }
    cv::Mat image;
    try {
        image = cv::imread(input_image_path, cv::IMREAD_COLOR);
    } catch (const cv::Exception& error) {
        std::cerr << "Could not read carrier image: " << error.what() << '\n';
        return false;
    }
    if (image.empty()) {
        std::cerr << "Could not open carrier image: " << input_image_path << '\n';
        return false;
    }
    swap_red_blue(image);

    std::ifstream input(binary_file_path, std::ios::binary);
    if (!input) {
        std::cerr << "Could not open payload file: " << binary_file_path << '\n';
        return false;
    }
    const std::vector<unsigned char> file_data((std::istreambuf_iterator<char>(input)), {});
    std::vector<unsigned char> encoded(kMagic.begin(), kMagic.end());
    encoded.reserve(kHeaderSize + file_data.size());
    append_u64_be(encoded, static_cast<std::uint64_t>(file_data.size()));
    encoded.insert(encoded.end(), file_data.begin(), file_data.end());

    if (!embed_data_in_image(encoded, image)) {
        return false;
    }

    swap_red_blue(image);
    try {
        if (!cv::imwrite(output_image_path, image)) {
            std::cerr << "Could not write steganographic image: " << output_image_path << '\n';
            return false;
        }
    } catch (const cv::Exception& error) {
        std::cerr << "Could not save steganographic image: " << error.what() << '\n';
        return false;
    }
    std::cout << "Payload hidden in " << output_image_path << '\n';
    return true;
}

bool Smoke::image_to_binary_file(const std::string& image_file_path,
                                 const std::string& output_binary_file_path) {
    cv::Mat image;
    try {
        image = cv::imread(image_file_path, cv::IMREAD_COLOR);
    } catch (const cv::Exception& error) {
        std::cerr << "Could not read steganographic image: " << error.what() << '\n';
        return false;
    }
    if (image.empty()) {
        std::cerr << "Could not open steganographic image: " << image_file_path << '\n';
        return false;
    }
    swap_red_blue(image);
    const std::size_t capacity = image.total() * image.elemSize() / 8U;
    if (capacity < kHeaderSize) {
        std::cerr << "Image is too small to contain a MysticImage payload.\n";
        return false;
    }

    const auto header = extract_bytes(image, kHeaderSize);
    if (!std::equal(kMagic.begin(), kMagic.end(), header.begin())) {
        std::cerr << "Image does not contain a MysticImage steganographic header.\n";
        return false;
    }
    const std::uint64_t length = read_u64_be(header.data() + kMagic.size());
    if (length > capacity - kHeaderSize) {
        std::cerr << "Steganographic payload length is corrupt.\n";
        return false;
    }

    const auto encoded = extract_bytes(image, kHeaderSize + static_cast<std::size_t>(length));
    std::ofstream output(output_binary_file_path, std::ios::binary);
    if (!output) {
        std::cerr << "Could not open output file: " << output_binary_file_path << '\n';
        return false;
    }
    output.write(reinterpret_cast<const char*>(encoded.data() + kHeaderSize),
                 static_cast<std::streamsize>(length));
    if (!output) {
        std::cerr << "Could not write output file: " << output_binary_file_path << '\n';
        return false;
    }
    std::cout << "Payload extracted to " << output_binary_file_path << '\n';
    return true;
}
