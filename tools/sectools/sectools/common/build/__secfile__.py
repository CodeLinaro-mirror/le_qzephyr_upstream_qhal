#===============================================================================
# Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
# SPDX-License-Identifier: BSD-3-Clause-Clear
#===============================================================================

import os

DIR_PATH = None

def init():
    global DIR_PATH
    DIR_PATH = os.path.dirname(os.path.abspath(__file__))

