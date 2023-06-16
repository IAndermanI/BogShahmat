#ifndef BOGSHAHMAT_BITBOARDS_AND_CONSTANTS_H
#define BOGSHAHMAT_BITBOARDS_AND_CONSTANTS_H

#include <bits/stdc++.h>

using namespace std;

#define get_bit(bitboard, square) ((bitboard) & (1ULL << square))
#define pop_bit(bitboard, square) (get_bit(bitboard, square) ? ((bitboard) ^= (1ULL << square)) : 0ULL)
#define set_bit(bitboard, square) ((bitboard) |= (1ULL << square))

typedef uint64_t u64;

enum {
    a1, b1, c1, d1, e1, f1, g1, h1,
    a2, b2, c2, d2, e2, f2, g2, h2,
    a3, b3, c3, d3, e3, f3, g3, h3,
    a4, b4, c4, d4, e4, f4, g4, h4,
    a5, b5, c5, d5, e5, f5, g5, h5,
    a6, b6, c6, d6, e6, f6, g6, h6,
    a7, b7, c7, d7, e7, f7, g7, h7,
    a8, b8, c8, d8, e8, f8, g8, h8, empty_square
};

enum {
    P, N, B, R, Q, K, p, n, b, r, q, k
};
string name_string = "PNBRQKpnbrqk";

u64 not_a_file = 0xfefefefefefefefe;
u64 a_file = ~not_a_file;
u64 not_b_file = 0xfdfdfdfdfdfdfdfd;
u64 not_h_file = 0x7f7f7f7f7f7f7f7f;
u64 h_file = ~not_h_file;
u64 not_g_file = 0xbfbfbfbfbfbfbfbf;
u64 g_file = ~not_g_file;
u64 not_1_row = 0xffffffffffffff00;
u64 not_8_row = 0x00ffffffffffffff;

u64 bitboards[12] = {0x000000000000ff00, 0x0000000000000042, 0x0000000000000024, 0x0000000000000081, 0x0000000000000008,
                     0x0000000000000010,
                     0x00ff000000000000, 0x4200000000000000, 0x2400000000000000, 0x8100000000000000, 0x0800000000000000,
                     0x1000000000000000};

// All white, all black and all together
u64 occupancies[3] = {0x000000000000ffff, 0xffff000000000000, 0xffff00000000ffff};

const int rook_bits[64] = {
        12, 11, 11, 11, 11, 11, 11, 12,
        11, 10, 10, 10, 10, 10, 10, 11,
        11, 10, 10, 10, 10, 10, 10, 11,
        11, 10, 10, 10, 10, 10, 10, 11,
        11, 10, 10, 10, 10, 10, 10, 11,
        11, 10, 10, 10, 10, 10, 10, 11,
        11, 10, 10, 10, 10, 10, 10, 11,
        12, 11, 11, 11, 11, 11, 11, 12
};

const int bishop_bits[64] = {
        6, 5, 5, 5, 5, 5, 5, 6,
        5, 5, 5, 5, 5, 5, 5, 5,
        5, 5, 7, 7, 7, 7, 5, 5,
        5, 5, 7, 9, 9, 7, 5, 5,
        5, 5, 7, 9, 9, 7, 5, 5,
        5, 5, 7, 7, 7, 7, 5, 5,
        5, 5, 5, 5, 5, 5, 5, 5,
        6, 5, 5, 5, 5, 5, 5, 6
};

// To track castle rights
const int castling_rights[64] = {
        13, 15, 15, 15, 12, 15, 15, 14,
        15, 15, 15, 15, 15, 15, 15, 15,
        15, 15, 15, 15, 15, 15, 15, 15,
        15, 15, 15, 15, 15, 15, 15, 15,
        15, 15, 15, 15, 15, 15, 15, 15,
        15, 15, 15, 15, 15, 15, 15, 15,
        15, 15, 15, 15, 15, 15, 15, 15,
        7, 15, 15, 15, 3, 15, 15, 11
};


// To track some info about the position
bool move_color = 0;
int enpassant_square = empty_square;

// Castling rules:
// Bitstring of length 4 represents what can castle.
// Example: 1011. Black king can castle long, white can castle both short and long.
enum {
    wk = 1, wq = 2, bk = 4, bq = 8
};
int castle = 0;
u64 hash_key = 0ULL;


char pieceType(u64 board, int square) {
    for (int i = 0; i < 12; i++) {
        if (get_bit(bitboards[i] & board, square)) {
            return name_string[i];
        }
    }
    return '.';
}

string squareString(int square) {
    string file = "abcdefgh";
    string rank = "12345678";
    string ans;
    ans += file[square % 8];
    ans += rank[square / 8];
    return ans;
}

int stringSquare(string str) {
    return (str[0] - 'a') + (str[1] - '1') * 8;
}

int countBits(u64 bitboard) {
    int count = 0;
    while (bitboard) {
        count++;
        bitboard &= bitboard - 1;
    }
    return count;
}

int getLSBIndex(u64 bitboard) {
    if (bitboard != 0)
        return countBits((bitboard & -bitboard) - 1);
    else
        return -1;
}


#endif //BOGSHAHMAT_BITBOARDS_AND_CONSTANTS_H
