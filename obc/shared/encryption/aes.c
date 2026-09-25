/*
 * AES-128 and AES-128-GCM - reference implementation
 *
 * A from-scratch, table-driven implementation written for learning, built up in
 * layers from the primary standards:
 *
 *   - AES-128 block cipher and inverse ....... NIST FIPS 197
 *       https://nvlpubs.nist.gov/nistpubs/FIPS/NIST.FIPS.197-upd1.pdf
 *   - CTR mode ............................... NIST SP 800-38A (Sec. 6.5, F.5)
 *   - GHASH / GF(2^128) multiply ............. NIST SP 800-38D (Sec. 6.3-6.4)
 *   - AES-128-GCM (authenticated encryption).. NIST SP 800-38D (Sec. 7)
 *                                              McGrew-Viega (test vectors)
 *
 * The file is organized bottom-up: shared parameters and tables, then the two
 * finite-field arithmetics (GF(2^8) for AES, GF(2^128) for GHASH), the AES state
 * transforms and key schedule, the AES cipher, the CTR mode, GHASH, and finally
 * the GCM layer that combines them. A self-contained test suite (validated
 * against FIPS 197, SP 800-38A F.5.1, and McGrew-Viega vectors) follows in main.
 *
 * State convention: the 16-byte block is held as state[row][col] and is loaded
 * and stored column-major, i.e. state[r][c] = in[r + 4*c]  (FIPS 197 Sec. 3.4).
 *
 * NOTE: This is an educational implementation. It is faithful to the standards
 * but is NOT hardened against timing/cache side-channel attacks and should not
 * be used to protect real secrets. Use a vetted library or hardware AES-GCM for
 * production.
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* ============================================================================
 * Algorithm parameters (FIPS 197, Sec. 5, Table 3)
 * ==========================================================================*/

#define KEY_LEN 16         /* key size in bytes (128 bits)          */
#define Nb 4               /* columns in the state                  */
#define Nk 4               /* 32-bit words in the key               */
#define Nr 10              /* number of rounds for AES-128          */
#define NW (Nb * (Nr + 1)) /* key-schedule length in words (= 44)   */

/* ============================================================================
 * Lookup tables
 * ==========================================================================*/

/* Round constants for the key schedule (FIPS 197, Table 5). Leading byte only;
 * the low three bytes of each Rcon word are zero. */
static const uint32_t Rcon[Nr] = {0x01000000, 0x02000000, 0x04000000, 0x08000000, 0x10000000,
                                  0x20000000, 0x40000000, 0x80000000, 0x1B000000, 0x36000000};

/* Forward S-box (FIPS 197, Table 4), indexed sbox[high nibble][low nibble]. */
static const uint8_t sbox[16][16] = {
    /*        y=0   1     2     3     4     5     6     7     8     9     a     b     c     d     e     f  */
    /* x=0 */ {0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5, 0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76},
    /* x=1 */ {0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0, 0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0},
    /* x=2 */ {0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc, 0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15},
    /* x=3 */ {0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a, 0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75},
    /* x=4 */ {0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0, 0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84},
    /* x=5 */ {0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b, 0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf},
    /* x=6 */ {0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85, 0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8},
    /* x=7 */ {0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5, 0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2},
    /* x=8 */ {0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17, 0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73},
    /* x=9 */ {0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88, 0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb},
    /* x=a */ {0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c, 0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79},
    /* x=b */ {0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9, 0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08},
    /* x=c */ {0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6, 0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a},
    /* x=d */ {0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e, 0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e},
    /* x=e */ {0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94, 0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf},
    /* x=f */ {0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68, 0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16}};

/* Inverse S-box (FIPS 197, Table 6). Verified at startup against sbox via
 * verify_sbox_inverse(): InvS(S(b)) == b for all 256 bytes. */
static const uint8_t invSbox[16][16] = {
    /*        y=0   1     2     3     4     5     6     7     8     9     a     b     c     d     e     f  */
    /* x=0 */ {0x52, 0x09, 0x6a, 0xd5, 0x30, 0x36, 0xa5, 0x38, 0xbf, 0x40, 0xa3, 0x9e, 0x81, 0xf3, 0xd7, 0xfb},
    /* x=1 */ {0x7c, 0xe3, 0x39, 0x82, 0x9b, 0x2f, 0xff, 0x87, 0x34, 0x8e, 0x43, 0x44, 0xc4, 0xde, 0xe9, 0xcb},
    /* x=2 */ {0x54, 0x7b, 0x94, 0x32, 0xa6, 0xc2, 0x23, 0x3d, 0xee, 0x4c, 0x95, 0x0b, 0x42, 0xfa, 0xc3, 0x4e},
    /* x=3 */ {0x08, 0x2e, 0xa1, 0x66, 0x28, 0xd9, 0x24, 0xb2, 0x76, 0x5b, 0xa2, 0x49, 0x6d, 0x8b, 0xd1, 0x25},
    /* x=4 */ {0x72, 0xf8, 0xf6, 0x64, 0x86, 0x68, 0x98, 0x16, 0xd4, 0xa4, 0x5c, 0xcc, 0x5d, 0x65, 0xb6, 0x92},
    /* x=5 */ {0x6c, 0x70, 0x48, 0x50, 0xfd, 0xed, 0xb9, 0xda, 0x5e, 0x15, 0x46, 0x57, 0xa7, 0x8d, 0x9d, 0x84},
    /* x=6 */ {0x90, 0xd8, 0xab, 0x00, 0x8c, 0xbc, 0xd3, 0x0a, 0xf7, 0xe4, 0x58, 0x05, 0xb8, 0xb3, 0x45, 0x06},
    /* x=7 */ {0xd0, 0x2c, 0x1e, 0x8f, 0xca, 0x3f, 0x0f, 0x02, 0xc1, 0xaf, 0xbd, 0x03, 0x01, 0x13, 0x8a, 0x6b},
    /* x=8 */ {0x3a, 0x91, 0x11, 0x41, 0x4f, 0x67, 0xdc, 0xea, 0x97, 0xf2, 0xcf, 0xce, 0xf0, 0xb4, 0xe6, 0x73},
    /* x=9 */ {0x96, 0xac, 0x74, 0x22, 0xe7, 0xad, 0x35, 0x85, 0xe2, 0xf9, 0x37, 0xe8, 0x1c, 0x75, 0xdf, 0x6e},
    /* x=a */ {0x47, 0xf1, 0x1a, 0x71, 0x1d, 0x29, 0xc5, 0x89, 0x6f, 0xb7, 0x62, 0x0e, 0xaa, 0x18, 0xbe, 0x1b},
    /* x=b */ {0xfc, 0x56, 0x3e, 0x4b, 0xc6, 0xd2, 0x79, 0x20, 0x9a, 0xdb, 0xc0, 0xfe, 0x78, 0xcd, 0x5a, 0xf4},
    /* x=c */ {0x1f, 0xdd, 0xa8, 0x33, 0x88, 0x07, 0xc7, 0x31, 0xb1, 0x12, 0x10, 0x59, 0x27, 0x80, 0xec, 0x5f},
    /* x=d */ {0x60, 0x51, 0x7f, 0xa9, 0x19, 0xb5, 0x4a, 0x0d, 0x2d, 0xe5, 0x7a, 0x9f, 0x93, 0xc9, 0x9c, 0xef},
    /* x=e */ {0xa0, 0xe0, 0x3b, 0x4d, 0xae, 0x2a, 0xf5, 0xb0, 0xc8, 0xeb, 0xbb, 0x3c, 0x83, 0x53, 0x99, 0x61},
    /* x=f */ {0x17, 0x2b, 0x04, 0x7e, 0xba, 0x77, 0xd6, 0x26, 0xe1, 0x69, 0x14, 0x63, 0x55, 0x21, 0x0c, 0x7d}};

