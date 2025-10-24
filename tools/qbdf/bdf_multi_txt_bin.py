'''
 * Copyright (c) 2019 Qualcomm Technologies, Inc.
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
    
    # Generate bin filename
    if('_' in i.split(os.sep)[len(i.split(os.sep))-1]):
        bin_f=list(i[:len(i)-4])
        bin_f.reverse()
        bin_f[bin_f.index('_')]='.'
        bin_f.reverse()
        bin_f=''.join(bin_f)
    else:
        bin_f=i[:len(i)-4]+'.bin'
    
    # If output directory is specified, change bin file path
    if output_dir and output_dir != os.path.dirname(i):
        bin_filename = os.path.basename(bin_f)
        bin_f = os.path.join(output_dir, bin_filename)
    
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
        
        print("Executing python bin2txt.py "+newb+" "+newf2)
        b2t.main(["",newb,newf2])

    print("Removing old .txt files.")
    for i in files_diff:
        try:
            os.remove(i)
        except:
            print("Make sure the txt files are not read-only.")
            exit()
    
    print("Renaming new .txt files as old.")
    for i in files_diff:
        os.rename(i[:len(i)-4]+"_new.txt",i)

    print("\n\nSuccessfully generated binaries for BDF files.")

if __name__== "__main__":
    main(sys.argv)