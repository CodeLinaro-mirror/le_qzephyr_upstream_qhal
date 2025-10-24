## system imports
import os
import sys
import argparse
import shutil

## custom imports
import bdf_multi_bin_txt as bdf_mul

def main(args):
    input_dir = args.DIR

    if not os.path.exists("NvTpl.txt"):
        print("Please place NvTpl.txt in current directory and retry.")
        sys.exit()
    
    ## Main function
    bdf_mul.main(["", input_dir])
    
    print("\nAll txt files are generated in {} directory.".format(input_dir))

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("DIR", help="Directory contains all BDF binary files")    
    args = parser.parse_args()
    main(args)