/* ============================================================================
 * GF(2^8) arithmetic (FIPS 197, Sec. 4.2)
 * Used by MixColumns / InvMixColumns.
 * ==========================================================================*/

/* Multiply a byte by {02} in GF(2^8): left shift, reducing modulo
 * m(x) = x^8 + x^4 + x^3 + x + 1 (constant 0x1B) when the high bit was set. */
static uint8_t xtimes(uint8_t b) {
  if (b & 0x80) return (uint8_t)((b << 1) ^ 0x1B);
  return (uint8_t)(b << 1);
}

/* General GF(2^8) multiply. Decomposes `mult` bitwise: for each set bit k, XOR
 * in {2^k} * s, accumulated by repeated xtimes(). */
static uint8_t gmul(uint8_t s, uint8_t mult) {
  uint8_t result = 0;
  uint8_t term = s; /* {01}*s, then {02}*s, {04}*s, ... */
  for (int i = 0; i < 8; ++i) {
    if (mult & (1u << i)) result ^= term;
    term = xtimes(term);
  }
  return result;
}

/* ============================================================================
 * GF(2^128) arithmetic (SP 800-38D, Sec. 6.3)
 * Used by GHASH. Note this is a DIFFERENT field from the GF(2^8) above, with a
 * "little endian" bit convention: bit 0 is the leftmost bit of the block and the
 * lowest-order polynomial coefficient, so the multiply shifts RIGHT and reduces
 * with R = 0xE1 in byte 0 when the bit shifted off the right end is set.
 * ==========================================================================*/

/* Multiply two 128-bit blocks in GF(2^128): out = x * y. Aliasing-safe (out may
 * be the same buffer as x or y) since the product is accumulated in a local and
 * copied out only at the end. */
void gf128_mul(const uint8_t x[16], const uint8_t y[16], uint8_t out[16]) {
  uint8_t accumulator[16] = {0}; /* Z = 0^128 */
  uint8_t copy[16];
  memcpy(copy, y, 16); /* V = Y */

  for (size_t i = 0; i < 128; ++i) {
    /* If bit i of X (bit 0 = leftmost) is set, accumulate the current V. */
    int bit = (x[i / 8] >> (7 - (i % 8))) & 1;
    if (bit) {
      for (size_t b = 0; b < 16; ++b) {
        accumulator[b] ^= copy[b];
      }
    }

    /* V = V >> 1 across all 16 bytes, carrying each byte's LSB into the next. */
    uint8_t carry = 0;
    for (size_t b = 0; b < 16; ++b) {
      uint8_t new_carry = copy[b] & 1;
      copy[b] = (uint8_t)((copy[b] >> 1) | (carry << 7));
      carry = new_carry;
    }

    /* If the bit that fell off the right end was set, reduce by R (0xE1||0^120). */
    if (carry) {
      copy[0] ^= 0xE1;
    }
  }

  memcpy(out, accumulator, 16);
}

/* ============================================================================
 * AES state load / store (column-major, FIPS 197 Sec. 3.4)
 * ==========================================================================*/

static void load_state(uint8_t state[4][4], const uint8_t in[16]) {
  for (size_t col = 0; col < 4; ++col)
    for (size_t row = 0; row < 4; ++row) state[row][col] = in[row + 4 * col];
}

static void store_state(const uint8_t state[4][4], uint8_t out[16]) {
  for (size_t col = 0; col < 4; ++col)
    for (size_t row = 0; row < 4; ++row) out[row + 4 * col] = state[row][col];
}

/* ============================================================================
 * Forward round transformations (FIPS 197, Sec. 5.1)
 * ==========================================================================*/

/* SubBytes (Sec. 5.1.1): substitute each byte through the S-box. */
static void subBytes(uint8_t state[4][4]) {
  for (size_t row = 0; row < 4; ++row)
    for (size_t col = 0; col < 4; ++col) state[row][col] = sbox[state[row][col] >> 4][state[row][col] & 0x0F];
}

/* ShiftRows (Sec. 5.1.2): cyclically shift row r left by r bytes. */
static void shiftRows(uint8_t state[4][4]) {
  for (size_t row = 1; row < 4; ++row) { /* row 0 is unchanged */
    uint32_t buf = (uint32_t)state[row][0] << 24 | (uint32_t)state[row][1] << 16 | (uint32_t)state[row][2] << 8 |
                   (uint32_t)state[row][3];
    buf = buf << (8 * row) | buf >> (32 - 8 * row); /* rotate left by `row` bytes */
    state[row][0] = (uint8_t)(buf >> 24);
    state[row][1] = (uint8_t)(buf >> 16);
    state[row][2] = (uint8_t)(buf >> 8);
    state[row][3] = (uint8_t)(buf & 0xFF);
  }
}

/* MixColumns (Sec. 5.1.3): multiply each column by the fixed matrix
 * [02 03 01 01 / 01 02 03 01 / 01 01 02 03 / 03 01 01 02]. */
static void mixColumns(uint8_t state[4][4]) {
  for (size_t col = 0; col < 4; ++col) {
    uint8_t s0 = state[0][col], s1 = state[1][col], s2 = state[2][col], s3 = state[3][col];
    state[0][col] = gmul(s0, 0x02) ^ gmul(s1, 0x03) ^ gmul(s2, 0x01) ^ gmul(s3, 0x01);
    state[1][col] = gmul(s0, 0x01) ^ gmul(s1, 0x02) ^ gmul(s2, 0x03) ^ gmul(s3, 0x01);
    state[2][col] = gmul(s0, 0x01) ^ gmul(s1, 0x01) ^ gmul(s2, 0x02) ^ gmul(s3, 0x03);
    state[3][col] = gmul(s0, 0x03) ^ gmul(s1, 0x01) ^ gmul(s2, 0x01) ^ gmul(s3, 0x02);
  }
}

/* AddRoundKey (Sec. 5.1.4): XOR each column with a key-schedule word.
 * This transformation is its own inverse and is reused by the inverse cipher. */
static void addRoundKey(uint8_t state[4][4], const uint32_t rk[4]) {
  for (size_t c = 0; c < 4; ++c) {
    uint32_t col =
        (uint32_t)state[0][c] << 24 | (uint32_t)state[1][c] << 16 | (uint32_t)state[2][c] << 8 | (uint32_t)state[3][c];
    col ^= rk[c];
    state[0][c] = (uint8_t)(col >> 24);
    state[1][c] = (uint8_t)(col >> 16);
    state[2][c] = (uint8_t)(col >> 8);
    state[3][c] = (uint8_t)(col & 0xFF);
  }
}

/* ============================================================================
 * Inverse round transformations (FIPS 197, Sec. 5.3)
 * AddRoundKey is its own inverse and is reused from the forward section above.
 * ==========================================================================*/

/* InvShiftRows (Sec. 5.3.1): cyclically shift row r right by r bytes. */
static void invShiftRows(uint8_t state[4][4]) {
  for (size_t row = 1; row < 4; ++row) { /* row 0 is unchanged */
    uint32_t buf = (uint32_t)state[row][0] << 24 | (uint32_t)state[row][1] << 16 | (uint32_t)state[row][2] << 8 |
                   (uint32_t)state[row][3];
    buf = buf >> (8 * row) | buf << (32 - 8 * row); /* rotate right by `row` bytes */
    state[row][0] = (uint8_t)(buf >> 24);
    state[row][1] = (uint8_t)(buf >> 16);
    state[row][2] = (uint8_t)(buf >> 8);
    state[row][3] = (uint8_t)(buf & 0xFF);
  }
}

/* InvSubBytes (Sec. 5.3.2): substitute each byte through the inverse S-box. */
static void invSubBytes(uint8_t state[4][4]) {
  for (size_t row = 0; row < 4; ++row)
    for (size_t col = 0; col < 4; ++col) state[row][col] = invSbox[state[row][col] >> 4][state[row][col] & 0x0F];
}

