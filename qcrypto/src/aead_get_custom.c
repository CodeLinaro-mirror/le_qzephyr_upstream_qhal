/**
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause
 */
 
#if CONFIG_SECURE_STORAGE_ITS_IMPLEMENTATION_ZEPHYR
#include <zephyr/secure_storage/its/transform/aead_get.h>
#include <zephyr/drivers/hwinfo.h>
#include <zephyr/init.h>
#include <psa/crypto.h>
#include <string.h>
#include <sys/types.h>
#include <mbedtls/platform_util.h>
#include "CeML.h"

#define USER_PASSWORD_SIZE 16
uint8_t g_securefs_password[USER_PASSWORD_SIZE] = {
	0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
	0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff
};

#ifdef CONFIG_SECURE_STORAGE_ITS_TRANSFORM_AEAD_KEY_PROVIDER_CUSTOM
psa_status_t secure_storage_its_transform_aead_get_key(
		secure_storage_its_uid_t uid,
		uint8_t key[static CONFIG_SECURE_STORAGE_ITS_TRANSFORM_AEAD_KEY_SIZE])
{
	psa_status_t ret;
	uint32 aes_key[4] = {0x0};
	CeMLErrorType err_code = CEML_ERROR_SUCCESS;
	CeMLCntxHandle *cipher_cntx = NULL;

	/*deriving a cipher key from HW key*/
	ret = CeML_hw_kdf(cipher_cntx, CEML_KDF_SECURE_STORAGE,
					(void *)(g_securefs_password), USER_PASSWORD_SIZE,
					0, aes_key, sizeof(aes_key) / sizeof(aes_key[0]));

	if (ret != CEML_ERROR_SUCCESS) {
		return PSA_ERROR_HARDWARE_FAILURE;
	}

	/* set key to all zero to enable KDF key in CCM encryption */
	memset(key, 0, CONFIG_SECURE_STORAGE_ITS_TRANSFORM_AEAD_KEY_SIZE);

	return ret;
}
#endif /* CONFIG_SECURE_STORAGE_ITS_TRANSFORM_AEAD_KEY_PROVIDER_CUSTOM */

#ifdef CONFIG_SECURE_STORAGE_ITS_TRANSFORM_AEAD_SCHEME_CUSTOM
#define PSA_KEY_TYPE PSA_KEY_TYPE_AES
#define PSA_ALG PSA_ALG_CCM
void secure_storage_its_transform_aead_get_scheme(psa_key_type_t *key_type, psa_algorithm_t *alg)
{
	*key_type = PSA_KEY_TYPE;
	*alg = PSA_ALG;
}
#endif /* CONFIG_SECURE_STORAGE_ITS_TRANSFORM_AEAD_SCHEME_CUSTOM */
#endif /* CONFIG_SECURE_STORAGE_ITS_IMPLEMENTATION_ZEPHYR */