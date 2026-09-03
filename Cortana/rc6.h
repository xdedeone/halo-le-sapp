#pragma once

typedef unsigned long long ULONGLONG;

void rc6_key_setup(unsigned char *K, unsigned char* iv);

void rc6_block_encrypt(ULONGLONG *pt, ULONGLONG *ct);

void rc6_block_decrypt(ULONGLONG *ct, ULONGLONG *pt);

void rc6_cbc_encrypt(ULONGLONG *buff, size_t size);

void rc6_cbc_decrypt(ULONGLONG *buff, size_t size);