/* InvMixColumns (Sec. 5.3.3): multiply each column by the inverse matrix
 * [0e 0b 0d 09 / 09 0e 0b 0d / 0d 09 0e 0b / 0b 0d 09 0e]. */
static void invMixColumns(uint8_t state[4][4]) {
  for (size_t col = 0; col < 4; ++col) {
    uint8_t s0 = state[0][col], s1 = state[1][col], s2 = state[2][col], s3 = state[3][col];
    state[0][col] = gmul(s0, 0x0e) ^ gmul(s1, 0x0b) ^ gmul(s2, 0x0d) ^ gmul(s3, 0x09);
    state[1][col] = gmul(s0, 0x09) ^ gmul(s1, 0x0e) ^ gmul(s2, 0x0b) ^ gmul(s3, 0x0d);
    state[2][col] = gmul(s0, 0x0d) ^ gmul(s1, 0x09) ^ gmul(s2, 0x0e) ^ gmul(s3, 0x0b);
    state[3][col] = gmul(s0, 0x0b) ^ gmul(s1, 0x0d) ^ gmul(s2, 0x09) ^ gmul(s3, 0x0e);
  }
}

/* ============================================================================
 * Key expansion (FIPS 197, Sec. 5.2)
 * ==========================================================================*/

/* Rotate a word left by one byte: [a0,a1,a2,a3] -> [a1,a2,a3,a0]. */
static uint32_t RotWord(uint32_t word) { return (word << 8) | (word >> 24); }

/* Apply the S-box to each of the four bytes of a word. */
static uint32_t SubWord(uint32_t word) {
  uint8_t b0 = sbox[(word >> 28) & 0x0F][(word >> 24) & 0x0F];
  uint8_t b1 = sbox[(word >> 20) & 0x0F][(word >> 16) & 0x0F];
  uint8_t b2 = sbox[(word >> 12) & 0x0F][(word >> 8) & 0x0F];
  uint8_t b3 = sbox[(word >> 4) & 0x0F][word & 0x0F];
  return ((uint32_t)b0 << 24) | ((uint32_t)b1 << 16) | ((uint32_t)b2 << 8) | b3;
}

/* Expand the 16-byte key into NW = 44 round-key words. */
static void keyExpansion(const uint8_t *key, uint32_t w[NW]) {
  for (size_t i = 0; i < NW; ++i) {
    if (i < Nk) {
      w[i] = (uint32_t)key[4 * i] << 24 | (uint32_t)key[4 * i + 1] << 16 | (uint32_t)key[4 * i + 2] << 8 |
             (uint32_t)key[4 * i + 3];
    } else {
      uint32_t temp = w[i - 1];
      if (i % Nk == 0) temp = SubWord(RotWord(temp)) ^ Rcon[(i / Nk) - 1];
      w[i] = w[i - Nk] ^ temp;
    }
  }
}

/* ============================================================================
 * AES-128 cipher and inverse cipher (FIPS 197, Alg. 1 and Alg. 3)
 * Round-key words for round r are w[4*r .. 4*r+3], passed as (w + 4*r).
 * ==========================================================================*/

void aes128_encrypt(const uint8_t in[16], const uint8_t key[16], uint8_t out[16]) {
  uint8_t state[4][4];
  uint32_t w[NW];
  load_state(state, in);
  keyExpansion(key, w);

  addRoundKey(state, w); /* initial round key w[0..3] */

  for (size_t round = 1; round < Nr; ++round) {
    subBytes(state);
    shiftRows(state);
    mixColumns(state);
    addRoundKey(state, w + 4 * round);
  }

  /* Final round: no MixColumns. */
  subBytes(state);
  shiftRows(state);
  addRoundKey(state, w + 4 * Nr);

  store_state(state, out);
}

void aes128_decrypt(const uint8_t in[16], const uint8_t key[16], uint8_t out[16]) {
  uint8_t state[4][4];
  uint32_t w[NW];
  load_state(state, in);
  keyExpansion(key, w);

  addRoundKey(state, w + 4 * Nr); /* initial round key w[40..43] */

  for (size_t round = Nr - 1; round >= 1; --round) {
    invShiftRows(state);
    invSubBytes(state);
    addRoundKey(state, w + 4 * round);
    invMixColumns(state);
  }

  /* Final round: no InvMixColumns. */
  invShiftRows(state);
  invSubBytes(state);
  addRoundKey(state, w); /* final round key w[0..3] */

  store_state(state, out);
}

/* ============================================================================
 * CTR mode (SP 800-38A, Sec. 6.5 / the GCTR function of SP 800-38D)
 * Encryption and decryption are the same operation (XOR with the keystream), so
 * only the forward cipher is ever used. GCM's inc32 convention is used: the
 * counter is the rightmost 32 bits, incremented per block; bytes 0-11 are fixed.
 * ==========================================================================*/

/* Increment the trailing 32-bit counter of a 16-byte block, big-endian, wrapping
 * modulo 2^32. Only bytes 12-15 change; the leading 12 bytes (nonce) are fixed. */
void inc32(uint8_t counter[16]) {
  for (int i = 15; i >= 12; --i) {
    counter[i] = (counter[i] + 1) % 256;
    if (counter[i] != 0) {
      break;
    } else {
      counter[i] = 0;
    }
  }
}

/* CTR encrypt/decrypt `len` bytes of `in` into `out` using initial counter block
 * `icb`. The final block may be partial (only `len % 16` bytes XORed), so no
 * padding is required. The caller's `icb` is not modified. */
void aes128_ctr(const uint8_t in[], size_t len, const uint8_t key[16], const uint8_t icb[16], uint8_t *out) {
  uint8_t ctr[16], keystream[16];

  // Copy ICB into counter blocks
  memcpy(ctr, icb, 16);

  size_t offset = 0;
  while (offset < len) {
    aes128_encrypt(ctr, key, keystream);
    size_t n;
    if (len - offset < 16) {
      n = len - offset;
    } else {
      n = 16;
    }

    for (size_t i = 0; i < n; ++i) {
      out[offset + i] = in[offset + i] ^ keystream[i];
    }

    inc32(ctr);
    offset += 16;
  }
};

/* ============================================================================
 * GHASH (SP 800-38D, Sec. 6.4)
 * ==========================================================================*/

/* GHASH over `num_blocks` 16-byte blocks with hash subkey H:
 * Y0 = 0; Yi = (Yi-1 XOR Xi) * H. `data` must already be block-aligned (any
 * zero-padding is the GCM layer's responsibility). */
void ghash(const uint8_t data[16], const uint8_t H[16], const size_t num_blocks, uint8_t out[16]) {
  uint8_t buf[16] = {0};
  for (size_t i = 0; i < num_blocks; ++i) {
    const uint8_t *block = data + 16 * i;
    for (size_t b = 0; b < 16; ++b) {
      buf[b] ^= block[b];
    }
    gf128_mul(buf, H, buf);
  }
  memcpy(out, buf, 16);
}

/* ============================================================================
 * AES-128-GCM (NIST SP 800-38D, Sec. 7 / McGrew-Viega)
 * Combines CTR-mode encryption with GHASH authentication. Restricted to 96-bit
 * (12-byte) IVs, the recommended and common case.
 * ==========================================================================*/

/* Encode a 64-bit value big-endian into 8 bytes. */
static void put_be64(uint8_t out[8], uint64_t v) {
  for (int i = 0; i < 8; ++i) out[7 - i] = (uint8_t)(v >> (8 * i));
}

/* Build GHASH input  A || 0^v || C || 0^u || [len(A)]64 || [len(C)]64  and hash
 * it (Algorithm 4, Steps 4-5). aad_len and ct_len are BYTE lengths; the length
 * block encodes BIT lengths. Result (the block S) goes in `S`. */
