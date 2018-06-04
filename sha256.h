#ifndef SHA256_H
    #define SHA256_H

    #include <string.h>
    #include <inttypes.h>

    #include "logging.h"
    
    /* Elementary functions used by SHA256 */
    #define Ch(x, y, z)     ((x & (y ^ z)) ^ z)
    #define Maj(x, y, z)    ((x & (y | z)) | (y & z))
    #define ROTR(x, n)      ((x >> n) | (x << (32 - n)))
    #define S0(x)           (ROTR(x, 2) ^ ROTR(x, 13) ^ ROTR(x, 22))
    #define S1(x)           (ROTR(x, 6) ^ ROTR(x, 11) ^ ROTR(x, 25))
    #define s0(x)           (ROTR(x, 7) ^ ROTR(x, 18) ^ (x >> 3))
    #define s1(x)           (ROTR(x, 17) ^ ROTR(x, 19) ^ (x >> 10))

    /* SHA256 round function */
    #define RND(a, b, c, d, e, f, g, h, k) \
        do { \
            t0 = h + S1(e) + Ch(e, f, g) + k; \
            t1 = S0(a) + Maj(a, b, c); \
            d += t0; \
            h  = t0 + t1; \
        } while (0)

    /* Adjusted round function for rotating state */
    #define RNDr(S, W, i) \
        RND(S[(64 - i) % 8], S[(65 - i) % 8], \
            S[(66 - i) % 8], S[(67 - i) % 8], \
            S[(68 - i) % 8], S[(69 - i) % 8], \
            S[(70 - i) % 8], S[(71 - i) % 8], \
            W[i] + sha256_k[i])

    #define likely(x)       __builtin_expect((x),1)
    #define unlikely(x)     __builtin_expect((x),0)

    void sha256_init(uint32_t *state);
    void sha256_transform(uint32_t *state, const uint32_t *block);

    uint32_t scanhash_sha256d(const uint32_t header[], uint32_t *result);

#endif