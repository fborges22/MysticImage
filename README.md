# MysticImage

MysticImage losslessly converts any file to a PNG and restores the exact original bytes. It can also hide a file in the least-significant bits of a carrier image and extract it later.

Two compatible command-line implementations are included:

- `mystique`: C++20/OpenCV executable
- `mysticimg.py`: Python 3/Pillow script

Both use the same versioned format, validate headers and payload lengths, support empty files, and return a non-zero exit status on failure.

## Python quick start

Python 3.10 or newer is required.

```bash
python -m venv .venv
# Windows
.venv\Scripts\python -m pip install -r requirements.txt
# Linux/macOS
.venv/bin/python -m pip install -r requirements.txt
```

Convert a file to an image and restore it:

```bash
python mysticimg.py bin2png archive.zip archive.png
python mysticimg.py png2bin archive.png restored.zip
```

Existing output files are protected by default. Pass `--overwrite` to replace one.

Batch conversion accepts regular files for `bin2png` and PNG files for `png2bin`:

```bash
python mysticimg.py bin2png input-directory output-directory --directory
python mysticimg.py png2bin image-directory restored-directory --directory
```

## C++ build and use

Requirements: CMake 3.16+, a C++20 compiler, and OpenCV with the `core` and `imgcodecs` components.

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

The executable location depends on the generator (`build/mystique` or `build/Release/mystique.exe`). Its commands are:

```bash
mystique bin2png archive.zip archive.png
mystique png2bin archive.png restored.zip
```

The original argument order remains accepted for compatibility: `mystique archive.zip archive.png bin2png`.

## Steganography

Capacity is approximately `(width × height × 3) / 8 - 16` bytes for an RGB carrier. Always write the result as PNG: JPEG and other lossy processing destroy the embedded bits.

```bash
# Python
python mysticimg.py hide secret.bin carrier.png concealed.png
python mysticimg.py reveal concealed.png recovered.bin

# C++
mystique hide secret.bin carrier.png concealed.png
mystique reveal concealed.png recovered.bin
```

Steganography conceals data but does not encrypt it. Encrypt sensitive content before embedding it.

## Test

```bash
python -m unittest discover -s tests -v
```

Tests cover random and empty-file round trips, steganography, capacity and corrupt-header errors, and CLI overwrite protection.

## Install and package the C++ executable

```bash
cmake --install build --prefix /path/to/install
cd build
cpack
```

CPack creates ZIP and TGZ archives in the build directory.

## Format compatibility

Version 1.1 images use a 16-byte header (`MYSTIC01` or `MYSTEG01` plus an unsigned 64-bit big-endian payload length). Older C++ images did not store the original length and cannot be restored without their expected byte count. Older Python images used an unversioned header and are intentionally rejected instead of risking extraction from an unrelated or corrupted image.