static void gcm_ghash_lengths(const uint8_t H[16], const uint8_t *aad, size_t aad_len, const uint8_t *ct, size_t ct_len,
                              uint8_t S[16]) {
  size_t a_blk = (aad_len + 15) / 16; /* AAD blocks, zero-padded up   */
  size_t c_blk = (ct_len + 15) / 16;  /* ciphertext blocks, padded up */
  size_t nblk = a_blk + c_blk + 1;    /* + 1 for the length block     */

  uint8_t *buf = calloc(nblk, 16);                                /* zeroed: gives the 0^v / 0^u padding */
  memcpy(buf, aad, aad_len);                                      /* AAD, remainder of its blocks stays 0 */
  memcpy(buf + a_blk * 16, ct, ct_len);                           /* ciphertext, likewise zero-padded     */
  put_be64(buf + (a_blk + c_blk) * 16, (uint64_t)aad_len * 8);    /* [len(A)]64 */
  put_be64(buf + (a_blk + c_blk) * 16 + 8, (uint64_t)ct_len * 8); /* [len(C)]64 */

  ghash(buf, H, nblk, S);
  free(buf);
}

/* Authenticated encryption. Produces ciphertext `ct` (same length as `pt`) and a
 * 128-bit tag. `iv` is 12 bytes. `aad` may be NULL if aad_len == 0. */
void gcm_encrypt(const uint8_t key[16], const uint8_t iv[12], const uint8_t *pt, size_t pt_len, const uint8_t *aad,
                 size_t aad_len, uint8_t *ct, uint8_t tag[16]) {
  uint8_t H[16], J0[16], icb[16], S[16], EJ0[16];
  static const uint8_t zero[16] = {0};

  aes128_encrypt(zero, key, H); /* Step 1: H = E(K, 0^128)           */

  memcpy(J0, iv, 12); /* Step 2: J0 = IV || 0^31 || 1      */
  J0[12] = 0;
  J0[13] = 0;
  J0[14] = 0;
  J0[15] = 1;

  memcpy(icb, J0, 16); /* Step 3: C = GCTR(inc32(J0), P)    */
  inc32(icb);
  aes128_ctr(pt, pt_len, key, icb, ct);

  gcm_ghash_lengths(H, aad, aad_len, ct, pt_len, S); /* Steps 4-5: S = GHASH(...) */

  aes128_encrypt(J0, key, EJ0); /* Step 6: T = MSB_t(GCTR(J0, S))     */
  for (int i = 0; i < 16; ++i)  /* single block => S XOR E(K, J0)     */
    tag[i] = S[i] ^ EJ0[i];
}

/* Constant-time 16-byte compare: returns 1 if equal, 0 otherwise. No early exit,
 * so it doesn't leak via timing where two tags first differ. */
static int ct_equal16(const uint8_t a[16], const uint8_t b[16]) {
  uint8_t diff = 0;
  for (int i = 0; i < 16; ++i) diff |= (uint8_t)(a[i] ^ b[i]);
  return diff == 0;
}

/* Authenticated decryption (SP 800-38D Algorithm 5). Verifies the tag BEFORE
 * releasing plaintext. Returns 0 on success (pt filled), -1 on auth failure
 * (pt zeroed, nothing trustworthy emitted). iv is 12 bytes; aad may be NULL if
 * aad_len == 0. pt must have room for ct_len bytes. */
int gcm_decrypt(const uint8_t key[16], const uint8_t iv[12], const uint8_t *ct, size_t ct_len, const uint8_t *aad,
                size_t aad_len, const uint8_t tag[16], uint8_t *pt) {
  uint8_t H[16], J0[16], icb[16], S[16], EJ0[16], Tprime[16];
  static const uint8_t zero[16] = {0};

  aes128_encrypt(zero, key, H); /* H = E(K, 0^128) */
  memcpy(J0, iv, 12);           /* J0 = IV || 0^31 || 1 */
  J0[12] = 0;
  J0[13] = 0;
  J0[14] = 0;
  J0[15] = 1;

  /* Recompute the tag from the RECEIVED ciphertext and AAD. */
  gcm_ghash_lengths(H, aad, aad_len, ct, ct_len, S);
  aes128_encrypt(J0, key, EJ0);
  for (int i = 0; i < 16; ++i) Tprime[i] = S[i] ^ EJ0[i];

  /* Verify FIRST. On mismatch, release nothing. */
  if (!ct_equal16(Tprime, tag)) {
    if (ct_len) memset(pt, 0, ct_len);
    return -1;
  }

  /* Authentic: decrypt. CTR is symmetric, so this is the same call as encrypt. */
  memcpy(icb, J0, 16);
  inc32(icb);
  aes128_ctr(ct, ct_len, key, icb, pt);
  return 0;
}

/* ============================================================================
 * Utilities
 * ==========================================================================*/

/* Coerce a NUL-terminated string into exactly 16 key bytes, truncating or
 * zero-padding as needed. Convenience for string keys; not used by the tests. */
void sanitizeKey(const char *input, uint8_t out[KEY_LEN]) {
  size_t len = strlen(input);
  for (size_t i = 0; i < KEY_LEN; ++i) out[i] = (i < len) ? (uint8_t)input[i] : 0x00;
}

/* ============================================================================
 * Test harness
 * ==========================================================================*/

static int g_run = 0, g_pass = 0;

static void report(const char *name, int ok, const uint8_t *got, const uint8_t *exp) {
  g_run++;
  if (ok) {
    g_pass++;
    printf("[PASS] %-34s\n", name);
    return;
  }
  printf("[FAIL] %-34s\n       got     :", name);
  for (size_t i = 0; i < 16; ++i) printf(" %02X", got[i]);
  printf("\n       expected:");
  for (size_t i = 0; i < 16; ++i) printf(" %02X", exp[i]);
  printf("\n");
}

static int bytes_equal(const uint8_t *a, const uint8_t *b) { return memcmp(a, b, 16) == 0; }

static void check_word(const char *name, uint32_t got, uint32_t expected) {
  g_run++;
  if (got == expected) {
    g_pass++;
    printf("[PASS] %-28s = 0x%08X\n", name, (unsigned)got);
  } else {
    printf("[FAIL] %-28s : got 0x%08X, expected 0x%08X\n", name, (unsigned)got, (unsigned)expected);
  }
}

/* Compare two 16-byte blocks, report pass/fail with hex diff. */
static void check_block(const char *name, const uint8_t got[16], const uint8_t expected[16]) {
  report(name, bytes_equal(got, expected), got, expected);
}

/* Compare `n` bytes; report pass/fail. For GCM buffers of arbitrary length. */
static void check_bytes(const char *name, const uint8_t *got, const uint8_t *exp, size_t n) {
  g_run++;
  if (memcmp(got, exp, n) == 0) {
    g_pass++;
    printf("[PASS] %-34s\n", name);
  } else {
    printf("[FAIL] %-34s\n       got     :", name);
    for (size_t i = 0; i < n; ++i) printf(" %02X", got[i]);
    printf("\n       expected:");
    for (size_t i = 0; i < n; ++i) printf(" %02X", exp[i]);
    printf("\n");
  }
}

/* Report a boolean condition (for return-code / rejection checks). */
static void check_cond(const char *name, int ok) {
  g_run++;
  if (ok) {
    g_pass++;
    printf("[PASS] %-34s\n", name);
  } else {
    printf("[FAIL] %-34s\n", name);
  }
}

/* Apply addRoundKey to a state built from `in` and compare to `expected`. */
static void check_addRoundKey(const char *name, const uint8_t in[16], const uint32_t rk[4],
                              const uint8_t expected[16]) {
  uint8_t state[4][4], out[16];
  load_state(state, in);
  addRoundKey(state, rk);
  store_state(state, out);
  report(name, bytes_equal(out, expected), out, expected);
}

/* Apply a single state transform to `in` and compare to `expected`. */
static void check_transform(const char *name, void (*fn)(uint8_t[4][4]), const uint8_t in[16],
                            const uint8_t expected[16]) {
  uint8_t state[4][4], out[16];
  load_state(state, in);
  fn(state);
  store_state(state, out);
  report(name, bytes_equal(out, expected), out, expected);
}

