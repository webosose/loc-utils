/*
 * Copyright (c) 2015 LG Electronics Inc. All Rights Reserved.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <loc_security.h>

#define GEOLOCKEY_CONFIG_PATH       "/etc/geolocation.conf"
#define GEOCODEKEY_CONFIG_PATH      "/etc/geocode.conf"

/* ---- Base64 Encoding/Decoding Table --- */
#define BASE64CHARSET   "ABCDEFGHIJKLMNOPQ!@#$%WXYZ"\
                        "abcdefghijklmnopqrstuvwxyz"\
                        "0123456789"\
                        "+/";
#define TABLELEN        63
#define PADDINGCHAR     '='

/* decodeblock - decode 4 '6-bit' characters into 3 8-bit binary bytes */
void decodeblock(unsigned char in[], char *clrstr)
{
    unsigned char out[4];
    out[0] = in[0] << 2 | in[1] >> 4;
    out[1] = in[1] << 4 | in[2] >> 2;
    out[2] = in[2] << 6 | in[3] >> 0;
    out[3] = '\0';
    strncat(clrstr, out, sizeof(out));

}

int locSecurityBase64Decode(const char *file_path, unsigned char **decrypted)
{
    int asciival = 0;
    int phase = 0;
    int index = 0;
    int cipherLen = 0;
    int err = LOC_SECURITY_ERROR_FAILURE;
    char *charval = 0;
    unsigned char *cipher = NULL;
    char clrdst[1024] = "";
    unsigned char decoderinput[4];
    FILE *inFile = NULL;
    char encodingtabe[TABLELEN + 1] = BASE64CHARSET;

    if (!file_path)
        return LOC_SECURITY_ERROR_FAILURE;

    inFile = fopen(file_path, "rt");

    if (inFile == NULL)
        return LOC_SECURITY_ERROR_FAILURE;

    fseek(inFile, 0L, SEEK_END);
    cipherLen = ftell(inFile);
    rewind(inFile);

    /* allocate memory for entire content */
    cipher = calloc(1, cipherLen + 1);
    if (!cipher)
        goto CLEANUP;

    /* copy the file into the buffer  */
    if (1 != fread(cipher, cipherLen, 1, inFile))
        goto CLEANUP;

    clrdst[0] = '\0';
    phase = 0;
    index=0;
    while(cipher[index]) {

        asciival = (int) cipher[index];
        if(PADDINGCHAR == asciival) {
            decodeblock(decoderinput, clrdst);
            break;
        }
     charval = strchr(encodingtabe, asciival);
     if(charval) {
            decoderinput[phase] = charval - encodingtabe;
            phase = (phase + 1) % 4;
            if(phase == 0) {
                decodeblock(decoderinput, clrdst);
          decoderinput[0]=decoderinput[1]=decoderinput[2]=decoderinput[3]=0;
            }
        }
     index++;
    }//end of while

    *decrypted = strdup(clrdst);

    err = LOC_SECURITY_ERROR_SUCCESS;


CLEANUP:

    if (cipher)
        free(cipher);

    if (inFile != NULL)
        fclose(inFile);

    return err;

}


