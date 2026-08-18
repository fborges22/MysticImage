#ifndef MYSTIC_STANDARD_HPP
#define MYSTIC_STANDARD_HPP

#include <string>

namespace MysticStandard {
bool binary_file_to_image(const std::string& binary_file_path,
                          const std::string& output_image_path);
bool image_to_binary_file(const std::string& image_file_path,
                          const std::string& output_binary_file_path);
}

#endif