/* Confirm fwd then inv restores an arbitrary state (needs no external vector). */
static void check_transform_roundtrip(const char *name, void (*fwd)(uint8_t[4][4]), void (*inv)(uint8_t[4][4])) {
  uint8_t in[16], out[16], state[4][4];
  for (size_t i = 0; i < 16; ++i) in[i] = (uint8_t)(0x10 + i);
  load_state(state, in);
  fwd(state);
  inv(state);
  store_state(state, out);
  report(name, bytes_equal(out, in), out, in);
}

static void check_encrypt(const char *name, const uint8_t pt[16], const uint8_t key[16], const uint8_t expected[16]) {
  uint8_t out[16];
  aes128_encrypt(pt, key, out);
  report(name, bytes_equal(out, expected), out, expected);
}

static void check_decrypt(const char *name, const uint8_t ct[16], const uint8_t key[16], const uint8_t expected[16]) {
  uint8_t out[16];
  aes128_decrypt(ct, key, out);
  report(name, bytes_equal(out, expected), out, expected);
}

/* Confirm decrypt inverts encrypt for a given block/key. */
static void check_cipher_roundtrip(const char *name, const uint8_t pt[16], const uint8_t key[16]) {
  uint8_t ct[16], rt[16];
  aes128_encrypt(pt, key, ct);
  aes128_decrypt(ct, key, rt);
  report(name, bytes_equal(rt, pt), rt, pt);
}

/* Check invSbox is the exact inverse of sbox across all 256 bytes. */
static void verify_sbox_inverse(void) {
  int ok = 1;
  for (size_t x = 0; x < 16; ++x)
    for (size_t y = 0; y < 16; ++y) {
      uint8_t b = (uint8_t)((x << 4) | y);
      uint8_t fwd = sbox[b >> 4][b & 0x0F];
      if (invSbox[fwd >> 4][fwd & 0x0F] != b) ok = 0;
    }
  g_run++;
  if (ok) g_pass++;
  printf(ok ? "[PASS] sbox/invSbox inverse check\n" : "[FAIL] sbox/invSbox inverse check\n");
}

/* inc32: apply to a copy of `in`, compare all 16 bytes to `expected`. */
static void check_inc32(const char *name, const uint8_t in[16], const uint8_t expected[16]) {
  uint8_t c[16];
  memcpy(c, in, 16);
  inc32(c);
  report(name, bytes_equal(c, expected), c, expected);
}

/* CTR round-trip: encrypt then decrypt (same icb) must restore the input, for
 * any length. Also confirms the ciphertext actually differs from the plaintext,
 * so an all-zero-keystream no-op would fail rather than silently round-trip. */
static void check_ctr_roundtrip(const char *name, size_t len) {
  uint8_t key[16], icb[16], pt[128], ct[128], rt[128];
  for (size_t i = 0; i < 16; ++i) {
    key[i] = (uint8_t)(0x2b + i);
    icb[i] = (uint8_t)(0xf0 + i);
  }
  for (size_t i = 0; i < len; ++i) pt[i] = (uint8_t)(i * 7 + 1);

  aes128_ctr(pt, len, key, icb, ct); /* encrypt          */
  aes128_ctr(ct, len, key, icb, rt); /* decrypt (same icb) */

  int restored = (memcmp(rt, pt, len) == 0);
  int changed = (len == 0) || (memcmp(ct, pt, len) != 0); /* keystream was applied */

  g_run++;
  if (restored && changed) {
    g_pass++;
    printf("[PASS] %-28s (len=%3zu)\n", name, len);
  } else {
    printf("[FAIL] %-28s (len=%3zu) restored=%d changed=%d\n", name, len, restored, changed);
  }
}

/* Block 0 keystream: ctr(P) over one block == P XOR aes128_encrypt(icb). */
static void check_ctr_keystream_block0(void) {
  uint8_t key[16], icb[16], pt[16], ct[16], ks[16], expect[16];
  for (size_t i = 0; i < 16; ++i) {
    key[i] = (uint8_t)(0x00 + i); /* FIPS key 000102..0f */
    icb[i] = (uint8_t)(0xf0 + i);
    pt[i] = (uint8_t)(0x10 + i);
  }
  aes128_encrypt(icb, key, ks);
  for (size_t i = 0; i < 16; ++i) expect[i] = pt[i] ^ ks[i];

  aes128_ctr(pt, 16, key, icb, ct);
  report("ctr block0 = P XOR E(icb)", bytes_equal(ct, expect), ct, expect);
}

/* Block 1 keystream: second block of a 2-block ctr == P1 XOR E(inc32(icb)).
 * A broken inc32 would pass block0 but fail here. */
static void check_ctr_keystream_block1(void) {
  uint8_t key[16], icb[16], ctr2[16], pt[32], ct[32], ks2[16], expect2[16];
  for (size_t i = 0; i < 16; ++i) {
    key[i] = (uint8_t)(0x00 + i);
    icb[i] = (uint8_t)(0xf0 + i);
  }
  for (size_t i = 0; i < 32; ++i) pt[i] = (uint8_t)(0x10 + i);

  memcpy(ctr2, icb, 16);
  inc32(ctr2);
  aes128_encrypt(ctr2, key, ks2);
  for (size_t i = 0; i < 16; ++i) expect2[i] = pt[16 + i] ^ ks2[i];

  aes128_ctr(pt, 32, key, icb, ct);
  report("ctr block1 uses inc32(icb)", bytes_equal(ct + 16, expect2), ct + 16, expect2);
}

/* CTR against the NIST SP 800-38A F.5.1 CTR-AES128 known-answer vector. */
static void check_ctr_nist_f51(void) {
  static const uint8_t key[16] = {0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6,
                                  0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c};
  static const uint8_t icb[16] = {0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7,
                                  0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff};
  static const uint8_t pt[64] = {0x6b, 0xc1, 0xbe, 0xe2, 0x2e, 0x40, 0x9f, 0x96, 0xe9, 0x3d, 0x7e, 0x11, 0x73,
                                 0x93, 0x17, 0x2a, 0xae, 0x2d, 0x8a, 0x57, 0x1e, 0x03, 0xac, 0x9c, 0x9e, 0xb7,
                                 0x6f, 0xac, 0x45, 0xaf, 0x8e, 0x51, 0x30, 0xc8, 0x1c, 0x46, 0xa3, 0x5c, 0xe4,
                                 0x11, 0xe5, 0xfb, 0xc1, 0x19, 0x1a, 0x0a, 0x52, 0xef, 0xf6, 0x9f, 0x24, 0x45,
                                 0xdf, 0x4f, 0x9b, 0x17, 0xad, 0x2b, 0x41, 0x7b, 0xe6, 0x6c, 0x37, 0x10};
  static const uint8_t ct[64] = {0x87, 0x4d, 0x61, 0x91, 0xb6, 0x20, 0xe3, 0x26, 0x1b, 0xef, 0x68, 0x64, 0x99,
                                 0x0d, 0xb6, 0xce, 0x98, 0x06, 0xf6, 0x6b, 0x79, 0x70, 0xfd, 0xff, 0x86, 0x17,
                                 0x18, 0x7b, 0xb9, 0xff, 0xfd, 0xff, 0x5a, 0xe4, 0xdf, 0x3e, 0xdb, 0xd5, 0xd3,
                                 0x5e, 0x5b, 0x4f, 0x09, 0x02, 0x0d, 0xb0, 0x3e, 0xab, 0x1e, 0x03, 0x1d, 0xda,
                                 0x2f, 0xbe, 0x03, 0xd1, 0x79, 0x21, 0x70, 0xa0, 0xf3, 0x00, 0x9c, 0xee};

  uint8_t out[64];

  /* Encrypt: plaintext -> ciphertext */
  aes128_ctr(pt, 64, key, icb, out);
  int enc_ok = (memcmp(out, ct, 64) == 0);
  g_run++;
  if (enc_ok) {
    g_pass++;
    printf("[PASS] CTR NIST F.5.1 encrypt\n");
  } else {
    printf("[FAIL] CTR NIST F.5.1 encrypt\n");
  }

  /* Decrypt: same function on ciphertext -> plaintext */
  aes128_ctr(ct, 64, key, icb, out);
  int dec_ok = (memcmp(out, pt, 64) == 0);
  g_run++;
  if (dec_ok) {
    g_pass++;
    printf("[PASS] CTR NIST F.5.1 decrypt\n");
  } else {
    printf("[FAIL] CTR NIST F.5.1 decrypt\n");
  }
}

