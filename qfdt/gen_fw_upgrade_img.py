#!/usr/bin/python
#===============================================================================
# Copyright (c) 2024 Qualcomm Innovation Center, Inc. All rights reserved.
# SPDX-License-Identifier: BSD-3-Clause-Clear
#===============================================================================

import struct
import xml.etree.ElementTree as ET
import xml.dom.minidom as minidom
import math
import hashlib
import logging
import os
import sys

class Fw_Upgrade_Img_Descriptor_Entry:
    ''' Firmware Upgrade Image Descriptor Entry, stores the data and translates it into
    binary

    Firmware Upgrade Image  Descriptor Entries
    Little endian         <
    uint32 Signature      I
    uint32 Image ID       I
    uint32 ver            I
    char   filename       s*128
    uint32 disk size      I
    uint32 image_lenth    I
    uint32 HASH_TYPE      I
    uint8  HASH           B*32
    '''

    # size of a single firmware descriptor entry = (6 * 4) + 128 + 32.
    FWD_ENTRIES_FORMAT = "<I I I 128s I I I"
    fde_size = 184
    fde_packed = struct.Struct(FWD_ENTRIES_FORMAT)

    def __init__ (self):
        ''' Create an empty Firware Partition Entry '''
        self.signature = 0
        self.image_id = 0
        self.ver = 0
        self.filename = ""
        self.disk_size = 0
        self.image_len = 0
        self.hash_type = 0
        self.hash = bytearray([0x00]*32)

    def update_image_len (self, image_len):
        ''' Update the block size used for the entry. Changing the block size
        modifies the size_in_block of the image '''
        # Check that the input is an integer
        if not isinstance(image_len, int):
            raise AssertionError ("The input block size isn't an integer")

        self.image_len = image_len
        if self.disk_size < image_len:
            self.disk_size = image_len

    def clear_filename (self):
        self.filename = ""

    def update_disk_size (self, disk_size):
        ''' Update the size_in_block for the entry. '''
        # Check that the input is an integer
        if not isinstance(disk_size, int):
            raise AssertionError ("The disk size isn't an integer")

        if self.disk_size < disk_size:
            self.disk_size = disk_size

    def update_hash(self, data):
        m = hashlib.sha256()
        m.update(data)
        self.hash = m.digest()
        return

    def to_binary (self):
        ''' Convert the firmware descriptor entry into a packed binary
        form '''
        data = self.fde_packed.pack(self.signature,
                                    self.image_id,
									self.ver,
									self.filename.encode(),
                                    self.disk_size,
									self.image_len,
									self.hash_type)
        data = data + self.hash
        return data;

    def from_xml (self, xml_root):
        ''' Parses the XML Root from an ElementTree, the XML data should
        look like:
		<partition filename="ioe_ram_m4_free_rtos.mbn" signature="0x54445746" image_id="10" ver="1" HASH_TYPE="1"/>
        '''
        if xml_root.tag != 'partition':
            raise AssertionError("Trying to parse something that is not a partition." % (size))

        self.image_id = int(xml_root.attrib['image_id'], 0)
        self.ver = int(xml_root.attrib['ver'], 0)
        self.filename = xml_root.attrib['filename']
        self.disk_size = int(xml_root.attrib['size_in_kb'], 0) * 1024
        self.signature = int(xml_root.attrib['signature'], 0)
        self.hash_type = int(xml_root.attrib['HASH_TYPE'], 0)

        if self.image_id == 0:
            print('0 is not valid image id')
            rtn = 0
        if self.ver == 0:
            print('0 is not valid image version')
            rtn = 0
        if self.signature == 0:
            print('0 is not valid signature')
            rtn = 0
        if self.hash_type == 0:
            print('0 is not valid hash type')
            rtn = 0
        rtn = 1;

        return rtn

