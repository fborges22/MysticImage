#include "Standard.hpp"
#include "Smoke.hpp"

#include <iostream>
#include <string>

namespace {
void usage(const char* program) {
    std::cerr << "Usage:\n"
              << "  " << program << " bin2png <input-file> <output.png>\n"
              << "  " << program << " png2bin <input.png> <output-file>\n"
              << "  " << program << " hide <payload-file> <carrier.png> <output.png>\n"
              << "  " << program << " reveal <input.png> <output-file>\n";
}
}

int main(int argc, char** argv) {
    if (argc < 2) {
        usage(argv[0]);
        return 2;
    }

    const std::string mode = argv[1];
    bool success = false;
    if (mode == "bin2png" && argc == 4) {
        success = MysticStandard::binary_file_to_image(argv[2], argv[3]);
    } else if (mode == "png2bin" && argc == 4) {
        success = MysticStandard::image_to_binary_file(argv[2], argv[3]);
    } else if (mode == "hide" && argc == 5) {
        success = Smoke::binary_file_to_image(argv[2], argv[3], argv[4]);
    } else if (mode == "reveal" && argc == 4) {
        success = Smoke::image_to_binary_file(argv[2], argv[3]);
    } else if (argc == 4 && std::string(argv[3]) == "bin2png") {
        success = MysticStandard::binary_file_to_image(argv[1], argv[2]);
    } else if (argc == 4 && std::string(argv[3]) == "png2bin") {
        success = MysticStandard::image_to_binary_file(argv[1], argv[2]);
    } else {
        usage(argv[0]);
        return 2;
    }

    return success ? 0 : 1;
}