static void test_gf128_mul(void) {
  /* The multiplicative identity in GCM's little-endian field is bit 0 set:
     0x80 followed by fifteen 0x00 (the constant term "1"). */
  static const uint8_t one[16] = {0x80};
  static const uint8_t zero[16] = {0};

  /* A couple of arbitrary field elements. */
  static const uint8_t a[16] = {0x03, 0x88, 0xda, 0xce, 0x60, 0xb6, 0xa3, 0x92,
                                0xf3, 0x28, 0xc2, 0xb9, 0x71, 0xb2, 0xfe, 0x78};
  static const uint8_t b[16] = {0x66, 0xe9, 0x4b, 0xd4, 0xef, 0x8a, 0x2c, 0x3b,
                                0x88, 0x4c, 0xfa, 0x59, 0xca, 0x34, 0x2b, 0x2e};

  uint8_t r1[16], r2[16];

  /* 1. Identity: X * 1 == X */
  gf128_mul(a, one, r1);
  check_block("gf128 a*1 == a", r1, a);

  /* 2. Identity is commutative: 1 * X == X */
  gf128_mul(one, a, r1);
  check_block("gf128 1*a == a", r1, a);

  /* 3. Zero: X * 0 == 0 */
  gf128_mul(a, zero, r1);
  check_block("gf128 a*0 == 0", r1, zero);

  /* 4. Commutativity: a*b == b*a */
  gf128_mul(a, b, r1);
  gf128_mul(b, a, r2);
  check_block("gf128 a*b == b*a", r1, r2);

  /* 5. Aliasing safety: output into an input buffer must still be correct.
        Compute a*b normally, then a*b with out==first arg, compare. */
  gf128_mul(a, b, r1);
  uint8_t alias[16];
  memcpy(alias, a, 16);
  gf128_mul(alias, b, alias); /* out aliases x */
  check_block("gf128 a*b aliased out==x", alias, r1);

  /* 6. Distributivity: a*(b^c) == (a*b) ^ (a*c) */
  static const uint8_t c[16] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88,
                                0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff, 0x00};
  uint8_t bc[16], abc[16], ab[16], ac[16], sum[16];
  for (size_t i = 0; i < 16; ++i) bc[i] = b[i] ^ c[i];
  gf128_mul(a, bc, abc);
  gf128_mul(a, b, ab);
  gf128_mul(a, c, ac);
  for (size_t i = 0; i < 16; ++i) sum[i] = ab[i] ^ ac[i];
  check_block("gf128 distributive", abc, sum);
}

static void test_ghash(void) {
  /* H and some field elements (reuse the multiply test values). */
  static const uint8_t H[16] = {0x66, 0xe9, 0x4b, 0xd4, 0xef, 0x8a, 0x2c, 0x3b,
                                0x88, 0x4c, 0xfa, 0x59, 0xca, 0x34, 0x2b, 0x2e};
  static const uint8_t x1[16] = {0x03, 0x88, 0xda, 0xce, 0x60, 0xb6, 0xa3, 0x92,
                                 0xf3, 0x28, 0xc2, 0xb9, 0x71, 0xb2, 0xfe, 0x78};
  static const uint8_t x2[16] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88,
                                 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff, 0x00};

  uint8_t g[16], m[16], tmp[16];

  /* 1. Empty input: no blocks -> Y stays Y0 = 0. */
  static const uint8_t zero[16] = {0};
  ghash(NULL, H, 0, g); /* data unused when num_blocks==0 */
  check_block("ghash empty == 0", g, zero);

  /* 2. Single block: GHASH(X1) = (0 ^ X1) * H = X1 * H. */
  ghash(x1, H, 1, g);
  gf128_mul(x1, H, m);
  check_block("ghash 1 block == X1*H", g, m);

  /* 3. Two blocks, computed by hand from the recurrence:
        Y1 = X1 * H
        Y2 = (Y1 ^ X2) * H
     Build the reference with gf128_mul directly, then compare. */
  {
    uint8_t data[32];
    memcpy(data, x1, 16);
    memcpy(data + 16, x2, 16);

    gf128_mul(x1, H, tmp);                           /* Y1 = X1 * H            */
    for (size_t b = 0; b < 16; ++b) tmp[b] ^= x2[b]; /* Y1 ^ X2 */
    gf128_mul(tmp, H, m);                            /* Y2 = (Y1 ^ X2) * H     */

    ghash(data, H, 2, g);
    check_block("ghash 2 blocks == recurrence", g, m);
  }

  /* 4. Linearity in the first block: GHASH is built from XOR + multiply, both
        of which distribute over XOR. For a single block,
        GHASH(A ^ B) == GHASH(A) ^ GHASH(B), since (A^B)*H = A*H ^ B*H. */
  {
    uint8_t axb[16], ga[16], gb[16], gaxb[16], gsum[16];
    for (size_t b = 0; b < 16; ++b) axb[b] = x1[b] ^ x2[b];
    ghash(x1, H, 1, ga);
    ghash(x2, H, 1, gb);
    ghash(axb, H, 1, gaxb);
    for (size_t b = 0; b < 16; ++b) gsum[b] = ga[b] ^ gb[b];
    check_block("ghash single-block linear", gaxb, gsum);
  }
}

