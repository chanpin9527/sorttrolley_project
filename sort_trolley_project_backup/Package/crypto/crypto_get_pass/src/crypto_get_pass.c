#include "crypto_get_pass.h"
#include "stdio.h"
#include "string.h"
#include "sha256.h"

#define CRYPTO_BUFF_SIZE	64
static char s_username[CRYPTO_BUFF_SIZE] = {0};
static char s_clientid[CRYPTO_BUFF_SIZE] = {0};
static char s_password[CRYPTO_BUFF_SIZE+1] = {0};

char *generate_username(uint32_t tick, const char *clientid) 
{
	if(clientid == NULL)
		return NULL;
	memset(s_username, 0x00, CRYPTO_BUFF_SIZE);
	sprintf(s_username, "%d@0@%s", tick, clientid);
	
	return s_username;
	//最终返回 1680169178345@1@vehicle_abc_01@auto_distribute
}
char *generate_clientid(const char *product_name, const char *device_name) 
{
	if((product_name == NULL) || (device_name == NULL))
		return NULL;
	memset(s_clientid, 0x00, CRYPTO_BUFF_SIZE);
	sprintf(s_clientid, "%s@%s", device_name, product_name);

	return s_clientid;
	//最终返回 vehicle_abc_01@auto_distribute
}

void sha256(unsigned char *input, int len, unsigned char *output) {
    SHA256_CTX ctx;
    sha256_init(&ctx);
    sha256_update(&ctx, input, len);
    sha256_final(&ctx,output);
}

void hmac_sha256(unsigned char *key, int key_len, unsigned char *data, int data_len, unsigned char *result) 
{
    unsigned char key_padded[BLOCK_SIZE];
    unsigned char o_key_pad[BLOCK_SIZE];
    unsigned char i_key_pad[BLOCK_SIZE];
    unsigned char tmp_key[SHA256_DIGEST_LENGTH];
    unsigned char inner_hash[SHA256_DIGEST_LENGTH + BLOCK_SIZE];
    unsigned char outer_hash[SHA256_DIGEST_LENGTH + BLOCK_SIZE];

    if (key_len > BLOCK_SIZE) {
        sha256(key, key_len, tmp_key);
        key = tmp_key;
        key_len = SHA256_DIGEST_LENGTH;
    }

    memcpy(key_padded, key, key_len);
    memset(key_padded + key_len, 0, BLOCK_SIZE - key_len);

    for (int i = 0; i < BLOCK_SIZE; i++) {
        o_key_pad[i] = key_padded[i] ^ 0x5c;
        i_key_pad[i] = key_padded[i] ^ 0x36;
    }

    memcpy(inner_hash, i_key_pad, BLOCK_SIZE);
    memcpy(inner_hash + BLOCK_SIZE, data, data_len);

    sha256(inner_hash, BLOCK_SIZE + data_len, result);

    memcpy(outer_hash, o_key_pad, BLOCK_SIZE);
    memcpy(outer_hash + BLOCK_SIZE, result, SHA256_DIGEST_LENGTH);

    sha256(outer_hash, BLOCK_SIZE + SHA256_DIGEST_LENGTH, result);
}

char value2hex(const int value)
{
    char hex = 0;
    if((value >= 0) && (value <= 9))
    {
        hex = (char)(value + '0');
    }
    else if((value > 9) && (value <16))
    {
        hex = (char)(value - 10 + 'a');
    }
    return hex;
}

int str2hex(char *str,char *hex)
{
    uint8_t high = 0, low = 0, temp = 0;
    if((str == NULL) || (hex == NULL))
    {
        return -1;
    }
    if(strlen(str) == 0)
    {
        return -2;
    }
    
    while(*str)
    {
        temp = (uint8_t)(*str);
        high = temp >> 4;
        low = temp & 0x0f;
        *hex = value2hex(high);
        hex ++;
        *hex = value2hex(low);
        hex ++;
        str ++;
    }
    *hex = '\0';
    return 0;
}

char *generate_password(char *product_name, char *device_name, char *secure_code)
{
	unsigned char result[SHA256_DIGEST_LENGTH+1] = {0};
	char input[256] = {0};
//	
//	//加密字符串生成
	memset(result, 0x00, SHA256_DIGEST_LENGTH);
	sprintf(input, "%s%s%s", product_name, device_name, secure_code);
    
    hmac_sha256((unsigned char *)secure_code,strlen(secure_code),(unsigned char *)input,strlen(input),result);
    
	for(uint8_t i = 0; i < SHA256_DIGEST_LENGTH; i++)
	{
		sprintf(&s_password[2 * i], "%02x", *(result + i));
	}
//    str2hex((char *)result,s_password);
	
	return s_password;
    
    
}

