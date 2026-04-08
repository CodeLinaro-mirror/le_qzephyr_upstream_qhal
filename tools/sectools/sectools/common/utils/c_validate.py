#===============================================================================
# Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
# SPDX-License-Identifier: BSD-3-Clause-Clear
#===============================================================================

def range_check(name, value, min, max):
    if value > max:
        raise RuntimeError("%r cannot be larger than %d" % (name, max))
    elif value < min:
        raise RuntimeError("%r cannot be less than %d" % (name, min))