static void test_gcm(void) {
  uint8_t ct[64], tag[16], rec[64];
  int rc;

  /* ---- Test Case 1: empty plaintext, empty AAD ---- */
  {
    static const uint8_t key[16] = {0};
    static const uint8_t iv[12] = {0};
    static const uint8_t exp_tag[16] = {0x58, 0xe2, 0xfc, 0xce, 0xfa, 0x7e, 0x30, 0x61,
                                        0x36, 0x7f, 0x1d, 0x57, 0xa4, 0xe7, 0x45, 0x5a};

    gcm_encrypt(key, iv, NULL, 0, NULL, 0, ct, tag);
    check_bytes("GCM TC1 tag", tag, exp_tag, 16);

    rc = gcm_decrypt(key, iv, NULL, 0, NULL, 0, exp_tag, rec);
    check_cond("GCM TC1 decrypt authentic", rc == 0);
  }

  /* ---- Test Case 2: one zero block, empty AAD ---- */
  {
    static const uint8_t key[16] = {0};
    static const uint8_t iv[12] = {0};
    static const uint8_t p[16] = {0};
    static const uint8_t exp_ct[16] = {0x03, 0x88, 0xda, 0xce, 0x60, 0xb6, 0xa3, 0x92,
                                       0xf3, 0x28, 0xc2, 0xb9, 0x71, 0xb2, 0xfe, 0x78};
    static const uint8_t exp_tag[16] = {0xab, 0x6e, 0x47, 0xd4, 0x2c, 0xec, 0x13, 0xbd,
                                        0xf5, 0x3a, 0x67, 0xb2, 0x12, 0x57, 0xbd, 0xdf};

    gcm_encrypt(key, iv, p, 16, NULL, 0, ct, tag);
    check_bytes("GCM TC2 ciphertext", ct, exp_ct, 16);
    check_bytes("GCM TC2 tag", tag, exp_tag, 16);

    rc = gcm_decrypt(key, iv, exp_ct, 16, NULL, 0, exp_tag, rec);
    check_cond("GCM TC2 decrypt authentic", rc == 0);
    check_bytes("GCM TC2 recovered pt", rec, p, 16);
  }

  /* ---- Test Case 3: 64-byte plaintext, empty AAD, non-zero IV ---- */
  {
    static const uint8_t key[16] = {0xfe, 0xff, 0xe9, 0x92, 0x86, 0x65, 0x73, 0x1c,
                                    0x6d, 0x6a, 0x8f, 0x94, 0x67, 0x30, 0x83, 0x08};
    static const uint8_t iv[12] = {0xca, 0xfe, 0xba, 0xbe, 0xfa, 0xce, 0xdb, 0xad, 0xde, 0xca, 0xf8, 0x88};
    static const uint8_t p[64] = {0xd9, 0x31, 0x32, 0x25, 0xf8, 0x84, 0x06, 0xe5, 0xa5, 0x59, 0x09, 0xc5, 0xaf,
                                  0xf5, 0x26, 0x9a, 0x86, 0xa7, 0xa9, 0x53, 0x15, 0x34, 0xf7, 0xda, 0x2e, 0x4c,
                                  0x30, 0x3d, 0x8a, 0x31, 0x8a, 0x72, 0x1c, 0x3c, 0x0c, 0x95, 0x95, 0x68, 0x09,
                                  0x53, 0x2f, 0xcf, 0x0e, 0x24, 0x49, 0xa6, 0xb5, 0x25, 0xb1, 0x6a, 0xed, 0xf5,
                                  0xaa, 0x0d, 0xe6, 0x57, 0xba, 0x63, 0x7b, 0x39, 0x1a, 0xaf, 0xd2, 0x55};
    static const uint8_t exp_ct[64] = {0x42, 0x83, 0x1e, 0xc2, 0x21, 0x77, 0x74, 0x24, 0x4b, 0x72, 0x21, 0xb7, 0x84,
                                       0xd0, 0xd4, 0x9c, 0xe3, 0xaa, 0x21, 0x2f, 0x2c, 0x02, 0xa4, 0xe0, 0x35, 0xc1,
                                       0x7e, 0x23, 0x29, 0xac, 0xa1, 0x2e, 0x21, 0xd5, 0x14, 0xb2, 0x54, 0x66, 0x93,
                                       0x1c, 0x7d, 0x8f, 0x6a, 0x5a, 0xac, 0x84, 0xaa, 0x05, 0x1b, 0xa3, 0x0b, 0x39,
                                       0x6a, 0x0a, 0xac, 0x97, 0x3d, 0x58, 0xe0, 0x91, 0x47, 0x3f, 0x59, 0x85};
    static const uint8_t exp_tag[16] = {0x4d, 0x5c, 0x2a, 0xf3, 0x27, 0xcd, 0x64, 0xa6,
                                        0x2c, 0xf3, 0x5a, 0xbd, 0x2b, 0xa6, 0xfa, 0xb4};

    gcm_encrypt(key, iv, p, 64, NULL, 0, ct, tag);
    check_bytes("GCM TC3 ciphertext", ct, exp_ct, 64);
    check_bytes("GCM TC3 tag", tag, exp_tag, 16);

    rc = gcm_decrypt(key, iv, exp_ct, 64, NULL, 0, exp_tag, rec);
    check_cond("GCM TC3 decrypt authentic", rc == 0);
    check_bytes("GCM TC3 recovered pt", rec, p, 64);
  }

  /* ---- Test Case 4: 60-byte plaintext + 20-byte AAD (partial blocks both) ---- */
  {
    static const uint8_t key[16] = {0xfe, 0xff, 0xe9, 0x92, 0x86, 0x65, 0x73, 0x1c,
                                    0x6d, 0x6a, 0x8f, 0x94, 0x67, 0x30, 0x83, 0x08};
    static const uint8_t iv[12] = {0xca, 0xfe, 0xba, 0xbe, 0xfa, 0xce, 0xdb, 0xad, 0xde, 0xca, 0xf8, 0x88};
    static const uint8_t p[60] = {0xd9, 0x31, 0x32, 0x25, 0xf8, 0x84, 0x06, 0xe5, 0xa5, 0x59, 0x09, 0xc5,
                                  0xaf, 0xf5, 0x26, 0x9a, 0x86, 0xa7, 0xa9, 0x53, 0x15, 0x34, 0xf7, 0xda,
                                  0x2e, 0x4c, 0x30, 0x3d, 0x8a, 0x31, 0x8a, 0x72, 0x1c, 0x3c, 0x0c, 0x95,
                                  0x95, 0x68, 0x09, 0x53, 0x2f, 0xcf, 0x0e, 0x24, 0x49, 0xa6, 0xb5, 0x25,
                                  0xb1, 0x6a, 0xed, 0xf5, 0xaa, 0x0d, 0xe6, 0x57, 0xba, 0x63, 0x7b, 0x39};
    static const uint8_t aad[20] = {0xfe, 0xed, 0xfa, 0xce, 0xde, 0xad, 0xbe, 0xef, 0xfe, 0xed,
                                    0xfa, 0xce, 0xde, 0xad, 0xbe, 0xef, 0xab, 0xad, 0xda, 0xd2};
    static const uint8_t exp_ct[60] = {0x42, 0x83, 0x1e, 0xc2, 0x21, 0x77, 0x74, 0x24, 0x4b, 0x72, 0x21, 0xb7,
                                       0x84, 0xd0, 0xd4, 0x9c, 0xe3, 0xaa, 0x21, 0x2f, 0x2c, 0x02, 0xa4, 0xe0,
                                       0x35, 0xc1, 0x7e, 0x23, 0x29, 0xac, 0xa1, 0x2e, 0x21, 0xd5, 0x14, 0xb2,
                                       0x54, 0x66, 0x93, 0x1c, 0x7d, 0x8f, 0x6a, 0x5a, 0xac, 0x84, 0xaa, 0x05,
                                       0x1b, 0xa3, 0x0b, 0x39, 0x6a, 0x0a, 0xac, 0x97, 0x3d, 0x58, 0xe0, 0x91};
    static const uint8_t exp_tag[16] = {0x5b, 0xc9, 0x4f, 0xbc, 0x32, 0x21, 0xa5, 0xdb,
                                        0x94, 0xfa, 0xe9, 0x5a, 0xe7, 0x12, 0x1a, 0x47};

    gcm_encrypt(key, iv, p, 60, aad, 20, ct, tag);
    check_bytes("GCM TC4 ciphertext", ct, exp_ct, 60);
    check_bytes("GCM TC4 tag", tag, exp_tag, 16);

    rc = gcm_decrypt(key, iv, exp_ct, 60, aad, 20, exp_tag, rec);
    check_cond("GCM TC4 decrypt authentic", rc == 0);
    check_bytes("GCM TC4 recovered pt", rec, p, 60);

    /* Tamper: flip one tag byte -> must be rejected. */
    {
      uint8_t bad_tag[16];
      memcpy(bad_tag, exp_tag, 16);
      bad_tag[0] ^= 0x01;
      rc = gcm_decrypt(key, iv, exp_ct, 60, aad, 20, bad_tag, rec);
      check_cond("GCM TC4 tampered tag rejected", rc == -1);
    }

    /* Tamper: flip one ciphertext byte -> must be rejected. */
    {
      uint8_t bad_ct[60];
      memcpy(bad_ct, exp_ct, 60);
      bad_ct[5] ^= 0x80;
      rc = gcm_decrypt(key, iv, bad_ct, 60, aad, 20, exp_tag, rec);
      check_cond("GCM TC4 tampered ct rejected", rc == -1);
    }
  }
}

/* ============================================================================
 * main: run the full test suite
 * ==========================================================================*/

