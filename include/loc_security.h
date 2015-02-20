/*
 * Copyright (c) 2015 LG Electronics Inc. All Rights Reserved.
 */

#ifndef _LOC_SECURITY_H_
#define _LOC_SECURITY_H_

#ifdef __cplusplus
extern "C" {
#endif

#define LOC_SECURITY_ERROR_SUCCESS      0
#define LOC_SECURITY_ERROR_FAILURE      -1


// OPENSSL - Decryption
int loc_security_openssl_decrypt_file(const char *file_path, unsigned char **decrypted);



#ifdef __cplusplus
}
#endif

#endif
