#!/usr/bin/env python3
import os
import math
import argparse
import logging
import numpy as np
from PIL import Image
from glob import glob

logging.basicConfig(level=logging.INFO, format='%(levelname)s: %(message)s')

def binary_file_to_image(binary_file_path, output_image_path):
    with open(binary_file_path, 'rb') as binary_file:
        binary_data = binary_file.read()

    data_length = len(binary_data)
    pixel_count = data_length // 3 + (1 if data_length % 3 else 0)
    width = int(math.sqrt(pixel_count))
    height = int(math.ceil(pixel_count / width))

    padded_length = width * height * 3
    image_data = np.frombuffer(binary_data, dtype=np.uint8)
    image_data = np.pad(image_data, (0, padded_length - data_length), 'constant', constant_values=0)
    image_data = image_data.reshape((height, width, 3))

    image = Image.fromarray(image_data, 'RGB')
    image.save(output_image_path, format='PNG', optimize=True)
    logging.info(f"Image saved as {output_image_path}")

def image_to_binary_file(image_file_path, output_binary_file_path):
    image = Image.open(image_file_path).convert('RGB')
    image_data = np.array(image)
    binary_data = image_data.flatten().tobytes()

    with open(output_binary_file_path, 'wb') as binary_file:
        binary_file.write(binary_data)
    logging.info(f"Binary data saved as {output_binary_file_path}")

def convert_directory(input_dir, output_dir, mode):
    os.makedirs(output_dir, exist_ok=True)
    extension = '*.bin' if mode == 'bin2png' else '*.png'
    files = glob(os.path.join(input_dir, extension))

    for file_path in files:
        filename = os.path.splitext(os.path.basename(file_path))[0]
        output_path = os.path.join(output_dir, filename + ('.png' if mode == 'bin2png' else '.bin'))
        if os.path.exists(output_path):
            confirm = input(f"{output_path} exists. Overwrite? (y/n): ")
            if confirm.lower() != 'y':
                continue

        if mode == 'bin2png':
            binary_file_to_image(file_path, output_path)
        else:
            image_to_binary_file(file_path, output_path)

def is_image_file(file_path):
    return file_path.lower().endswith(('.png', '.jpg', '.jpeg', '.bmp', '.tiff'))

def main():
    parser = argparse.ArgumentParser(description='Convert between binary file and image.')
    parser.add_argument('-i', '--input_file', type=str, help='Path to the input file or directory.')
    parser.add_argument('-o', '--output_file', type=str, help='Path to the output file or directory.')
    parser.add_argument('-m', '--mode', type=str, required=True, choices=['png2bin', 'bin2png'], help='Conversion mode.')
    parser.add_argument('--dir', action='store_true', help='Enable batch directory conversion.')

    args = parser.parse_args()

    if args.dir:
        if not args.input_file or not args.output_file:
            parser.error('--dir requires both --input_file and --output_file as directories')
        convert_directory(args.input_file, args.output_file, args.mode)
    else:
        if not args.input_file or not args.output_file:
            parser.error('File mode requires both --input_file and --output_file')

        if os.path.exists(args.output_file):
            confirm = input(f"{args.output_file} exists. Overwrite? (y/n): ")
            if confirm.lower() != 'y':
                return

        if args.mode == 'bin2png':
            binary_file_to_image(args.input_file, args.output_file)
        elif args.mode == 'png2bin':
            if not is_image_file(args.input_file):
                logging.error("Input file is not a supported image format.")
                return
            image_to_binary_file(args.input_file, args.output_file)

if __name__ == "__main__":
    main()
