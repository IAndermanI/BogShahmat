#ifndef BOGSHAHMAT_MOVE_LOGIC_H
#define BOGSHAHMAT_MOVE_LOGIC_H

#include "bitboards_and_constants.h"
#include <x86intrin.h>

// PRE-COUNTED MOVES
u64 pawn_attacks[64][2];
u64 knight_moves[64];
u64 king_moves[64];
u64 rook_masks[64];
u64 bishop_masks[64];
u64 bishop_moves[64][8192];
u64 rook_moves[64][16384];

//PAWN
void fillPawnAttacks() {
    u64 pawn = 1;
    for (int i = 0; i < 64; i++) {
        u64 ans = 0;
        u64 copy = pawn;
        copy <<= 7;
        if (copy & not_h_file) {
            ans |= copy;
        }
        copy = pawn;
        copy <<= 9;
        if (copy & not_a_file) {
            ans |= copy;
        }
        pawn_attacks[i][0] = ans;
        pawn <<= 1;
    }
    pawn = 1;
    for (int i = 0; i < 64; i++) {
        u64 ans = 0;
        u64 copy = pawn;
        copy >>= 7;
        if (copy & not_a_file) {
            ans |= copy;
        }
        copy = pawn;
        copy >>= 9;
        if (copy & not_h_file) {
            ans |= copy;
        }
        pawn_attacks[i][1] = ans;
        pawn <<= 1;
    }
}

// KING
void fillKingMoves() {
    u64 king = 1;
    for (int i = 0; i < 64; i++) {
        u64 ans = 0;
        u64 up1 = king << 8;
        u64 down1 = king >> 8;
        ans |= (up1 << 1) & not_a_file;
        ans |= (up1 >> 1) & not_h_file;
        ans |= (down1 << 1) & not_a_file;
        ans |= (down1 >> 1) & not_h_file;
        ans |= (king << 1) & not_a_file;
        ans |= (king >> 1) & not_h_file;
        ans |= up1;
        ans |= down1;
        king_moves[i] = ans;
        king = king << 1;
    }
}

// KNIGHT
void fillKnightMoves() {
    u64 knight = 1;
    for (int i = 0; i < 64; i++) {
        u64 ans = 0;
        u64 up1 = knight << 8;
        u64 up2 = knight << 16;
        u64 down1 = knight >> 8;
        u64 down2 = knight >> 16;
        ans |= (up1 << 2) & not_a_file & not_b_file;
        ans |= (up1 >> 2) & not_h_file & not_g_file;
        ans |= (down1 << 2) & not_a_file & not_b_file;
        ans |= (down1 >> 2) & not_h_file & not_g_file;
        ans |= (up2 << 1) & not_a_file;
        ans |= (up2 >> 1) & not_h_file;
        ans |= (down2 << 1) & not_a_file;
        ans |= (down2 >> 1) & not_h_file;
        knight_moves[i] = ans;
        knight = knight << 1;
    }
}

// SLIDING PIECES
void fillRookMasks() {
    u64 rook = 1;
    for (int i = 0; i < 64; i++) {
        u64 ans = 0;
        u64 copy = rook << 8;
        while (copy) {
            ans |= copy;
            copy <<= 8;
        }
        copy = rook >> 8;
        while (copy) {
            ans |= copy;
            copy >>= 8;
        }
        copy = rook << 1;
        while (!(copy & a_file) && copy) {
            ans |= copy;
            copy <<= 1;
        }
        copy = rook >> 1;
        while (!(copy & h_file) && copy) {
            ans |= copy;
            copy >>= 1;
        }
        rook_masks[i] = ans;
        rook <<= 1;
    }
}

void fillBishopMasks() {
    u64 bishop = 1;
    for (int i = 0; i < 64; i++) {
        u64 ans = 0;
        u64 copy = bishop << 9;
        while (!(copy & a_file) && copy) {
            ans |= copy;
            copy <<= 9;
        }
        copy = bishop << 7;
        while (!(copy & h_file) && copy) {
            ans |= copy;
            copy <<= 7;
        }
        copy = bishop >> 9;
        while (!(copy & h_file) && copy) {
            ans |= copy;
            copy >>= 9;
        }
        copy = bishop >> 7;
        while (!(copy & a_file) && copy) {
            ans |= copy;
            copy >>= 7;
        }
        bishop_masks[i] = ans;
        bishop <<= 1;
    }
}

