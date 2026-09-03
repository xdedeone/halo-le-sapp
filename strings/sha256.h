/*
* FILE:	sha2.h
* AUTHOR:	Aaron D. Gifford - http://www.aarongifford.com/
*
* Copyright (c) 2000-2001, Aaron D. Gifford
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions
* are met:
* 1. Redistributions of source code must retain the above copyright
*    notice, this list of conditions and the following disclaimer.
* 2. Redistributions in binary form must reproduce the above copyright
*    notice, this list of conditions and the following disclaimer in the
*    documentation and/or other materials provided with the distribution.
* 3. Neither the name of the copyright holder nor the names of contributors
*    may be used to endorse or promote products derived from this software
*    without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTOR(S) ``AS IS'' AND
* ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTOR(S) BE LIABLE
* FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
* OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
* HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
* LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
* OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
* SUCH DAMAGE.
*
* $Id: sha2.h,v 1.1 2001/11/08 00:02:01 adg Exp adg $
*/

#ifndef __SHA2_H__
#define __SHA2_H__

/*** SHA-256/384/512 Various Length Definitions ***********************/
#define SHA256_BLOCK_LENGTH		64
#define SHA256_DIGEST_LENGTH		32


/*** SHA-256/384/512 Context Structures *******************************/
/* NOTE: If your architecture does not define either u_intXX_t types or
* uintXX_t (from inttypes.h), you may need to define things by hand
* for your system:
*/

typedef unsigned char sha2_byte;	/* Exactly 1 byte */
typedef unsigned long sha2_word32;	/* Exactly 4 bytes */
typedef unsigned long long sha2_word64;	/* Exactly 4 bytes */

/*
* Most BSD systems already define u_intXX_t types, as does Linux.
* Some systems, however, like Compaq's Tru64 Unix instead can use
* uintXX_t types defined by very recent ANSI C standards and included
* in the file:
*
*   #include <inttypes.h>
*
* If you choose to use <inttypes.h> then please define:
*
*   #define SHA2_USE_INTTYPES_H
*
* Or on the command line during compile:
*
*   cc -DSHA2_USE_INTTYPES_H ...
*/
struct SHA256_CTX {
	sha2_word32	state[8];
	sha2_word64	bitcount;
	sha2_byte	buffer[SHA256_BLOCK_LENGTH];
};

/*** SHA-256/384/512 Function Prototypes ******************************/
void SHA256_Init(SHA256_CTX *);
void SHA256_Update(SHA256_CTX*, const sha2_byte*, size_t);
void SHA256_Final(SHA256_CTX*, sha2_byte*);
void SHA256_Data(const unsigned char*, size_t, const unsigned char*, size_t, unsigned char*);

#endif