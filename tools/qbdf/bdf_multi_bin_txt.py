'''
 * Copyright (c) 2021 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
'''
from sys import argv
import sys
import os
import bin2txt as b2t

def create_filenames(i):
    bin_f = i
    if not('.bin' in i):
        txt_f = list(i)
        txt_f[txt_f.index('.')] = '_'
        txt_f = ''.join(txt_f) + '.txt'
    else:
        txt_f = i[:len(i)-4] + '.txt'
    return txt_f, bin_f

def main(argv):
    if len(argv) < 2:
        print("Less arguments. Please provide enough arguments")
        exit()

    if len(argv) > 2:
        print("More arguments. Please provide enough arguments")
        exit()

    script, temp1 = argv

    print("********* BDF_multi_bin_txt tool version 1.0 **********")
    print("\nThis tool is used to generate txt files for multiple bin files.\nMake sure required NvTpl.txt in current directory\n")

    if not os.path.exists("NvTpl.txt"):
        print("NvTpl.txt not found in current directory. Please place it and retry.")
        exit()
    
    bdf_template = "NvTpl.txt"
    
    files_toprocess = set()
    paths = [temp1]
    for p in paths:
        if os.path.isfile(p):
            files_toprocess.add(p)
        elif os.path.isdir(p):
            for root, dirs, files in os.walk(p):
                files_toprocess.update([os.path.join(root, f) for f in files])

    # process bin only
    files_proc = []
    for f in files_toprocess:
        if f.endswith('.bin') or (f.endswith('.b') and '.b' in f):
            files_proc.append(f)
    
    if not files_proc:
        print("No binary files found in {}".format(temp1))
        exit()

    for i in files_proc:
        newf, newb = create_filenames(i)
        print("Executing python bin2txt.py {} NvTpl.txt".format(newb))

        b2t.main(["", newb, bdf_template, newf])

    print("\n\nSuccessfully generated txt files of given binaries.")

if __name__ == "__main__":
    main(sys.argv)