class Fw_Upgrade_Img_Descriptor:
    ''' Definition of one Firmware Upgrade Image Header
    Little endian        <
    uint32 sig          I
	uint32 ver          I
	uint32 format       I
    uint32 image_len    I
    uint8  num_images   B
	IMG_ENTRY
	....
	IMG_ENTRY
	uint8 HASH          B*32
    '''
    Fw_Upgrade_Img_HEADER = '<IIIIB'
    fdt_packed = struct.Struct(Fw_Upgrade_Img_HEADER)

    def __init__ (self):
        ''' Initalizes the Firmware Upgrade Image Descriptor'''

        self.signature = 0
        self.version = 0
        self.format = 1
        self.image_len = 0
        self.num_images = 0
        self.entries = []
        self.hash = [0x00]*32

    def check_parameters(self):
        rtn = 1
        if self.version == 0:
            print('version number is not correct, check version setting at header')
            rtn = 0
        if self.format == 0:
            print('format number is not correct, check format setting at header')
            rtn = 0
        if self.signature == 0:
            print('signature is not correct, check signature setting at header')
            rtn = 0
        if self.num_images == 0:
            print('number of images is zero, check partiton setting at XML file')
            rtn = 0
        return rtn

    def update_image_len (self, image_len):
        ''' Update the block size used for the entry. Changing the block size
        modifies the size_in_block of the image '''
        # Check that the input is an integer
        if not isinstance(image_len, int):
            raise AssertionError ("The input block size isn't an integer")

        self.image_len = self.image_len + image_len

    def add_entry (self, entry):
        self.entries.append(entry)
        self.num_images = self.num_images + 1

    def to_binary (self):
        ''' Convert a Fw_Upgrade_Img_Descriptor class into a binary data stream
        that can be saved directly into a file. '''

        data = bytearray()
        data = data + self.fdt_packed.pack(self.signature,
                                    self.version,
                                    self.format,
                                    self.image_len,
                                    self.num_images
                                    )
        for entry in self.entries:
            data = data + entry.to_binary()

        '''add hash'''
        data = data + bytearray(self.hash)
        return data

    def update_hash(self):
        data = bytearray()
        data = data + self.fdt_packed.pack(self.signature,
                                    self.version,
                                    self.format,
                                    self.image_len,
                                    self.num_images
                                    )
        for entry in self.entries:
            data = data + entry.to_binary()

        m = hashlib.sha256()
        m.update(data)
        self.hash = m.digest()
        return

    def from_xml_file (self, xml_file):
        ''' Parses the XML Root from an ElementTree, the XML data should
        look like:
        <fw_upgrade_img_descriptor>
        <header signature="0x54445746" version="1"/>
        <partition filename="ioe_ram_m4_free_rtos.mbn" signature="0x54445746" image_id="10" ver="1" HASH_TYPE="1"/>
        <partition filename="ioe_kf.bin" signature="0x54445746" image_id="11" ver="1" HASH_TYPE="1"/>
        <partition filename="ioe_ram_m0_free_rtos.mbn" signature="0x54445746" image_id="12" ver="1" HASH_TYPE="1"/>
        </fw_upgrade_img_descriptor>
        '''

        # Clear any pre-existing data.
        self.__init__()

        xml = ET.parse(xml_file)
        fw_descriptor = xml.getroot()
        if fw_descriptor.tag != 'fw_upgrade_img_descriptor':
            print("XML didn't start with the correct tag <fw_upgrade_img_descriptor>")
            return 0

        for child in fw_descriptor:
            if child.tag == 'header':
                self.signature = int(child.attrib['signature'], 0)
                self.version = int(child.attrib['version'], 0)
                self.format = int(child.attrib['format'], 0)

            elif child.tag == 'partition':
                entry = Fw_Upgrade_Img_Descriptor_Entry()
                if entry.from_xml(child) != 1:
                    print("parameter setting is not correct at Firmware Upgrade Image Descriptor entry")
                    return 0
                self.add_entry(entry)
        return 1

    def search_file_in_build_dir(self, filename, build_dir, board=None):
        ''' Search for a file in the build directory
        
        Args:
            filename: The filename to search for (e.g., zephyr_HASHED.elf)
            build_dir: The build directory to search in
            board: Optional board name for SBL files
            
        Returns:
            Full path to the file if found, otherwise the original filename
        '''
        if not build_dir or not os.path.exists(build_dir):
            return filename
        
        # For SBL files, construct the expected path based on board name
        if 'sbl' in filename.lower() and board:
            # Expected path: build_dir/modules/hal_qcom/qboot/zephyr/{board}_sbl_HASHED.elf
            sbl_path = os.path.join(build_dir, 'modules', 'hal_qcom', 'qboot', 'zephyr', f'{board}_sbl_HASHED.elf')
            if os.path.exists(sbl_path):
                logging.info(f'Found SBL file: {sbl_path}')
                return sbl_path
        
        # For zephyr_HASHED.elf, search in build_dir/zephyr/
        if 'zephyr_HASHED.elf' in filename:
            zephyr_path = os.path.join(build_dir, 'zephyr', 'zephyr_HASHED.elf')
            if os.path.exists(zephyr_path):
                logging.info(f'Found zephyr file: {zephyr_path}')
                return zephyr_path
        
        # If not found in expected locations, try recursive search
        for root, dirs, files in os.walk(build_dir):
            if filename in files:
                found_path = os.path.join(root, filename)
                logging.info(f'Found file via search: {found_path}')
                return found_path
        
        logging.warning(f'File not found in build directory: {filename}')
        return filename

    def gen_whole_disk_binary(self, filename, build_dir=None, board=None):
        ''' Write to the given file name a copy of the binary blob contained
        in the partition table. Note: All files must be in current folder.
        
        Args:
            filename: Output filename
            build_dir: Optional build directory to search for ELF files
            board: Optional board name for SBL file resolution
        '''

        out = open(filename, 'wb')

        #file already pre-filled with 0xFF, add the tables
        out.write(self.to_binary())

        #update image length and hash at Firmware Upgrade Image Header
        out.seek(0,2)
        header_len = out.tell()
        self.update_image_len(header_len)

        if self.format == 1: # partial format
            for entry in self.entries:
                if len(entry.filename) > 0:
                    # Search for file in build directory if provided
                    # Use image_id to determine which file to use:
                    # image_id 1 = SBL, image_id 10 = zephyr application
                    actual_filename = entry.filename
                    file_replaced = False
                    if build_dir and os.path.exists(build_dir):
                        if entry.image_id == 1 and board:
                            # SBL file
                            sbl_path = os.path.join(build_dir, 'modules', 'hal_qcom', 'qboot', 'zephyr', f'{board}_sbl_HASHED.elf')
                            if os.path.exists(sbl_path):
                                actual_filename = sbl_path
                                file_replaced = True
                                logging.info(f'Using SBL file from build_dir for image_id {entry.image_id}: {sbl_path}')
                        elif entry.image_id == 10:
                            # Zephyr application file
                            zephyr_path = os.path.join(build_dir, 'zephyr', 'zephyr_HASHED.elf')
                            if os.path.exists(zephyr_path):
                                actual_filename = zephyr_path
                                file_replaced = True
                                logging.info(f'Using zephyr file from build_dir for image_id {entry.image_id}: {zephyr_path}')
                    
                    if not file_replaced and len(entry.filename) > 0:
                        logging.info(f'Using original XML path for image_id {entry.image_id}: {entry.filename}')
                    
                    logging.debug('Will try to open file %s' % (actual_filename))
                    try:
                        with open(actual_filename , 'rb') as f:
                            # get file size
                            f.seek(0,2)
                            size = f.tell()

                            # Append 0xFF to each image before calculating hash to make total disk size a multiple of 4KB.
                            # Copy data from input file and appended bytes into a temporary file.
                            temp = open('temp_file', 'wb+')
                            f.seek(0,0)
                            data = f.read(size)
                            temp.write(data)
                            total_size = size
                            if (size % 4096) != 0:
                                size_0xff = 4096 - (size % 4096)
                                total_size = total_size + size_0xff
                                temp.write(bytearray([0xFF] * size_0xff))

                            # Read data from temporary file to update hash, image length and disk size at partition entry.
                            temp.seek(0,0)
                            total_data = temp.read(total_size)
                            entry.update_hash(total_data)
                            entry.update_image_len(size)
                            entry.update_disk_size(total_size)

                            #close the temporary file and delete it.
                            temp.close()
                            os.remove('temp_file')

                            #close input file
                            f.close()
                    except IOError as e:
                        logging.exception("Unable to open the file '%s'\n" % (entry.filename))
                        print("Can't open file %s" % (entry.filename))
                        return 0

        else: # format with all in one
            for entry in self.entries:
                if len(entry.filename) > 0:
                    # Search for file in build directory if provided
                    # Use image_id to determine which file to use:
                    # image_id 1 = SBL, image_id 10 = zephyr application
                    actual_filename = entry.filename
                    file_replaced = False
                    if build_dir and os.path.exists(build_dir):
                        if entry.image_id == 1 and board:
                            # SBL file
                            sbl_path = os.path.join(build_dir, 'modules', 'hal_qcom', 'qboot', f'{board}_sbl_HASHED.elf')
                            if os.path.exists(sbl_path):
                                actual_filename = sbl_path
                                file_replaced = True
                                logging.info(f'Using SBL file from build_dir for image_id {entry.image_id}: {sbl_path}')
                        elif entry.image_id == 10:
                            # Zephyr application file
                            zephyr_path = os.path.join(build_dir, 'zephyr', 'zephyr_HASHED.elf')
                            if os.path.exists(zephyr_path):
                                actual_filename = zephyr_path
                                file_replaced = True
                                logging.info(f'Using zephyr file from build_dir for image_id {entry.image_id}: {zephyr_path}')
                    
                    if not file_replaced and len(entry.filename) > 0:
                        logging.info(f'Using original XML path for image_id {entry.image_id}: {entry.filename}')
                    
                    logging.debug('Will try to open file %s' % (actual_filename))
                    try:
                        with open(actual_filename , 'rb') as f:
                            # get file size
                            f.seek(0,2)
                            size = f.tell()
                            #read data from input file.
                            f.seek(0,0)
                            data = f.read(size)

                            #place it on the output file
                            out.write(data)

                            #update image len at partion entry
                            entry.update_image_len(size)

                            #update HASH at parttion entry
                            entry.update_hash(data)

                            logging.debug('Read %d bytes from input, out of %d' % (len(data), size))

                            #close input file
                            f.close()

                            #clear fileanme
                            entry.clear_filename()
                    except IOError as e:
                        logging.exception("Unable to open the file '%s'\n" % (entry.filename))
                        print("Can't open file %s" % (entry.filename))
                        return 0

        #update HASH at Firmware Upgrade Image Header HASH
        self.update_hash()

        #write final Firmware Upgrade Image Header
        out.seek(0,0)
        out.write(self.to_binary())

        #close output fiel
        out.close()
        return 1

