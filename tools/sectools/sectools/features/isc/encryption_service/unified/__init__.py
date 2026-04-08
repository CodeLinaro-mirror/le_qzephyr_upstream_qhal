#===============================================================================
# Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
# SPDX-License-Identifier: BSD-3-Clause-Clear
#===============================================================================

from . import encdec
# Import built-in encrypted key providers
from sectools.features.isc.encryption_service.unified.encrypted_key_provider import ServerEncryptedKeyProvider

# Try to import oem-defined encrypted key providers
try:
    from plugin import encrypted_key_provider
except ImportError:
    pass
