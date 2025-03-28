# Mystic Image
Utility for converting binary files into images and vice-versa. This application also supports steganographic encoding and decoding, enabling the embedding and extraction of binary data within image pixel data.

# How to Use:

To convert an binary file into image:

```bash
mystique myapp.exe myimg.png bin2png
```

To convert back the image to binary file:

```bash
mystique myimg.png myapp.exe png2bin
```

## Build the Project:

```bash
cmake -B_BUILD -DCMAKE_INSTALL_PREFIX=/path/to/install .
cmake --build _BUILD
```

## Install the Application:

```bash
cmake --install
```

## Package the Application:

```bash
cpack
```

This will generate the specified packages (e.g., .zip and .tgz) in the build directory.

Ensure that you have CMake, CPack, and OpenCV installed on your system to successfully build, install, and package the project.
