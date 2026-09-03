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
// RC6-64/256/32

#include "rc6.h"

#define w 64	/* word size in bits */
#define r 256	/* based on security estimates */
#define b 32

/* Magic constants for key setup */
//#define P16 0xB7E1
//#define Q16 0x9E37
//#define P32 0xB7E15163
//#define Q32 0x9E3779B9
//#define P64 0xB7E151628AED2A6B
//#define Q64 0x9E3779B97F4A7C15
#define NotP64 0x481EAE9D7512D594
#define NotQ64 0x61C8864680B583EA

/* derived constants */
#define bytes   (w / 8)						/* bytes per word */
#define c       ((b + bytes - 1) / bytes)	/* key in words, rounded up */
#define R256    (2 * r + 4)
#define lgw     6                       	/* log2(w) -- wussed out */

/* Rotations */
#define ROTL(x,y) (((x)<<(y&(w-1))) | ((x)>>(w-(y&(w-1)))))
#define ROTR(x,y) (((x)>>(y&(w-1))) | ((x)<<(w-(y&(w-1)))))

ULONGLONG S[R256];		/* Key schedule */
ULONGLONG IV[8];

// To hide magic values from crypt-scanners
ULONGLONG __declspec(naked) __stdcall not(const ULONGLONG n)
{
	__asm
	{
		mov eax, dword ptr ss : [esp + 4]
		not eax
		mov edx, dword ptr ss : [esp + 8]
		not edx
		retn 8
	}
}

void rc6_key_setup(unsigned char *K, unsigned char *iv)
{
	int i, j, s, v;
	ULONGLONG L[c]; /* Big enough for max b */
	ULONGLONG A, B;

	L[c - 1] = 0;
	for (i = b - 1; i >= 0; i--)
		L[i / bytes] = (L[i / bytes] << 8) + K[i];

	S[0] = not(NotP64);
	for (i = 1; i <= 2 * r + 3; i++)
		S[i] = S[i - 1] + not(NotQ64);

	A = B = i = j = 0;
	v = R256;
	if (c > v) v = c;
	v *= 3;

	for (s = 1; s <= v; s++)
	{
		A = S[i] = ROTL(S[i] + A + B, 3);
		B = L[j] = ROTL(L[j] + A + B, A + B);
		i = (i + 1) % R256;
		j = (j + 1) % c;
	}

	IV[0] = iv[2];
	IV[1] = iv[3];
	IV[2] = iv[0];
	IV[3] = iv[1];
	IV[4] = iv[2];
	IV[5] = iv[3];
	IV[6] = iv[0];
	IV[7] = iv[1];
}

void rc6_block_encrypt(ULONGLONG *pt, ULONGLONG *ct)
{
	ULONGLONG A, B, C, D, t, u, x;

	A = pt[0];
	B = pt[1];
	C = pt[2];
	D = pt[3];
	B += S[0];
	D += S[1];
	for (size_t i = 2; i <= 2 * r; i += 2)
	{
		t = ROTL(B * (2 * B + 1), lgw);
		u = ROTL(D * (2 * D + 1), lgw);
		A = ROTL(A ^ t, u) + S[i];
		C = ROTL(C ^ u, t) + S[i + 1];
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

void rc6_block_decrypt(ULONGLONG *ct, ULONGLONG *pt)
{
	ULONGLONG A, B, C, D, t, u, x;

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
		u = ROTL(D * (2 * D + 1), lgw);
		t = ROTL(B * (2 * B + 1), lgw);
		C = ROTR(C - S[i + 1], t) ^ u;
		A = ROTR(A - S[i], u) ^ t;
	}
	D -= S[1];
	B -= S[0];
	pt[0] = A;
	pt[1] = B;
	pt[2] = C;
	pt[3] = D;
}

void rc6_cbc_encrypt(ULONGLONG *buff, size_t size)
{
	size_t blocks = size / 8;
	for (size_t i = 0; i < blocks; i += 4)
	{
		IV[0] ^= buff[i];
		IV[1] ^= buff[i + 1];
		IV[2] ^= buff[i + 2];
		IV[3] ^= buff[i + 3];
		rc6_block_encrypt(IV, IV);
		buff[i] = IV[0];
		buff[i + 1] = IV[1];
		buff[i + 2] = IV[2];
		buff[i + 3] = IV[3];
	}
	IV[0] = IV[4];
	IV[1] = IV[5];
	IV[2] = IV[6];
	IV[3] = IV[7];
}

void rc6_cbc_decrypt(ULONGLONG *buff, size_t size)
{
	ULONGLONG tmp[4];
	size_t blocks = size / 8;
	for (size_t i = 0; i < blocks; i += 4)
	{
		tmp[0] = buff[i];
		tmp[1] = buff[i + 1];
		tmp[2] = buff[i + 2];
		tmp[3] = buff[i + 3];
		rc6_block_decrypt(&buff[i], &buff[i]);
		buff[i] ^= IV[0];
		buff[i + 1] ^= IV[1];
		buff[i + 2] ^= IV[2];
		buff[i + 3] ^= IV[3];
		IV[0] = tmp[0];
		IV[1] = tmp[1];
		IV[2] = tmp[2];
		IV[3] = tmp[3];
	}
	IV[0] = IV[4];
	IV[1] = IV[5];
	IV[2] = IV[6];
	IV[3] = IV[7];
}