def main():

    # Give a version number to this script.
    script_version = '1.0'

    # Check the version, not support python2.
    if sys.version_info[0] < 3:
        logging.critical ("This tool should run on Python 3.")
        return

    import argparse

    tool_verbose_description = """Tool to generate intermediate files for flashing a Firmware Upgrade Image Descriptor Table and
related images into disk.

Example Usage:
Run: python gen_fw_upgrade_img.py --xml fw_upgrade.xml --output fw_upgrade_img.bin

"""

    parser = argparse.ArgumentParser(description=tool_verbose_description, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument('--xml', type= open, required=True, help='The xml file for the firmware upgrade image generater')
    parser.add_argument('--output', type=str, required=False, help='The output file where to store the whole disk')
    parser.add_argument('-d', '--build-dir', type=str, required=False, help='Build directory to search for ELF files (e.g., qcc730mi_sbl_HASHED.elf, zephyr_HASHED.elf)')
    parser.add_argument('-b', '--board', type=str, required=False, help='Board name (e.g., qcc730mi, qcc730evbx) to determine SBL filename')
    parser.add_argument('-v', '--verbose', type=int, choices=[0,1,2,3,4,5], help='Verbose levels. Higher numbers include lower. For example, 3 means 3,2,1 and 0. 0=Critcal. 1=Error, 2=Warning 3=Info[Default], 4=Debug, 5=Everything', default=0)
    args = parser.parse_args()

    log_level = [logging.CRITICAL, logging.ERROR, logging.WARNING, logging.INFO, logging.DEBUG, logging.NOTSET]
    logging.basicConfig(format='[%(asctime)-15s] %(levelname)s: %(message)s File:%(filename)s:%(lineno)d Function:%(funcName)s ', level=log_level[args.verbose])

    if args.output is None:
        logging.error ('No output specified, please use --output to specify the output file')
        print('No output specified, please use --output to specify the output file')
        return

    #read the intput XML file.
    fwd = Fw_Upgrade_Img_Descriptor()
    if fwd.from_xml_file (args.xml.name) != 1:
        logging.error ('entry setting at XML are not correct')
        print('Failed to generate the whole firmware upgrade image')
        return

    #check input parameters
    if fwd.check_parameters() != 1:
        logging.error ('parameters at XML are not correct')
        print('Failed to generate the whole firmware upgrade image')
        return

    #Generate the disk image.
    if fwd.gen_whole_disk_binary(args.output, args.build_dir, args.board) == 1:
        #done here
        logging.info('Done generating the whole firmware upgrade image')
        print('Done generating the whole firmware upgrade image')
    else:
        #failed here
        logging.info('Failed to generate the whole firmware upgrade image')
        print('Failed to generate the whole firmware upgrade image')

    return

if __name__ == "__main__":
    main()
#    import pdb; pdb.set_trace()
