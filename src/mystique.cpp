#include "Standard.hpp"

int main(int argc, char** argv) {
    if (argc != 4) {
        std::cerr << "Usage: " << argv[0] << " <input_file> <output_file> <mode: png2bin | bin2png>" << std::endl;
        return 1;
    }

    std::string input_file = argv[1];
    std::string output_file = argv[2];
    std::string mode = argv[3];

    if (mode == "bin2png") {
        MysticStandard::binary_file_to_image(input_file, output_file);
    } else if (mode == "png2bin") {
        MysticStandard::image_to_binary_file(input_file, output_file);
    } else {
        std::cerr << "Invalid mode. Use 'png2bin' to convert image to binary or 'bin2png' to convert binary to image." << std::endl;
        return 1;
    }

    return 0;
}
