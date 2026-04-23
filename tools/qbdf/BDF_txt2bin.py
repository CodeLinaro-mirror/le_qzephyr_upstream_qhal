'''
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
'''
import bdf_multi_txt_bin as bdf_mul
#import elfGen
import sys
import os
from sys import argv
import shutil

print("********* BDF Multiple txt to bin generator tool version 1.0 ***********")
print("\nThis tool generates bin files for multiple text files.\n")

if(len(argv)<2):
    print("Usage: python BDF_txt2bin.py <input_directory> [output_directory]")
    print("  input_directory: Directory containing txt files")
    print("  output_directory: (Optional) Directory to save bin files. Default: same as input")
    exit()

if(len(argv)>3):
    print("Too many arguments. Please provide correct arguments")
    print("Usage: python BDF_txt2bin.py <input_directory> [output_directory]")
    exit()

# Get input and output directories
input_dir = argv[1]
output_dir = argv[2] if len(argv) == 3 else argv[1]

# Create output directory if it doesn't exist
if output_dir != input_dir:
    os.makedirs(output_dir, exist_ok=True)
    print(f"Output directory: {output_dir}\n")

myArgs = ["", input_dir, output_dir]

bdf_mul.main(myArgs)

print(f"\nAll bin files are generated in {output_dir} directory.")