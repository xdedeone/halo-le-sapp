/* rc6 (TM)
* Unoptimized sample implementation of Ron Rivest's submission to the
* AES bakeoff.
*
* Salvo Salasio, 19 June 1998
*
* Intellectual property notes:  The name of the algorithm (RC6) is
* trademarked; any property rights to the algorithm or the trademark
* should be discussed with discussed with the authors of the defining
* paper "The RC6(TM) Block Cipher": Ronald L. Rivest (MIT),
* M.J.B. Robshaw (RSA Labs), R. Sidney (RSA Labs), and Y.L. Yin (RSA Labs),
* distributed 18 June 1998 and available from the lead author's web site.
*
* This sample implementation is placed in the public domain by the author,
* Salvo Salasio.  The ROTL and ROTR definitions were cribbed from RSA Labs'
* RC5 reference implementation.
*/

/* RC6 is parameterized for w-bit words, b bytes of key, and
* r rounds.  The AES version of RC6 specifies b=16, 24, or 32;
* w=32; and r=20.
*/

// RC6-w/r/b
// as
// RC6-16/256/32

#include "rc6_16.h"

#define w 16	/* word size in bits */
#define r 256	/* based on security estimates */
#define b 32

/* Magic constants for key setup */
#define P16 0xB7E1
#define Q16 0x9E37
//#define P32 0xB7E15163
//#define Q32 0x9E3779B9
//#define P64 0xB7E151628AED2A6B
//#define Q64 0x9E3779B97F4A7C15

/* derived constants */
#define bytes   (w / 8)						/* bytes per word */
#define c       ((b + bytes - 1) / bytes)	/* key in words, rounded up */
#define R256    (2 * r + 4)
#define lgw     4                       	/* log2(w) -- wussed out */

/* Rotations */
WORD __forceinline ROL(WORD x, WORD y)
{
	return (x << (y & (w - 1)) | x >> (w - (y & (w - 1))));
}

WORD __forceinline ROR(WORD x, WORD y)
{
	return (x >> (y & (w - 1)) | x << (w - (y & (w - 1))));
}

void RC6_16::rc6_key_setup(unsigned char *K, unsigned char *iv)
{
	int i, j, s, v;
	WORD L[c]; /* Big enough for max b */
	WORD A, B;

	L[c - 1] = 0;
	for (i = b - 1; i >= 0; i--)
		L[i / bytes] = (L[i / bytes] << 8) + K[i];

	S[0] = P16;
	for (i = 1; i <= 2 * r + 3; i++)
		S[i] = S[i - 1] + Q16;

	A = B = i = j = 0;
	v = R256;
	if (c > v) v = c;
	v *= 3;

	for (s = 1; s <= v; s++)
	{
		A = S[i] = ROL(S[i] + A + B, 3);
		B = L[j] = ROL(L[j] + A + B, A + B);
		i = (i + 1) % R256;
		j = (j + 1) % c;
	}

	IV[0] = iv[2];
	IV[1] = iv[3];
	IV[2] = iv[0];
	IV[3] = iv[1];
}

void RC6_16::rc6_block_encrypt(WORD *pt, WORD *ct)
{
	WORD A, B, C, D, t, u, x;

	A = pt[0];
	B = pt[1];
	C = pt[2];
	D = pt[3];
	B += S[0];
	D += S[1];
	for (size_t i = 2; i <= 2 * r; i += 2)
	{
		t = ROL(B * (2 * B + 1), lgw);
		u = ROL(D * (2 * D + 1), lgw);
		A = ROL(A ^ t, u) + S[i];
		C = ROL(C ^ u, t) + S[i + 1];
		x = A;
		A = B;
		B = C;
		C = D;
		D = x;
	}
	A += S[2 * r + 2];
	C += S[2 * r + 3];
	ct[0] = A;
	ct[1] = B;
	ct[2] = C;
	ct[3] = D;
}

void RC6_16::rc6_block_decrypt(WORD *ct, WORD *pt)
{
	WORD A, B, C, D, t, u, x;

	A = ct[0];
	B = ct[1];
	C = ct[2];
	D = ct[3];
	C -= S[2 * r + 3];
	A -= S[2 * r + 2];
	for (size_t i = 2 * r; i >= 2; i -= 2)
	{
		x = D;
		D = C;
		C = B;
		B = A;
		A = x;
		u = ROL(D * (2 * D + 1), lgw);
		t = ROL(B * (2 * B + 1), lgw);
		C = ROR(C - S[i + 1], t) ^ u;
		A = ROR(A - S[i], u) ^ t;
	}
	D -= S[1];
	B -= S[0];
	pt[0] = A;
	pt[1] = B;
	pt[2] = C;
	pt[3] = D;
}

void RC6_16::rc6_cbc_encrypt(WORD *buff, size_t size)
{
	WORD tmpIV[4] = { IV[0], IV[1], IV[2], IV[3] };

	size_t blocks = size / (bytes * 4);
	for (size_t i = 0; i < blocks; i++)
	{
		tmpIV[0] ^= buff[i * 4];
		tmpIV[1] ^= buff[i * 4 + 1];
		tmpIV[2] ^= buff[i * 4 + 2];
		tmpIV[3] ^= buff[i * 4 + 3];
		rc6_block_encrypt(tmpIV, tmpIV);
		buff[i * 4] = tmpIV[0];
		buff[i * 4 + 1] = tmpIV[1];
		buff[i * 4 + 2] = tmpIV[2];
		buff[i * 4 + 3] = tmpIV[3];
	}
}

void RC6_16::rc6_cbc_decrypt(WORD *buff, size_t size)
{
	WORD tmp[4];
	WORD tmpIV[4] = { IV[0], IV[1], IV[2], IV[3] };

	size_t blocks = size / (bytes * 4);
	for (size_t i = 0; i < blocks; i++)
	{
		tmp[0] = buff[i * 4];
		tmp[1] = buff[i * 4 + 1];
		tmp[2] = buff[i * 4 + 2];
		tmp[3] = buff[i * 4 + 3];
		rc6_block_decrypt(&buff[i * 4], &buff[i * 4]);
		buff[i * 4] ^= tmpIV[0];
		buff[i * 4 + 1] ^= tmpIV[1];
		buff[i * 4 + 2] ^= tmpIV[2];
		buff[i * 4 + 3] ^= tmpIV[3];
		tmpIV[0] = tmp[0];
		tmpIV[1] = tmp[1];
		tmpIV[2] = tmp[2];
		tmpIV[3] = tmp[3];
	}
}