u64 initBishopAttacks(int square, u64 block) {
    u64 attacks = 0ULL;
    int tr = square / 8;
    int tf = square % 8;
    int r, f;
    for (r = tr + 1, f = tf + 1; r <= 7 && f <= 7; r++, f++) {
        attacks |= (1ULL << (r * 8 + f));
        if (block & (1ULL << (r * 8 + f))) break;
    }

    for (r = tr + 1, f = tf - 1; r <= 7 && f >= 0; r++, f--) {
        attacks |= (1ULL << (r * 8 + f));
        if (block & (1ULL << (r * 8 + f))) break;
    }

    for (r = tr - 1, f = tf + 1; r >= 0 && f <= 7; r--, f++) {
        attacks |= (1ULL << (r * 8 + f));
        if (block & (1ULL << (r * 8 + f))) break;
    }

    for (r = tr - 1, f = tf - 1; r >= 0 && f >= 0; r--, f--) {
        attacks |= (1ULL << (r * 8 + f));
        if (block & (1ULL << (r * 8 + f))) break;
    }
    return attacks;
}

u64 initRookAttacks(int square, u64 block) {
// attacks bitboard
    u64 attacks = 0ULL;

// init files & ranks
    int f, r;

// init target files & ranks
    int tr = square / 8;
    int tf = square % 8;

// generate attacks
    for (r = tr + 1; r <= 7; r++) {
        attacks |= (1ULL << (r * 8 + tf));
        if (block & (1ULL << (r * 8 + tf))) break;
    }

    for (r = tr - 1; r >= 0; r--) {
        attacks |= (1ULL << (r * 8 + tf));
        if (block & (1ULL << (r * 8 + tf))) break;
    }

    for (f = tf + 1; f <= 7; f++) {
        attacks |= (1ULL << (tr * 8 + f));
        if (block & (1ULL << (tr * 8 + f))) break;
    }

    for (f = tf - 1; f >= 0; f--) {
        attacks |= (1ULL << (tr * 8 + f));
        if (block & (1ULL << (tr * 8 + f))) break;
    }
    return attacks;
}

void fillRookMoves() {
    for (int i = 0; i < 64; i++) {
        u64 mask = rook_masks[i];
        int variations = 1 << countBits(mask);
        for (int j = 0; j < variations; j++) {
            u64 occupancy = _pdep_u64(j, rook_masks[i]);
            u64 ind = _pext_u64(occupancy, rook_masks[i]);
            rook_moves[i][ind] = initRookAttacks(i, occupancy);
        }
    }
}

void fillBishopMoves() {
    for (int i = 0; i < 64; i++) {
        u64 mask = bishop_masks[i];
        int variations = 1 << countBits(mask);
        for (int j = 0; j < variations; j++) {
            u64 occupancy = _pdep_u64(j, bishop_masks[i]);
            u64 ind = _pext_u64(occupancy, bishop_masks[i]);
            bishop_moves[i][ind] = initBishopAttacks(i, occupancy);
        }
    }
}

static inline u64 getBishopAttacks(int square, u64 occupancy) {
    return bishop_moves[square][_pext_u64(occupancy & bishop_masks[square], bishop_masks[square])];
}

static inline u64 getRookAttacks(int square, u64 occupancy) {
    return rook_moves[square][_pext_u64(occupancy & rook_masks[square], rook_masks[square])];
}

static inline u64 getQueenAttacks(int square, u64 occupancy) {
    return getBishopAttacks(square, occupancy) | getRookAttacks(square, occupancy);
}

void fillAllMoves() {
    fillPawnAttacks();
    fillKingMoves();
    fillKnightMoves();
    fillRookMasks();
    fillBishopMasks();
    fillRookMoves();
    fillBishopMoves();
}

#endif //BOGSHAHMAT_MOVE_LOGIC_H
