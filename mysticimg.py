import numpy as np
from PIL import Image
import math
import argparse

def binary_file_to_image(binary_file_path, output_image_path):
    # Read the binary file
    with open(binary_file_path, 'rb') as binary_file:
        binary_data = binary_file.read()

    # Determine the dimensions of the image
    data_length = len(binary_data)
    pixel_count = data_length // 3 + (1 if data_length % 3 else 0)
    width = int(math.sqrt(pixel_count))
    height = int(math.ceil(pixel_count / width))
    
    # Create a numpy array from the binary data
    padded_length = width * height * 3
    image_data = np.frombuffer(binary_data, dtype=np.uint8)
    image_data = np.pad(image_data, (0, padded_length - data_length), 'constant', constant_values=0)
    
    # Reshape the array to fit the image dimensions
    image_data = image_data.reshape((height, width, 3))
    
    # Convert the numpy array to an image
    image = Image.fromarray(image_data, 'RGB')
    
    # Save the image
    image.save(output_image_path)
    print(f"Image saved as {output_image_path}")

def image_to_binary_file(image_file_path, output_binary_file_path):
    # Open the image file
    with open(image_file_path, 'rb') as image_file:
        image = Image.open(image_file)
        image = image.convert('RGB')  # Ensure image is in RGB mode

    # Convert the image data to a numpy array
    image_data = np.array(image)

    # Flatten the image data to a 1D array
    binary_data = image_data.flatten()

    # Convert the numpy array to bytes
    binary_data = binary_data.tobytes()

    # Write the binary data to the output file
    with open(output_binary_file_path, 'wb') as binary_file:
        binary_file.write(binary_data)
    
    print(f"Binary data saved as {output_binary_file_path}")

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Convert between binary file and image.')
    parser.add_argument('-i', '--input_file', type=str, required=True, help='Path to the input file.')
    parser.add_argument('-o', '--output_file', type=str, required=True, help='Path to the output file.')
    parser.add_argument('-m', '--mode', type=str, required=True, choices=['png2bin', 'bin2png'], help='Mode of conversion: png2bin or bin2png.')

    args = parser.parse_args()
    
    if args.mode == 'bin2png':
        binary_file_to_image(args.input_file, args.output_file)
    elif args.mode == 'png2bin':
        image_to_binary_file(args.input_file, args.output_file)
