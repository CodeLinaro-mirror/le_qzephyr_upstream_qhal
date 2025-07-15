#!/usr/bin/env python3
#===============================================================================
# Copyright (c) 2024 Qualcomm Innovation Center, Inc. All rights reserved.
# SPDX-License-Identifier: BSD-3-Clause
#===============================================================================

import os
import sys

if len(sys.argv) < 2:
    print("Please input output path!!!")
    sys.exit(-1)

nt_devcfg_header_output_dir = sys.argv[1]
current_file_path = os.path.dirname(os.path.abspath(__file__))
master_xml_path = os.path.join(current_file_path, "master_xml.xml")
net_devcfg_path = os.path.join(nt_devcfg_header_output_dir, "nt_devcfg.h")
net_devcfg_structure_path = os.path.join(nt_devcfg_header_output_dir, "nt_devcfg_structure.h")

# remove system generated files, if they already exist
if os.path.exists(net_devcfg_path):
    os.remove(net_devcfg_path)

if os.path.exists(net_devcfg_structure_path):
    os.remove(net_devcfg_structure_path)

def append_files(input_f, output_f):
    file = open(input_f, "r")
    data = file.read()
    file.close()

    fout = open(output_f, "a")
    fout.write(data)
    fout.close()


def append_text(text, input_f):
    fout = open(input_f, "a")
    fout.write(text)
    fout.close()

# code to generate header files
output_list1 = []
output_list2 = []
count = 1
count1 = 1

append_text("/** System Generated File\n", net_devcfg_path)

append_text("*  Don't Change Manually */\n", net_devcfg_path)

append_text("#ifndef CORE_DEV_CFG_EXPORT_NT_DEVCFG_H_\n", net_devcfg_path)
append_text("#define CORE_DEV_CFG_EXPORT_NT_DEVCFG_H_\n", net_devcfg_path)
append_text("typedef enum nt_devcfg_id_s \n", net_devcfg_path)
append_text("{\n", net_devcfg_path)

append_text("/** System Generated File\n", net_devcfg_structure_path)
append_text("*  Don't Change Manually */\n", net_devcfg_structure_path)
append_text("#ifndef CORE_DEV_CFG_INC_NT_DEVCFG_STRUCTURE_H_\n", net_devcfg_structure_path)
append_text("#define CORE_DEV_CFG_INC_NT_DEVCFG_STRUCTURE_H_\n", net_devcfg_structure_path)
append_text('#include "nt_devcfg_types.h" \n', net_devcfg_structure_path)
append_text('#include "nt_devcfg_def.h"\n', net_devcfg_structure_path)
append_text("void nt_devcfg_parse(); \n", net_devcfg_structure_path)
append_text("typedef struct nt_devcfg_structure_s \n", net_devcfg_structure_path)
append_text("{\n", net_devcfg_structure_path)


fhand = open(master_xml_path)
for line in fhand:
    line = line.strip()
    split_line = line.split()

    if len(split_line) > 2:
        if split_line[2].split("=")[0] == "id_name":
            if split_line[2].split("=")[1][-1] == '"':
                flag = split_line[2].split("=")[1][1:-1]
            else:
                flag = split_line[2].split("=")[1][1:]
            text = "    {} = {},\n".format(flag, count)

            if flag not in output_list1:
                count += 1
                output_list1.append(flag)
                append_text(text, net_devcfg_path)

            text2 = "    uint32 {};\n".format(flag)

            if text2 not in output_list2:
                output_list2.append(text2)
                append_text(text2, net_devcfg_structure_path)

append_text("} nt_devcfg_id_t; \n", net_devcfg_path)
append_text(
    "void* nt_devcfg_get_config(int enum_id);        // callback function for unit value \n",
    net_devcfg_path,
)
append_text(
    "void* nt_devcfg_ascii_config(int enum_id);        // callback function for ascii value \n",
    net_devcfg_path,
)
append_text("#endif /* CORE_DEV_CFG_EXPORT_NT_DEVCFG_H_ */ \n", net_devcfg_path)


append_text("} nt_devcfg_structure_t; \n", net_devcfg_structure_path)
append_text("#endif /* CORE_DEV_CFG_INC_NT_DEVCFG_STRUCTURE_H_ */\n", net_devcfg_structure_path)
