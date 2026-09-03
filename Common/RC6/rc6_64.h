#pragma once

typedef unsigned long long QWORD;

class RC6_64
{
	QWORD S[516];		/* Key schedule */
	QWORD IV[4];
public:
	void rc6_key_setup(unsigned char *K, unsigned char* iv);

	void rc6_block_encrypt(QWORD *pt, QWORD *ct);

	void rc6_block_decrypt(QWORD *ct, QWORD *pt);

	void rc6_cbc_encrypt(QWORD *buff, size_t size);

	void rc6_cbc_decrypt(QWORD *buff, size_t size);

	void rc6_cbc_encrypt(QWORD *buff, QWORD *out, size_t size);
};