int main(void) {
  /* Common FIPS-197 AES-128 example vectors. */
  static const uint8_t fips_key[16] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                                       0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f};
  static const uint8_t fips_pt[16] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
                                      0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff};
  static const uint8_t fips_ct[16] = {0x69, 0xc4, 0xe0, 0xd8, 0x6a, 0x7b, 0x04, 0x30,
                                      0xd8, 0xcd, 0xb7, 0x80, 0x70, 0xb4, 0xc5, 0x5a};

  /* -- RotWord -- */
  check_word("RotWord(0x09cf4f3c)", RotWord(0x09cf4f3c), 0xcf4f3c09);
  check_word("RotWord(0x01020304)", RotWord(0x01020304), 0x02030401);
  check_word("RotWord(0x00000000)", RotWord(0x00000000), 0x00000000);
  check_word("RotWord(0xff000000)", RotWord(0xff000000), 0x000000ff);

  /* -- SubWord -- */
  check_word("SubWord(0x00000000)", SubWord(0x00000000), 0x63636363);
  check_word("SubWord(0xffffffff)", SubWord(0xffffffff), 0x16161616);
  check_word("SubWord(0x01234567)", SubWord(0x01234567), 0x7c266e85);
  check_word("SubWord(0xcf4f3c09)", SubWord(0xcf4f3c09), 0x8a84eb01);
  check_word("SubWord(RotWord(...))", SubWord(RotWord(0x09cf4f3c)), 0x8a84eb01);

  /* -- gmul (FIPS 197 Sec. 4.2 example: {57}*{13} = {fe}) -- */
  check_word("gmul(0x57,0x13)", gmul(0x57, 0x13), 0x00fe);

  /* -- addRoundKey -- */
  {
    static const uint8_t data[16] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
                                     0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff};
    static const uint32_t seq_rk[4] = {0x00010203, 0x04050607, 0x08090a0b, 0x0c0d0e0f};
    static const uint32_t zero_rk[4] = {0, 0, 0, 0};
    static const uint8_t zeros[16] = {0};
    static const uint8_t seq[16] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                                    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f};
    static const uint8_t round0[16] = {0x00, 0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70,
                                       0x80, 0x90, 0xa0, 0xb0, 0xc0, 0xd0, 0xe0, 0xf0};
    static const uint32_t rk_col[4] = {0x00112233, 0x44556677, 0x8899aabb, 0xccddeeff};

    check_addRoundKey("addRoundKey zero key = identity", data, zero_rk, data);
    check_addRoundKey("addRoundKey self-cancel = zero", seq, seq_rk, zeros);
    check_addRoundKey("addRoundKey FIPS round-0 vector", data, seq_rk, round0);
    check_addRoundKey("addRoundKey places rk[c] in col", zeros, rk_col, data);
  }

  /* -- Full-cipher KAT (FIPS-197 AES-128 example) -- */
  check_encrypt("AES-128 encrypt KAT", fips_pt, fips_key, fips_ct);
  check_decrypt("AES-128 decrypt KAT", fips_ct, fips_key, fips_pt);

  /* -- invShiftRows direct checks -- */
  {
    static const uint8_t in[16] = {0x00, 0x10, 0x20, 0x30, 0x01, 0x11, 0x21, 0x31,
                                   0x02, 0x12, 0x22, 0x32, 0x03, 0x13, 0x23, 0x33};
    static const uint8_t exp[16] = {0x00, 0x13, 0x22, 0x31, 0x01, 0x10, 0x23, 0x32,
                                    0x02, 0x11, 0x20, 0x33, 0x03, 0x12, 0x21, 0x30};
    static const uint8_t only0[16] = {0xAA, 0, 0, 0, 0xBB, 0, 0, 0, 0xCC, 0, 0, 0, 0xDD, 0, 0, 0};
    check_transform("invShiftRows right-shift by row", invShiftRows, in, exp);
    check_transform("invShiftRows leaves row 0", invShiftRows, only0, only0);
  }

  /* -- invMixColumns direct check: 8e 4d a1 bc -> db 13 53 45 per column -- */
  {
    static const uint8_t in[16] = {0x8e, 0x4d, 0xa1, 0xbc, 0x8e, 0x4d, 0xa1, 0xbc,
                                   0x8e, 0x4d, 0xa1, 0xbc, 0x8e, 0x4d, 0xa1, 0xbc};
    static const uint8_t exp[16] = {0xdb, 0x13, 0x53, 0x45, 0xdb, 0x13, 0x53, 0x45,
                                    0xdb, 0x13, 0x53, 0x45, 0xdb, 0x13, 0x53, 0x45};
    check_transform("invMixColumns known column", invMixColumns, in, exp);
  }

  /* -- Table integrity and transform round-trips -- */
  verify_sbox_inverse();
  check_transform_roundtrip("shiftRows/invShiftRows round-trip", shiftRows, invShiftRows);
  check_transform_roundtrip("subBytes/invSubBytes round-trip", subBytes, invSubBytes);
  check_transform_roundtrip("mixColumns/invMixColumns round-trip", mixColumns, invMixColumns);

  /* -- Full encrypt/decrypt round-trips -- */
  {
    static const uint8_t zpt[16] = {0}, zk[16] = {0};
    static const uint8_t rpt[16] = {0xde, 0xad, 0xbe, 0xef, 0x01, 0x23, 0x45, 0x67,
                                    0x89, 0xab, 0xcd, 0xef, 0xfe, 0xdc, 0xba, 0x98};
    static const uint8_t rk[16] = {0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6,
                                   0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c};
    check_cipher_roundtrip("encrypt/decrypt FIPS block", fips_pt, fips_key);
    check_cipher_roundtrip("encrypt/decrypt all-zero", zpt, zk);
    check_cipher_roundtrip("encrypt/decrypt arbitrary", rpt, rk);
  }

  /* -- inc32 -- */
  {
    /* nonce = bytes 00..0b (must stay fixed); counter = trailing 4 bytes */
    static const uint8_t base[16] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 0x00, 0x00, 0x00, 0x00};
    static const uint8_t inc1[16] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 0x00, 0x00, 0x00, 0x01};
    check_inc32("inc32 basic +1", base, inc1);

    static const uint8_t byteff[16] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 0x00, 0x00, 0x00, 0xff};
    static const uint8_t carry1[16] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 0x00, 0x00, 0x01, 0x00};
    check_inc32("inc32 carry one byte", byteff, carry1);

    static const uint8_t twoff[16] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 0x00, 0x00, 0xff, 0xff};
    static const uint8_t carry2[16] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 0x00, 0x01, 0x00, 0x00};
    check_inc32("inc32 carry two bytes", twoff, carry2);

    /* full 32-bit wrap: counter ffffffff -> 00000000, nonce untouched */
    static const uint8_t allff[16] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 0xff, 0xff, 0xff, 0xff};
    static const uint8_t wrap[16] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 0x00, 0x00, 0x00, 0x00};
    check_inc32("inc32 wraps, nonce fixed", allff, wrap);
  }

  /* -- CTR keystream vs aes128_encrypt -- */
  check_ctr_keystream_block0();
  check_ctr_keystream_block1();

  /* -- CTR round-trip across block boundaries and partial blocks -- */
  check_ctr_roundtrip("ctr round-trip", 1);
  check_ctr_roundtrip("ctr round-trip", 15);
  check_ctr_roundtrip("ctr round-trip", 16);
  check_ctr_roundtrip("ctr round-trip", 17);
  check_ctr_roundtrip("ctr round-trip", 31);
  check_ctr_roundtrip("ctr round-trip", 32);
  check_ctr_roundtrip("ctr round-trip", 63);

  check_ctr_nist_f51();

  /* -- GF(2^128) multiply -- */
  test_gf128_mul();

  /* -- GHASH -- */
  test_ghash();

  /* -- AES-128-GCM (McGrew-Viega Appendix B) -- */
  test_gcm();

  printf("\n%d/%d tests passed\n", g_pass, g_run);
  return g_pass == g_run ? 0 : 1;
}
