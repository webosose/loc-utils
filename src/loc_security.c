/*
 * Copyright (c) 2015 LG Electronics Inc. All Rights Reserved.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/evp.h>
#include <openssl/conf.h>
#include <openssl/err.h>
#include <loc_security.h>


int loc_security_openssl_decrypt_file(const char *file_path, unsigned char **decrypted)
{
    unsigned char *cipher = NULL;
    int decrypted_len = 0;
    int cipher_len = 0;
    int len = 0;
    int err = LOC_SECURITY_ERROR_FAILURE;
    EVP_CIPHER_CTX *ctx = NULL;
    FILE *in = NULL;

    if (!file_path)
        return LOC_SECURITY_ERROR_FAILURE;

    in = fopen(file_path, "rt");
    if (in == NULL)
        return LOC_SECURITY_ERROR_FAILURE;

    /* Initialise the library */
    ERR_load_crypto_strings();
    OpenSSL_add_all_algorithms();
    OPENSSL_config(NULL);

    fseek(in, 0L, SEEK_END);
    cipher_len = ftell(in);
    rewind(in);

    /* allocate memory for entire content */
    cipher = calloc(1, cipher_len + 1);
    if (!cipher)
        goto CLEANUP;

    /* copy the file into the buffer */
    if (1 != fread(cipher, cipher_len, 1, in))
        goto CLEANUP;

    /* Create and initialise the context */
    if (!(ctx = EVP_CIPHER_CTX_new()))
        goto CLEANUP;

    if (1 != EVP_DecryptInit_ex(ctx,
                                EVP_aes_256_cbc(),
                                NULL,
                                (unsigned char *)"9876543210ABCDEFGHIJ@H&G_P@)901",
                                (unsigned char *)"Zcbmkgdrl_$^&opqs"))
        goto CLEANUP;

    /* Provide the message to be decrypted, and obtain the plaintext output.*/
    *decrypted = (unsigned char *)malloc(sizeof(unsigned char) * (cipher_len + 1));
    if (*decrypted == NULL)
        goto CLEANUP;

    if (1 != EVP_DecryptUpdate(ctx, *decrypted, &len, cipher, cipher_len))
        goto CLEANUP;

    decrypted_len = len;

    /* Finalise the decryption. Further plaintext bytes may be written at
     * this stage.
     */
    if (1 != EVP_DecryptFinal_ex(ctx, *decrypted + len, &len))
        goto CLEANUP;

    decrypted_len += len;

    /* Add a NULL terminator*/
    *(*decrypted + decrypted_len) = '\0';

    err = LOC_SECURITY_ERROR_SUCCESS;

CLEANUP:
    /* Clean up */
    if (ctx)
        EVP_CIPHER_CTX_free(ctx);

    EVP_cleanup();
    ERR_free_strings();

    if (cipher)
        free(cipher);

    if (in != NULL)
        fclose(in);

    return err;
}
