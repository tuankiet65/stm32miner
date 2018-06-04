/* Taken from https://github.com/tpruvot/cpuminer-multi/blob/linux/algo/sha2.c */

/*
 * Copyright 2011 ArtForz
 * Copyright 2011-2013 pooler
 * Copyright 2018 Ho Tuan Kiet
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version. See LICENSE for more details.
 */

#include "sha256.h"

static uint32_t __attribute__((section(".data"))) sha256_h[8] = {
	0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
	0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19
};

static uint32_t __attribute__((section(".data"))) sha256_k[64] = {
	0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5,
	0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
	0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
	0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
	0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc,
	0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
	0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7,
	0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
	0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
	0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
	0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3,
	0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
	0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5,
	0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
	0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
	0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

static uint32_t __attribute__((section(".data"))) sha256d_hash1[16] = {
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x80000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000100
};

void sha256_init(uint32_t *state) {
	memcpy(state, sha256_h, 32);
}

/*
 * SHA256 block compression function.  The 256-bit state is transformed via
 * the 512-bit input block to produce a new state.
 */
void sha256_transform(uint32_t *state, const uint32_t *block) {
	uint32_t W[64];
	uint32_t S[8];
	uint32_t t0, t1;
	int i;

	/* 1. Prepare message schedule W. */
	memcpy(W, block, 64);
	for (i = 16; i < 64; i += 2) {
		W[i]   = s1(W[i - 2]) + W[i - 7] + s0(W[i - 15]) + W[i - 16];
		W[i+1] = s1(W[i - 1]) + W[i - 6] + s0(W[i - 14]) + W[i - 15];
	}

	/* 2. Initialize working variables. */
	memcpy(S, state, 32);

	/* 3. Mix. */
	for (int i = 0; i < 64; ++i) RNDr(S, W, i);

	/* 4. Mix local working variables into global state */
	for (i = 0; i < 8; i++)
		state[i] += S[i];
}

static inline void sha256d_preextend(uint32_t *W) {
	W[16] = s1(W[14]) + W[ 9] + s0(W[ 1]) + W[ 0];
	W[17] = s1(W[15]) + W[10] + s0(W[ 2]) + W[ 1];
	W[18] = s1(W[16]) + W[11]             + W[ 2];
	W[19] = s1(W[17]) + W[12] + s0(W[ 4]);
	W[20] =             W[13] + s0(W[ 5]) + W[ 4];
	W[21] =             W[14] + s0(W[ 6]) + W[ 5];
	W[22] =             W[15] + s0(W[ 7]) + W[ 6];
	W[23] =             W[16] + s0(W[ 8]) + W[ 7];
	W[24] =             W[17] + s0(W[ 9]) + W[ 8];
	W[25] =                     s0(W[10]) + W[ 9];
	W[26] =                     s0(W[11]) + W[10];
	W[27] =                     s0(W[12]) + W[11];
	W[28] =                     s0(W[13]) + W[12];
	W[29] =                     s0(W[14]) + W[13];
	W[30] =                     s0(W[15]) + W[14];
	W[31] =                     s0(W[16]) + W[15];
}

static inline void sha256d_prehash(uint32_t *S, const uint32_t *W) {
	uint32_t t0, t1;
	RNDr(S, W, 0);
	RNDr(S, W, 1);
	RNDr(S, W, 2);
}

static inline void sha256d_ms(uint32_t *hash, uint32_t *W, const uint32_t *midstate, const uint32_t *prehash) {
	uint32_t S[64];
	uint32_t t0, t1;
	int i;

	S[18] = W[18];
	S[19] = W[19];
	S[20] = W[20];
	S[22] = W[22];
	S[23] = W[23];
	S[24] = W[24];
	S[30] = W[30];
	S[31] = W[31];

	W[18] += s0(W[3]);
	W[19] += W[3];
	W[20] += s1(W[18]);
	W[21]  = s1(W[19]);
	W[22] += s1(W[20]);
	W[23] += s1(W[21]);
	W[24] += s1(W[22]);
	W[25]  = s1(W[23]) + W[18];
	W[26]  = s1(W[24]) + W[19];
	W[27]  = s1(W[25]) + W[20];
	W[28]  = s1(W[26]) + W[21];
	W[29]  = s1(W[27]) + W[22];
	W[30] += s1(W[28]) + W[23];
	W[31] += s1(W[29]) + W[24];
	for (i = 32; i < 64; i += 2) {
		W[i]   = s1(W[i - 2]) + W[i - 7] + s0(W[i - 15]) + W[i - 16];
		W[i+1] = s1(W[i - 1]) + W[i - 6] + s0(W[i - 14]) + W[i - 15];
	}

	memcpy(S, prehash, 32);

	#ifdef DEBUG
		// To conserve flash space for UART debugging
		for (int i = 3; i < 64; ++i) RNDr(S, W, i);
	#else
		// Just unroll all of it
		RNDr(S, W,  3);
		RNDr(S, W,  4);
		RNDr(S, W,  5);
		RNDr(S, W,  6);
		RNDr(S, W,  7);
		RNDr(S, W,  8);
		RNDr(S, W,  9);
		RNDr(S, W, 10);
		RNDr(S, W, 11);
		RNDr(S, W, 12);
		RNDr(S, W, 13);
		RNDr(S, W, 14);
		RNDr(S, W, 15);
		RNDr(S, W, 16);
		RNDr(S, W, 17);
		RNDr(S, W, 18);
		RNDr(S, W, 19);
		RNDr(S, W, 20);
		RNDr(S, W, 21);
		RNDr(S, W, 22);
		RNDr(S, W, 23);
		RNDr(S, W, 24);
		RNDr(S, W, 25);
		RNDr(S, W, 26);
		RNDr(S, W, 27);
		RNDr(S, W, 28);
		RNDr(S, W, 29);
		RNDr(S, W, 30);
		RNDr(S, W, 31);
		RNDr(S, W, 32);
		RNDr(S, W, 33);
		RNDr(S, W, 34);
		RNDr(S, W, 35);
		RNDr(S, W, 36);
		RNDr(S, W, 37);
		RNDr(S, W, 38);
		RNDr(S, W, 39);
		RNDr(S, W, 40);
		RNDr(S, W, 41);
		RNDr(S, W, 42);
		RNDr(S, W, 43);
		RNDr(S, W, 44);
		RNDr(S, W, 45);
		RNDr(S, W, 46);
		RNDr(S, W, 47);
		RNDr(S, W, 48);
		RNDr(S, W, 49);
		RNDr(S, W, 50);
		RNDr(S, W, 51);
		RNDr(S, W, 52);
		RNDr(S, W, 53);
		RNDr(S, W, 54);
		RNDr(S, W, 55);
		RNDr(S, W, 56);
		RNDr(S, W, 57);
		RNDr(S, W, 58);
		RNDr(S, W, 59);
		RNDr(S, W, 60);
		RNDr(S, W, 61);
		RNDr(S, W, 62);
		RNDr(S, W, 63);
	#endif

	for (i = 0; i < 8; i++)
		S[i] += midstate[i];
	
	W[18] = S[18];
	W[19] = S[19];
	W[20] = S[20];
	W[22] = S[22];
	W[23] = S[23];
	W[24] = S[24];
	W[30] = S[30];
	W[31] = S[31];
	
	memcpy(S + 8, sha256d_hash1 + 8, 32);
	S[16] = s1(sha256d_hash1[14]) + sha256d_hash1[ 9] + s0(S[ 1]) + S[ 0];
	S[17] = s1(sha256d_hash1[15]) + sha256d_hash1[10] + s0(S[ 2]) + S[ 1];
	S[18] = s1(S[16]) + sha256d_hash1[11] + s0(S[ 3]) + S[ 2];
	S[19] = s1(S[17]) + sha256d_hash1[12] + s0(S[ 4]) + S[ 3];
	S[20] = s1(S[18]) + sha256d_hash1[13] + s0(S[ 5]) + S[ 4];
	S[21] = s1(S[19]) + sha256d_hash1[14] + s0(S[ 6]) + S[ 5];
	S[22] = s1(S[20]) + sha256d_hash1[15] + s0(S[ 7]) + S[ 6];
	S[23] = s1(S[21]) + S[16] + s0(sha256d_hash1[ 8]) + S[ 7];
	S[24] = s1(S[22]) + S[17] + s0(sha256d_hash1[ 9]) + sha256d_hash1[ 8];
	S[25] = s1(S[23]) + S[18] + s0(sha256d_hash1[10]) + sha256d_hash1[ 9];
	S[26] = s1(S[24]) + S[19] + s0(sha256d_hash1[11]) + sha256d_hash1[10];
	S[27] = s1(S[25]) + S[20] + s0(sha256d_hash1[12]) + sha256d_hash1[11];
	S[28] = s1(S[26]) + S[21] + s0(sha256d_hash1[13]) + sha256d_hash1[12];
	S[29] = s1(S[27]) + S[22] + s0(sha256d_hash1[14]) + sha256d_hash1[13];
	S[30] = s1(S[28]) + S[23] + s0(sha256d_hash1[15]) + sha256d_hash1[14];
	S[31] = s1(S[29]) + S[24] + s0(S[16])             + sha256d_hash1[15];
	for (i = 32; i < 60; i += 2) {
		S[i]   = s1(S[i - 2]) + S[i - 7] + s0(S[i - 15]) + S[i - 16];
		S[i+1] = s1(S[i - 1]) + S[i - 6] + s0(S[i - 14]) + S[i - 15];
	}
	S[60] = s1(S[58]) + S[53] + s0(S[45]) + S[44];

	sha256_init(hash);

	#ifdef DEBUG
		// To conserve flash space for UART debugging
		for (int i = 0; i < 57; ++i) RNDr(hash, S, i);
	#else 
		RNDr(hash, S,  0);
		RNDr(hash, S,  1);
		RNDr(hash, S,  2);
		RNDr(hash, S,  3);
		RNDr(hash, S,  4);
		RNDr(hash, S,  5);
		RNDr(hash, S,  6);
		RNDr(hash, S,  7);
		RNDr(hash, S,  8);
		RNDr(hash, S,  9);
		RNDr(hash, S, 10);
		RNDr(hash, S, 11);
		RNDr(hash, S, 12);
		RNDr(hash, S, 13);
		RNDr(hash, S, 14);
		RNDr(hash, S, 15);
		RNDr(hash, S, 16);
		RNDr(hash, S, 17);
		RNDr(hash, S, 18);
		RNDr(hash, S, 19);
		RNDr(hash, S, 20);
		RNDr(hash, S, 21);
		RNDr(hash, S, 22);
		RNDr(hash, S, 23);
		RNDr(hash, S, 24);
		RNDr(hash, S, 25);
		RNDr(hash, S, 26);
		RNDr(hash, S, 27);
		RNDr(hash, S, 28);
		RNDr(hash, S, 29);
		RNDr(hash, S, 30);
		RNDr(hash, S, 31);
		RNDr(hash, S, 32);
		RNDr(hash, S, 33);
		RNDr(hash, S, 34);
		RNDr(hash, S, 35);
		RNDr(hash, S, 36);
		RNDr(hash, S, 37);
		RNDr(hash, S, 38);
		RNDr(hash, S, 39);
		RNDr(hash, S, 40);
		RNDr(hash, S, 41);
		RNDr(hash, S, 42);
		RNDr(hash, S, 43);
		RNDr(hash, S, 44);
		RNDr(hash, S, 45);
		RNDr(hash, S, 46);
		RNDr(hash, S, 47);
		RNDr(hash, S, 48);
		RNDr(hash, S, 49);
		RNDr(hash, S, 50);
		RNDr(hash, S, 51);
		RNDr(hash, S, 52);
		RNDr(hash, S, 53);
		RNDr(hash, S, 54);
		RNDr(hash, S, 55);
		RNDr(hash, S, 56);
	#endif
	
	hash[2] += hash[6] + S1(hash[3]) + Ch(hash[3], hash[4], hash[5])
	         + S[57] + sha256_k[57];
	hash[1] += hash[5] + S1(hash[2]) + Ch(hash[2], hash[3], hash[4])
	         + S[58] + sha256_k[58];
	hash[0] += hash[4] + S1(hash[1]) + Ch(hash[1], hash[2], hash[3])
	         + S[59] + sha256_k[59];
	hash[7] += hash[3] + S1(hash[0]) + Ch(hash[0], hash[1], hash[2])
	         + S[60] + sha256_k[60]
	         + sha256_h[7];
}

uint32_t scanhash_sha256d(const uint32_t header[], uint32_t *result) {
	uint32_t data[64];
	uint32_t hash[8];
	uint32_t midstate[8];
	uint32_t prehash[8];
	
	memcpy(data, header + 16, 64);
	sha256d_preextend(data);
	
	sha256_init(midstate);
	sha256_transform(midstate, header);
	memcpy(prehash, midstate, 32);
	sha256d_prehash(prehash, header + 16);
	
	do {
		data[3]++;
		sha256d_ms(hash, data, midstate, prehash);
		if (hash[7] == 0x00000000) {
			LOG(INFO, "winning nonce: 0x%08x", __builtin_bswap32(data[3]));
			*result = data[3];
            return 1;
        }
		if (data[3] % 0xfff == 0) {
			LOG(INFO, "nonce: 0x%08x, first byte: 0x%08x", __builtin_bswap32(data[3]), hash[7]);
		}
	} while (1);

	return 0;
}