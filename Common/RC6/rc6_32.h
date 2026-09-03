#pragma once

typedef unsigned long DWORD;

class RC6_32
{
	DWORD S[516];		/* Key schedule */
	DWORD IV[4];
public:
	void rc6_key_setup(unsigned char *K, unsigned char* iv);

	void rc6_block_encrypt(DWORD *pt, DWORD *ct);

	void rc6_block_decrypt(DWORD *ct, DWORD *pt);

	void rc6_cbc_encrypt(DWORD *buff, size_t size);

	void rc6_cbc_decrypt(DWORD *buff, size_t size);
};