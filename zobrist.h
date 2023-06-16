#ifndef BOGSHAHMAT_ZOBRIST_H
#define BOGSHAHMAT_ZOBRIST_H

#include "bitboards_and_constants.h"

unsigned int random_state = 1804289383;

unsigned int getRandomU32() {
    unsigned int number = random_state;
    number ^= number << 13;
    number ^= number >> 17;
    number ^= number << 5;
    random_state = number;
    return number;
}

u64 getRandomU64() {
    u64 n1, n2, n3, n4;
    n1 = (u64) (getRandomU32()) & 0xFFFF;
    n2 = (u64) (getRandomU32()) & 0xFFFF;
    n3 = (u64) (getRandomU32()) & 0xFFFF;
    n4 = (u64) (getRandomU32()) & 0xFFFF;
    return n1 | (n2 << 16) | (n3 << 32) | (n4 << 48);
}

u64 piece_keys[12][64];
u64 enpassant_keys[64];
u64 castle_keys[16];
u64 color_key;

void fillHashKeys() {
    for (int i = P; i <= k; i++) {
        for (int j = 0; j < 64; j++) {
            piece_keys[i][j] = getRandomU64();
        }
    }
    for (int i = 0; i < 64; i++) {
        enpassant_keys[i] = getRandomU64();
    }
    for (int i = 0; i < 16; i++) {
        castle_keys[i] = getRandomU64();
    }
    color_key = getRandomU64();
}

u64 generateHashKey() {
    u64 hash_key = 0ULL;
    for (int i = P; i <= k; i++) {
        u64 bitboard_copy = bitboards[i];
        while (bitboard_copy) {
            int square = getLSBIndex(bitboard_copy);
            pop_bit(bitboard_copy, square);
            hash_key ^= piece_keys[i][square];
        }
    }
    if (enpassant_square != empty_square) {
        hash_key ^= enpassant_keys[enpassant_square];
    }
    hash_key ^= castle_keys[castle];
    if (move_color) {
        hash_key ^= color_key;
    }
    return hash_key;
}

#endif //BOGSHAHMAT_ZOBRIST_H
