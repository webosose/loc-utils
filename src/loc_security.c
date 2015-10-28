/*
 * Copyright (c) 2015 LG Electronics Inc. All Rights Reserved.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <loc_security.h>

#define GEOLOCKEY_CONFIG_PATH       "/etc/geolocation.conf"
#define GEOCODEKEY_CONFIG_PATH      "/etc/geocode.conf"
#define LBS_KEY "CDgvGzYI33BE0XvF54xrqaDmBrY="
#define GEOLOCATION_KEY "AIzaSyBoS6t0AeIuEYkPcmIsK_bBAVFOWFnoLsw"

int loc_security_openssl_decrypt_file(const char *file_path, unsigned char **decrypted)
{
    if(!strcmp(file_path,GEOLOCKEY_CONFIG_PATH))
        *decrypted = strdup(GEOLOCATION_KEY);

    else if(!strcmp(file_path,GEOCODEKEY_CONFIG_PATH))
        *decrypted = strdup(LBS_KEY);

    else
        return LOC_SECURITY_ERROR_FAILURE;

    if(*decrypted == NULL)
        return LOC_SECURITY_ERROR_FAILURE;

    return LOC_SECURITY_ERROR_SUCCESS;
}
