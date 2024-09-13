# Mystic Image
Utility for converting binary files into images and vice-versa.

# How to Use:

To convert an binary file into image:

```bash
mystic myapp.exe myimg.png bin2png
```

To convert back the image to binary file:

```bash
mystic myimg.png myapp.exe png2bin
```

## Build the Project:

```bash
mkdir build
cd build
cmake ..
make
```

## Install the Application:

```bash
sudo make install
```

## Package the Application:

```bash
cpack
```

This will generate the specified packages (e.g., .zip and .tgz) in the build directory.

Ensure that you have CMake, CPack, and OpenCV installed on your system to successfully build, install, and package the project.
