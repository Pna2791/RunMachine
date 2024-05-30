from PIL import Image
import numpy as np
import os

def rgb_to_rgb565(r, g, b):
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3)

def image_to_tft_array(image_path):
    # Open an image file
    with Image.open(image_path) as img:
        # Get the image dimensions
        tft_width, tft_height = img.size
        
        # Convert image to RGB (if not already in RGB)
        img = img.convert('RGB')
        
        # Convert image to numpy array
        img_array = np.array(img)
        
        # Create a list to hold the RGB565 values
        rgb565_array = []
        
        for row in img_array:
            for pixel in row:
                r, g, b = pixel
                rgb565 = rgb_to_rgb565(r, g, b)
                rgb565_array.append(rgb565)
        
        return rgb565_array, tft_width

def format_as_c_array(rgb565_array, array_name, width):
    # Create header for the array
    c_array = f"const PROGMEM uint16_t {array_name}[{len(rgb565_array)}] = {{\n"
    
    # Add array elements with appropriate formatting
    for i, val in enumerate(rgb565_array):
        if i % width == 0:
            c_array += "    "
        c_array += f"0x{val:04X}, "
        if (i + 1) % width == 0:
            c_array += f"  // 0x{i + 1:04X} ({i + 1}) pixels\n"
    
    # Close the array
    c_array += "\n};"
    
    return c_array

def save_to_header_file(c_array_str, file_name):
    with open(file_name, 'w') as file:
        file.write(c_array_str)

# Example usage
def main(image_path):
    # Get the base name of the image file (without extension)
    base_name = os.path.splitext(os.path.basename(image_path))[0]
    # Create the header file name by changing the extension to .h
    header_file_name = f"{base_name}.h"
    # Create the array name based on the base name
    array_name = base_name
    
    rgb565_array, tft_width = image_to_tft_array(image_path)
    c_array_str = format_as_c_array(rgb565_array, array_name, tft_width)
    save_to_header_file(c_array_str, header_file_name)
    print(f"Header file '{header_file_name}' created successfully.")

# Parameters
image_path = 'grafana.png'  # replace with your image path

main(image_path)
