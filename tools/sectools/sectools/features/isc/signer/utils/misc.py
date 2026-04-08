#===============================================================================
# Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
# SPDX-License-Identifier: BSD-3-Clause-Clear
#===============================================================================


def get_signing_config_overrides():
    retval = None
    try:
        from sectools.common.engg.config_overrides import get_config_overrides
        retval = get_config_overrides()
    except ImportError:
        pass
    return retval

