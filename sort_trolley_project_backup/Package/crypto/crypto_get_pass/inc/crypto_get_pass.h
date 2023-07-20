#ifndef __CRYPTO_GET_PASS_H_
#define __CRYPTO_GET_PASS_H_

#include "stdint.h"

#define BLOCK_SIZE 64
#define SHA256_DIGEST_LENGTH 32

char *generate_username(uint32_t tick, const char *clientid);

char *generate_clientid(const char *product_name, const char *device_name);

char *generate_password(char *product_name, char *device_name, char *secure_code);

#endif

