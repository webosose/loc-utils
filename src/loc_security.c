// Copyright (c) 2020 LG Electronics, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// SPDX-License-Identifier: Apache-2.0

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
    strncat(clrstr, (const char*)out, 4);

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
    unsigned char decoderinput[4] = "";
    FILE *inFile = NULL;
    char encodingtabe[TABLELEN + 2] = BASE64CHARSET;

    if (!file_path)
        return LOC_SECURITY_ERROR_FAILURE;

    inFile = fopen(file_path, "rt");

    if (inFile == NULL)
        return LOC_SECURITY_ERROR_FAILURE;

    fseek(inFile, 0L, SEEK_END);
    cipherLen = ftell(inFile);
    if (cipherLen < 0)
        goto CLEANUP;

    rewind(inFile);

    /* allocate memory for entire content */
    cipher = calloc(1, cipherLen + 1);
    if (!cipher)
        goto CLEANUP;

    /* copy the file into the buffer  */
    if (1 != fread(cipher, cipherLen, 1, inFile))
        goto CLEANUP;

    cipher[sizeof(cipher)-1] = '\0';
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

    *decrypted = (unsigned char *)strdup(clrdst);

    err = LOC_SECURITY_ERROR_SUCCESS;


CLEANUP:

    if (cipher)
        free(cipher);

    if (inFile != NULL)
        fclose(inFile);

    return err;

}


