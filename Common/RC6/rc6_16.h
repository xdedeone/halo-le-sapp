#pragma once

typedef unsigned short WORD;

class RC6_16
{
	WORD S[516];		/* Key schedule */
	WORD IV[4];
public:
	void rc6_key_setup(unsigned char *K, unsigned char* iv);

	void rc6_block_encrypt(WORD *pt, WORD *ct);

	void rc6_block_decrypt(WORD *ct, WORD *pt);

	void rc6_cbc_encrypt(WORD *buff, size_t size);

	void rc6_cbc_decrypt(WORD *buff, size_t size);
};