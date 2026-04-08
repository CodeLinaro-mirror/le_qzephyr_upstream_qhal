'''
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
'''
from sys import argv
import sys
import os
import txt2bin as t2b
import bin2txt as b2t


def create_filenames(i, output_dir=None):
    
    txt_f=i[:len(i)-4]+"_new.txt"
    
    # Generate bin filename - Only check filename part, not full path
    # Handle both Unix and Windows path separators explicitly
    # Replace backslashes with forward slashes first, then normalize
    normalized_path = i.replace('\\', '/')
    normalized_path = os.path.normpath(normalized_path)
    filename = os.path.basename(normalized_path)
    
    # Generate bin filename based on filename only (not full path)
    if('_' in filename):
        # Replace the last underscore in the filename with a dot
        filename_no_ext = filename[:-4]  # Remove .txt extension
        # Find last underscore and replace with dot
        last_underscore_pos = filename_no_ext.rfind('_')
        if last_underscore_pos != -1:
            bin_filename = filename_no_ext[:last_underscore_pos] + '.' + filename_no_ext[last_underscore_pos+1:]
        else:
            bin_filename = filename_no_ext + '.bin'
    else:
        bin_filename = filename[:-4] + '.bin'  # Remove .txt, add .bin
    
    # Construct full path for bin file
    if output_dir and output_dir != os.path.dirname(normalized_path):
        bin_f = os.path.join(output_dir, bin_filename)
    else:
        # Use same directory as input file
        input_dir = os.path.dirname(normalized_path)
        if input_dir:
            bin_f = os.path.join(input_dir, bin_filename)
        else:
            bin_f = bin_filename
    
    return txt_f, bin_f

def main(argv):

    if(len(argv)<2):
        print("Less arguments. Please provide enough arguments")
        exit()

    if(len(argv)>3):
        print("More arguments. Please provide enough arguments")
        exit()

    input_dir = argv[1]
    output_dir = argv[2] if len(argv) == 3 else argv[1]

    print("********* BDF_multi_txt_bin tool version 1.0 **********") 
    print("\nThis tool is used to generate bin files for multiple txt files.\n")

    files_toprocessdiff = set()
    paths = list()
    paths.append(input_dir)
    for p in paths:
        if os.path.isfile(p):
            files_toprocessdiff.add(p)
        elif os.path.isdir(p):
            for root, dirs, files in os.walk(p):
                files_toprocessdiff.update([os.path.join(root, f) for f in files])

    # only check txt file
    for i in files_toprocessdiff:
        if not i.endswith('.txt'):
            continue
            
        if "new" in i:
            print("Remove new files in "+input_dir+" directory and try. Please keep only checked in texts.")
            exit()

    files_diff = [i for i in files_toprocessdiff if i.endswith('.txt')]

    if not files_diff:
        print("No txt files found to process")
        exit()

    for i in files_diff:
        newf,newb=create_filenames(i, output_dir)
        newf2=newf[:len(newf)-8]+".txt"
        
        print("Executing python txt2bin.py "+newf2+" "+newb)
        t2b.main(["",newf2,newb])
        
        # Only perform bin2txt verification if output is in same directory as input
        # This preserves the original txt files when output directory is different
        if output_dir == input_dir or output_dir == os.path.dirname(i):
            print("Executing python bin2txt.py "+newb+" "+newf2)
            b2t.main(["",newb,newf2])
            
            print("Removing old .txt files.")
            for j in files_diff:
                try:
                    os.remove(j)
                except:
                    print("Make sure the txt files are not read-only.")
                    exit()
            
            print("Renaming new .txt files as old.")
            for j in files_diff:
                os.rename(j[:len(j)-4]+"_new.txt",j)
            break
        else:
            print(f"Skipping verification for {newf2} (output directory is different)")

    print("\n\nSuccessfully generated binaries for BDF files.")

if __name__== "__main__":
    main(sys.argv)