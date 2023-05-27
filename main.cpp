//MagnusDestroyer version 2.0 (still in progress)
#include <iostream>
#include <cstdint>
#include <bitset>
#include <vector>
#include <windows.h>

using namespace std;
#define get_bit(bitboard, square) ((bitboard) & (1ULL << square))
#define pop_bit(bitboard, square) (get_bit(bitboard, square) ? ((bitboard) ^= (1ULL << square)) : 0ULL)
#define set_bit(bitboard, square) ((bitboard) |= (1ULL << square))
typedef uint64_t u64;

// DESCRIPTION

// 0 - WHITE, 1 - BLACK
//
// Bitboard - representation of a chess board. Using 64-bit unsigned integer to define the board with bits.
// Less important bit - a1, then a2, then a3, ..., then b1, b2, ... and the most important - h8.
// Example:
// FEN: r2qk2r/ppp1bpp1/1n1p3p/3P4/3P4/5Q1P/PP3PP1/R1B1R1K1 w kq - 3 14
// Bitboard:
// 8 1 . . 1 1 . . 1
// 7 1 1 1 . 1 1 1 .
// 6 . 1 . 1 . . . 1
// 5 . . . 1 . . . .
// 4 . . . 1 . . . .
// 3 . . . . . 1 . 1
// 2 1 1 . . . 1 1 .
// 1 1 . 1 . 1 . 1 .
//   a b c d e f g h
//
// P - pawn, N - knight, B - bishop, R - rook, Q - queen, K - king. Capital letters - white pieces, lowercase - black.
//
// Pre-counted arrays used to find bitboard of attacks very fast.
//
// Castling: 4 bits: for short and long castle ability of white and black king.
// Example: 1011. Black king can castle long, white can castle both short and long.
//
// Move in 32-bit integer:
// Responsible bits                                     Hexadecimal numbers
// 0000 0000 0000 0000 0011 1111    source square       0x3f
// 0000 0000 0000 1111 1100 0000    target square       0xfc0
// 0000 0000 1111 0000 0000 0000    piece               0xf000
// 0000 1111 0000 0000 0000 0000    promoted piece      0xf0000
// 0001 0000 0000 0000 0000 0000    capture flag        0x100000
// 0010 0000 0000 0000 0000 0000    double push flag    0x200000
// 0100 0000 0000 0000 0000 0000    en-passant flag     0x400000
// 1000 0000 0000 0000 0000 0000    castling flag       0x800000
// Move list is just a vector of integers (moves)

// CONSTANTS

enum {a1, b1, c1, d1, e1, f1, g1, h1,
      a2, b2, c2, d2, e2, f2, g2, h2,
      a3, b3, c3, d3, e3, f3, g3, h3,
      a4, b4, c4, d4, e4, f4, g4, h4,
      a5, b5, c5, d5, e5, f5, g5, h5,
      a6, b6, c6, d6, e6, f6, g6, h6,
      a7, b7, c7, d7, e7, f7, g7, h7,
      a8, b8, c8, d8, e8, f8, g8, h8, empty_square
};
u64 not_a_file = 0xfefefefefefefefe;
u64 a_file = ~not_a_file;
u64 not_b_file = 0xfdfdfdfdfdfdfdfd;
u64 not_h_file = 0x7f7f7f7f7f7f7f7f;
u64 h_file = ~not_h_file;
u64 not_g_file = 0xbfbfbfbfbfbfbfbf;
u64 g_file = ~not_g_file;


// White pawns, knights, bishops, rooks, queen and king. Then black in the same order.
string name_string = "PNBRQKpnbrqk";
enum {
    P, N, B, R, Q, K, p, n, b, r, q, k
};
u64 bitboards[12] = {0x000000000000ff00, 0x0000000000000042, 0x0000000000000024, 0x0000000000000081, 0x0000000000000008,
                     0x0000000000000010,
                     0x00ff000000000000, 0x4200000000000000, 0x2400000000000000, 0x8100000000000000, 0x0800000000000000,
                     0x1000000000000000};
// All white, all black and all together
u64 occupancies[3] = {0x000000000000ffff, 0xffff000000000000, 0xffff00000000ffff};

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


// To count possible moves amount for sliding attack pieces
int rook_bits[64] = {
        12, 11, 11, 11, 11, 11, 11, 12,
        11, 10, 10, 10, 10, 10, 10, 11,
        11, 10, 10, 10, 10, 10, 10, 11,
        11, 10, 10, 10, 10, 10, 10, 11,
        11, 10, 10, 10, 10, 10, 10, 11,
        11, 10, 10, 10, 10, 10, 10, 11,
        11, 10, 10, 10, 10, 10, 10, 11,
        12, 11, 11, 11, 11, 11, 11, 12
};

int bishop_bits[64] = {
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
        7, 15, 15, 15,  3, 15, 15, 11
};

// PRE-COUNTED MOVES
u64 pawn_attacks[64][2];
u64 knight_moves[64];
u64 king_moves[64];
u64 rook_masks[64];
u64 bishop_masks[64];
u64 bishop_moves[64][512];
u64 rook_moves[64][4096];
u64 magic_rook[64];
u64 magic_bishop[64];


// FUNCTIONS TO WORK WITH CONSTANTS
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

// FUNCTIONS FOR MOVING PIECES

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

// SHIFTING PIECES
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

int countBits(u64 bitboard) {
    int count = 0;
    while (bitboard) {
        count++;
        bitboard &= bitboard - 1;
    }
    return count;
}

int get_ls1b_index(u64 bitboard) {
    if (bitboard != 0)
        return countBits((bitboard & -bitboard) - 1);
    else
        return -1;
}

u64 set_occupancy(int index, int bits_in_mask, u64 attack_mask) {
    u64 occupancy = 0;
    for (int count = 0; count < bits_in_mask; count++) {
        int square = get_ls1b_index(attack_mask);
        pop_bit(attack_mask, square);
        if (index & (1 << count))
            occupancy |= (1 << square);
    }
    return occupancy;
}

u64 bishop_attacks_on_the_fly(int square, u64 block) {
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

u64 rook_attacks_on_the_fly(int square, u64 block) {
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
            u64 board = set_occupancy(j, countBits(mask), mask);
            u64 ind = (board * magic_rook[i]) >> (64 - rook_bits[i]);
            rook_moves[i][ind] = rook_attacks_on_the_fly(i, board);
        }
    }
}

void fillBishopMoves() {
    for (int i = 0; i < 64; i++) {
        u64 mask = bishop_masks[i];
        int variations = 1 << countBits(mask);
        for (int j = 0; j < variations; j++) {
            u64 board = set_occupancy(j, countBits(mask), mask);
            u64 ind = (board * magic_bishop[i]) >> (64 - rook_bits[i]);
            bishop_moves[i][ind] = bishop_attacks_on_the_fly(i, board);
        }
    }
}

static inline u64 getBishopAttacks(int square, u64 occupancy) {
    /*occupancy &= bishop_masks[square];
    occupancy *=  magic_bishop[square];
    occupancy >>= (64 - bishop_bits[square]);
    return bishop_moves[square][occupancy];
     */
    return bishop_attacks_on_the_fly(square, occupancy);

}

static inline u64 getRookAttacks(int square, u64 occupancy) {
/*occupancy &= rook_masks[square];
occupancy *=  magic_rook[square];
occupancy >>= (64 - rook_bits[square]);
return rook_moves[square][occupancy];
 */
    return rook_attacks_on_the_fly(square, occupancy);
}

static inline u64 getQueenAttacks(int square, u64 occupancy) {
    /*u64 occ_bishop = occupancy;
    occ_bishop &= bishop_masks[square];
    occ_bishop *=  magic_bishop[square];
    occ_bishop >>= 64 - bishop_bits[square];
    u64 occ_rook = occupancy;
    occ_rook &= rook_masks[square];
    occ_rook *=  magic_rook[square];
    occ_rook >>= 64 - rook_bits[square];
    return bishop_moves[square][occ_bishop] | rook_moves[square][occ_rook];
     */
    return bishop_attacks_on_the_fly(square, occupancy) | rook_attacks_on_the_fly(square, occupancy);
}


// EVERYTHING TO MOVE
static inline bool isAttacked(int square, bool color) {
    if (!color && (pawn_attacks[square][1] & bitboards[P])) return 1;
    if (color && (pawn_attacks[square][0] & bitboards[p])) return 1;
    if (knight_moves[square] & (!color ? bitboards[N] : bitboards[n])) return 1;
    if (getBishopAttacks(square, occupancies[2]) &
        (!color ? (bitboards[B] | bitboards[Q]) : (bitboards[b] | bitboards[q])))
        return 1;
    if (getRookAttacks(square, occupancies[2]) &
        (!color ? (bitboards[R] | bitboards[Q]) : (bitboards[r] | bitboards[q])))
        return 1;
    if (king_moves[square] & (!color ? bitboards[K] : bitboards[k])) return 1;

    return 0;
}

static inline int setMove(int source, int target, int piece, int promoted_piece,
                   int capture, int double_push, int en_passant, int castling) {
    return source | (target << 6) | (piece << 12) | (promoted_piece << 16) | (capture << 20) | (double_push << 21)
           | (en_passant << 22) | (castling << 23);
}

#define copy_board u64 bitboard_copy[12]; \
        u64 occupancies_copy[3]; \
        memcpy(bitboard_copy, bitboards, 96); \
        memcpy(occupancies_copy, occupancies, 24); \
        bool move_color_copy = move_color; \
        int enpassant_copy = enpassant_square; \
        int castle_copy = castle;

#define restore_board memcpy(bitboards, bitboard_copy, 96); \
        memcpy(occupancies, occupancies_copy, 24); \
        move_color = move_color_copy; \
        enpassant_square = enpassant_copy; \
        castle = castle_copy;

static inline void generateMoves(vector<int>* moves) {
    int source, target;
    u64 bitboard, attacks;
    for (int i = P; i <= k; i++) {
        bitboard = bitboards[i];
        if (!move_color) {
            if (i == P) {
                while (bitboard) {
                    source = get_ls1b_index(bitboard);
                    target = source + 8;
                    if (target <= h8 && !get_bit(occupancies[2], target)) {
                        if (target >= a8) {
                            moves->push_back(setMove(source, target, P, N, 0, 0, 0, 0));
                            moves->push_back(setMove(source, target, P, B, 0, 0, 0, 0));
                            moves->push_back(setMove(source, target, P, R, 0, 0, 0, 0));
                            moves->push_back(setMove(source, target, P, Q, 0, 0, 0, 0));

                        } else {
                            moves->push_back(setMove(source, target, P, 0, 0, 0, 0, 0));
                        }
                        if (source < a3 && source >= a2 && !get_bit(occupancies[2], (target + 8))) {
                            moves->push_back(setMove(source, target + 8, P, 0, 0, 1, 0, 0));

                        }
                    }
                    attacks = pawn_attacks[source][0] & (occupancies[1] | 1ULL << enpassant_square);
                    while (attacks) {
                        int attack_square = get_ls1b_index(attacks);
                        if (get_bit(occupancies[1], attack_square)) {
                            if (attack_square >= a8) {
                                moves->push_back(setMove(source, attack_square, P, N, 1, 0, 0, 0));
                                moves->push_back(setMove(source, attack_square, P, B, 1, 0, 0, 0));
                                moves->push_back(setMove(source, attack_square, P, R, 1, 0, 0, 0));
                                moves->push_back(setMove(source, attack_square, P, Q, 1, 0, 0, 0));
                            } else {
                                moves->push_back(setMove(source, attack_square, P, 0, 1, 0, 0, 0));
                            }
                        }
                        if (attack_square == enpassant_square) {
                            moves->push_back(setMove(source, enpassant_square, P, 0, 1, 0, 1, 0));
                        }
                        pop_bit(attacks, attack_square);
                    }
                    pop_bit(bitboard, source);
                }
            }
            if (i == N) {
                while (bitboard) {
                    source = get_ls1b_index(bitboard);
                    attacks = knight_moves[source] & ~occupancies[0];
                    while (attacks) {
                        target = get_ls1b_index(attacks);
                        if (!get_bit(occupancies[2], target)) {
                            moves->push_back(setMove(source, target, N, 0, 0, 0, 0, 0));
                        } else {
                            moves->push_back(setMove(source, target, N, 0, 1, 0, 0, 0));
                        }
                        pop_bit(attacks, target);
                    }
                    pop_bit(bitboard, source);
                }
            }
            if (i == B) {
                while (bitboard) {
                    source = get_ls1b_index(bitboard);
                    attacks = getBishopAttacks(source, occupancies[2]) & ~occupancies[0];
                    while (attacks) {
                        target = get_ls1b_index(attacks);
                        if (!get_bit(occupancies[2], target)) {
                            moves->push_back(setMove(source, target, B, 0, 0, 0, 0, 0));
                        } else {
                            moves->push_back(setMove(source, target, B, 0, 1, 0, 0, 0));
                        }
                        pop_bit(attacks, target);
                    }
                    pop_bit(bitboard, source);
                }
            }
            if (i == R) {
                while (bitboard) {
                    source = get_ls1b_index(bitboard);
                    attacks = getRookAttacks(source, occupancies[2]) & ~occupancies[0];
                    while (attacks) {
                        target = get_ls1b_index(attacks);
                        if (!get_bit(occupancies[2], target)) {
                            moves->push_back(setMove(source, target, R, 0, 0, 0, 0, 0));
                        } else {
                            moves->push_back(setMove(source, target, R, 0, 1, 0, 0, 0));
                        }
                        pop_bit(attacks, target);
                    }
                    pop_bit(bitboard, source);
                }
            }
            if (i == Q) {
                while (bitboard) {
                    source = get_ls1b_index(bitboard);
                    attacks = getQueenAttacks(source, occupancies[2]) & ~occupancies[0];
                    while (attacks) {
                        target = get_ls1b_index(attacks);
                        if (!get_bit(occupancies[2], target)) {
                            moves->push_back(setMove(source, target, Q, 0, 0, 0, 0, 0));
                        } else {
                            moves->push_back(setMove(source, target, Q, 0, 1, 0, 0, 0));
                        }
                        pop_bit(attacks, target);
                    }
                    pop_bit(bitboard, source);
                }
            }
            if (i == K) {
                if (castle & wk && !get_bit(occupancies[2], f1) && !get_bit(occupancies[2], g1) && !isAttacked(e1, 1) &&
                    !isAttacked(f1, 1) && !isAttacked(g1, 1)) {
                    moves->push_back(setMove(e1, g1, K, 0, 0, 0, 0, 1));
                }
                if (castle & wq && !get_bit(occupancies[2], b1) && !get_bit(occupancies[2], c1) &&
                    !get_bit(occupancies[2], d1) && !isAttacked(c1, 1) && !isAttacked(d1, 1) &&
                    !isAttacked(e1, 1)) {
                    moves->push_back(setMove(e1, c1, K, 0, 0, 0, 0, 1));
                }
                while (bitboard) {
                    source = get_ls1b_index(bitboard);
                    attacks = king_moves[source] & ~occupancies[0];
                    while (attacks) {
                        target = get_ls1b_index(attacks);
                        if (!isAttacked(target, 1)) {
                            if (!get_bit(occupancies[2], target)) {
                                moves->push_back(setMove(source, target, K, 0, 0, 0, 0, 0));
                            } else {
                                moves->push_back(setMove(source, target, K, 0, 1, 0, 0, 0));
                            }
                        }
                        pop_bit(attacks, target);
                    }
                    pop_bit(bitboard, source);
                }
            }
        } else {
            if (i == p) {
                while (bitboard) {
                    source = get_ls1b_index(bitboard);
                    target = source - 8;
                    if (target <= h8 && !get_bit(occupancies[2], target)) {
                        if (target < a2) {
                            moves->push_back(setMove(source, target, p, n, 1, 0, 0, 0));
                            moves->push_back(setMove(source, target, p, b, 1, 0, 0, 0));
                            moves->push_back(setMove(source, target, p, r, 1, 0, 0, 0));
                            moves->push_back(setMove(source, target, p, q, 1, 0, 0, 0));
                        } else {
                            moves->push_back(setMove(source, target, p, 0, 0, 0, 0, 0));
                        }
                        if (source < a8 && source >= a7 && !get_bit(occupancies[2], (target - 8))) {
                            moves->push_back(setMove(source, target + 8, p, 0, 0, 1, 0, 0));
                        }
                    }
                    attacks = pawn_attacks[source][1] & (occupancies[0] | 1ULL << enpassant_square);
                    while (attacks) {
                        int attack_square = get_ls1b_index(attacks);
                        if (get_bit(occupancies[0], attack_square)) {
                            if (attack_square < a2) {
                                moves->push_back(setMove(source, attack_square, p, n, 1, 1, 0, 0));
                                moves->push_back(setMove(source, attack_square, p, b, 1, 1, 0, 0));
                                moves->push_back(setMove(source, attack_square, p, r, 1, 1, 0, 0));
                                moves->push_back(setMove(source, attack_square, p, q, 1, 1, 0, 0));
                            } else {
                                moves->push_back(setMove(source, attack_square, p, 0, 1, 0, 0, 0));
                            }
                        }
                        if (attack_square == enpassant_square) {
                            moves->push_back(setMove(source, enpassant_square, p, 0, 1, 0, 1, 0));
                        }
                        pop_bit(attacks, attack_square);
                    }
                    pop_bit(bitboard, source);
                }
            }
            if (i == n) {
                while (bitboard) {
                    source = get_ls1b_index(bitboard);
                    attacks = knight_moves[source] & ~occupancies[1];
                    while (attacks) {
                        target = get_ls1b_index(attacks);
                        if (!get_bit(occupancies[2], target)) {
                            moves->push_back(setMove(source, target, n, 0, 0, 0, 0, 0));
                        } else {
                            moves->push_back(setMove(source, target, n, 0, 1, 0, 0, 0));
                        }
                        pop_bit(attacks, target);
                    }
                    pop_bit(bitboard, source);
                }
            }
            if (i == b) {
                while (bitboard) {
                    source = get_ls1b_index(bitboard);
                    attacks = getBishopAttacks(source, occupancies[2]) & ~occupancies[1];
                    while (attacks) {
                        target = get_ls1b_index(attacks);
                        if (!get_bit(occupancies[2], target)) {
                            moves->push_back(setMove(source, target, b, 0, 0, 0, 0, 0));
                        } else {
                            moves->push_back(setMove(source, target, b, 0, 1, 0, 0, 0));
                        }
                        pop_bit(attacks, target);
                    }
                    pop_bit(bitboard, source);
                }
            }
            if (i == r) {
                while (bitboard) {
                    source = get_ls1b_index(bitboard);
                    attacks = getRookAttacks(source, occupancies[2]) & ~occupancies[1];
                    while (attacks) {
                        target = get_ls1b_index(attacks);
                        if (!get_bit(occupancies[2], target)) {
                            moves->push_back(setMove(source, target, r, 0, 0, 0, 0, 0));
                        } else {
                            moves->push_back(setMove(source, target, r, 0, 1, 0, 0, 0));
                        }
                        pop_bit(attacks, target);
                    }
                    pop_bit(bitboard, source);
                }
            }
            if (i == q) {
                while (bitboard) {
                    source = get_ls1b_index(bitboard);
                    attacks = getQueenAttacks(source, occupancies[2]) & ~occupancies[1];
                    while (attacks) {
                        target = get_ls1b_index(attacks);
                        if (!get_bit(occupancies[2], target)) {
                            moves->push_back(setMove(source, target, q, 0, 0, 0, 0, 0));
                        } else {
                            moves->push_back(setMove(source, target, q, 0, 1, 0, 0, 0));
                        }
                        pop_bit(attacks, target);
                    }
                    pop_bit(bitboard, source);
                }
            }
            if (i == k) {
                if (castle & bk && !get_bit(occupancies[2], f8) && !get_bit(occupancies[2], g8) && !isAttacked(e8, 0) &&
                    !isAttacked(f8, 0) && !isAttacked(g8, 0)) {
                    moves->push_back(setMove(e8, g8, k, 0, 0, 0, 0, 1));
                }
                if (castle & bq && !get_bit(occupancies[2], b8) && !get_bit(occupancies[2], c8) &&
                    !get_bit(occupancies[2], d8) && !isAttacked(c8, 0) && !isAttacked(d8, 0) &&
                    !isAttacked(e8, 0)) {
                    moves->push_back(setMove(e8, c8, k, 0, 0, 0, 0, 1));
                }
                while (bitboard) {
                    source = get_ls1b_index(bitboard);
                    attacks = king_moves[source] & ~occupancies[1];
                    while (attacks) {
                        target = get_ls1b_index(attacks);
                        if (!isAttacked(target, 0)) {
                            if (!get_bit(occupancies[2], target)) {
                                moves->push_back(setMove(source, target, k, 0, 0, 0, 0, 0));
                            } else {
                                moves->push_back(setMove(source, target, k, 0, 1, 0, 0, 0));
                            }
                        }
                        pop_bit(attacks, target);
                    }
                    pop_bit(bitboard, source);
                }
            }
        }
    }
}

inline int getMoveSource(int move) {
    return move & 0x3f;
}

inline int getMoveTarget(int move) {
    return (move & 0xfc0) >> 6;
}

inline int getMovePiece(int move) {
    return (move & 0xf000) >> 12;
}

inline int getMovePromotedPiece(int move) {
    return (move & 0xf0000) >> 16;
}

inline int getMoveCaptureFlag(int move) {
    return (move & 0x100000) >> 20;
}

inline int getMoveDoublePushFlag(int move) {
    return (move & 0x200000) >> 21;
}

inline int getMoveEnPassantFlag(int move) {
    return (move & 0x400000) >> 22;
}

inline int getMoveCastlingFlag(int move) {
    return (move & 0x800000) >> 23;
}

static inline bool makeMove(int move, int only_captures){
    if(!only_captures){
        copy_board;
        int source = getMoveSource(move);
        int target = getMoveTarget(move);
        int piece = getMovePiece(move);
        int promoted = getMovePromotedPiece(move);
        int capture = getMoveCaptureFlag(move);
        int double_push = getMoveDoublePushFlag(move);
        int en_passant = getMoveEnPassantFlag(move);
        int castling = getMoveCastlingFlag(move);
        pop_bit(bitboards[piece], source);
        set_bit(bitboards[piece], target);
        pop_bit(occupancies[move_color], source);
        set_bit(occupancies[move_color], target);
        pop_bit(occupancies[2], source);
        set_bit(occupancies[2], target);
        if(capture){
            int start = move_color ? P : p;
            int end = move_color ? K : k;
            for(int i=start; i<=end; i++){
                if(get_bit(bitboards[i], target)){
                    pop_bit(bitboards[i], target);
                    pop_bit(occupancies[!move_color], target);
                    break;
                }
            }
        }
        if(promoted){
            pop_bit(bitboards[piece], target);
            set_bit(bitboards[promoted], target);
        }
        if(en_passant){
            pop_bit(bitboards[move_color ? P : p], (move_color ? enpassant_square + 8 : enpassant_square - 8));
        }
        enpassant_square = 64;
        if(double_push){
            enpassant_square = move_color ? target + 8 : target - 8;
        }
        if(castling){
            if(target == g1){
                pop_bit(bitboards[R], h1);
                set_bit(bitboards[R], f1);
                pop_bit(occupancies[1], h1);
                set_bit(occupancies[1], f1);
                pop_bit(occupancies[2], h1);
                set_bit(occupancies[2], f1);
            }
            else if(target == c1){
                pop_bit(bitboards[R], a1);
                set_bit(bitboards[R], d1);
                pop_bit(occupancies[1], a1);
                set_bit(occupancies[1], d1);
                pop_bit(occupancies[2], a1);
                set_bit(occupancies[2], d1);
            }
            else if(target == g8){
                pop_bit(bitboards[r], h8);
                set_bit(bitboards[R], f8);
                pop_bit(occupancies[0], h8);
                set_bit(occupancies[0], f8);
                pop_bit(occupancies[2], h8);
                set_bit(occupancies[2], f8);
            }
            else if(target == c8){
                pop_bit(bitboards[R], a8);
                set_bit(bitboards[R], d8);
                pop_bit(occupancies[0], a8);
                set_bit(occupancies[0], d8);
                pop_bit(occupancies[2], a8);
                set_bit(occupancies[2], d8);
            }
        }
        castle &= castling_rights[source];
        castle &= castling_rights[target];

        move_color ^= 1;
        if(isAttacked((move_color ? get_ls1b_index(bitboards[K]) : get_ls1b_index(bitboards[k])), move_color)){
            restore_board;
            return 0;
        }
        else{
            return 1;
        }
    }
    else{
        if(getMoveCaptureFlag(move)){
            makeMove(move, 1);
        }
    }
}

// PRINT EVERYTHING NECESSARY

void printBitboard(u64 board) {
    string ranks[8];
    for (int i = 7; i >= 0; i--) {
        string s;
        for (int j = 0; j < 8; j++) {
            if (board % 2) {
                s += "1 ";
            } else {
                s += ". ";
            }
            board /= 2;
        }
        ranks[i] = s;
    }
    for (int i = 0; i < 8; i++) {
        cout << 8 - i << ' ' << ranks[i] << endl;
    }
    cout << "  a b c d e f g h " << endl;
}

void printBoard(u64 board = occupancies[2]) {
    for(int i=P;i<=k;i++){
        board |= bitboards[i];
    }
    string ranks[8];
    u64 board_copy = board;
    for (int i = 7; i >= 0; i--) {
        string s;
        for (int j = 0; j < 8; j++) {
            if (board % 2) {
                s += pieceType(board_copy, 8 * (7 - i) + j);
                s += ' ';
            } else {
                s += ". ";
            }
            board /= 2;
        }
        ranks[i] = s;
    }
    for (int i = 0; i < 8; i++) {
        cout << 8 - i << ' ' << ranks[i] << endl;
    }
    cout << "  a b c d e f g h " << endl;
    cout << "SIDE: " << (move_color ? "black" : "white") << endl;
    cout << "EN PASSANT: " << (enpassant_square - 64 ? squareString(enpassant_square) : "no") << endl;
    cout << "CASTLE: " << ((castle & wk) ? 'K' : '-') <<
         ((castle & wq) ? 'Q' : '-') <<
         ((castle & bk) ? 'k' : '-') <<
         ((castle & bq) ? 'q' : '-') << endl;
}

void printAttacks(bool color) {
    u64 result = 0;
    for (int i = 0; i < 64; i++) {
        result |= u64(isAttacked(i, color)) << i;
    }
    cout << endl;
    printBitboard(result);
}

void printMove(int move){
    cout << "MOVE: " << squareString(getMoveSource(move)) << squareString(getMoveTarget(move))
         << (getMovePromotedPiece(move) ? name_string[getMovePromotedPiece(move)] : ' ') << endl;
    cout <<"PIECE: " << name_string[getMovePiece(move)] << " CAPTURE: " << getMoveCaptureFlag(move)
         << " DOUBLE PUSH: " << getMoveDoublePushFlag(move) << " EN PASSANT: " << getMoveEnPassantFlag(move)
         << " CASTLE: " << getMoveCastlingFlag(move) << endl;
    cout<<endl;
}

void printMoveList(vector<int> moves) {
    for (int i = 0; i < moves.size(); i++) {
        printMove(moves[i]);
    }
    cout<<"TOTAL MOVES: "<<moves.size()<<endl;
}

// PERFORMANCE TEST (PERFT)

int timeNow(){
    return GetTickCount();
}


// OTHER HELPFUL STUFF

void loadData() {
    freopen("bishop_masks.txt", "r", stdin);
    for (int i = 0; i < 64; i++) {
        cin >> bishop_masks[i];
    }
    freopen("rook_masks.txt", "r", stdin);
    for (int i = 0; i < 64; i++) {
        cin >> rook_masks[i];
    }
    freopen("king_moves.txt", "r", stdin);
    for (int i = 0; i < 64; i++) {
        cin >> king_moves[i];
    }
    freopen("knight_moves.txt", "r", stdin);
    for (int i = 0; i < 64; i++) {
        cin >> knight_moves[i];
    }
    freopen("magic_bishop.txt", "r", stdin);
    for (int i = 0; i < 64; i++) {
        cin >> magic_bishop[i];
    }

    freopen("pawn_attacks.txt", "r", stdin);
    for (int i = 0; i < 64; i++) {
        cin >> pawn_attacks[i][0];
    }
    for (int i = 0; i < 64; i++) {
        cin >> pawn_attacks[i][1];
    }

    freopen("magic_rook.txt", "r", stdin);
    for (int i = 0; i < 64; i++) {
        cin >> magic_rook[i];
    }


    /*
    freopen("rook_moves.txt", "r", stdin);
    for(int i=0;i<64;i++){
        for(int j=0;j<4096;j++){
            cin>>rook_moves[i][j];
        }
    }
    freopen("bishop_moves.txt", "r", stdin);
    for(int i=0;i<64;i++){
        for(int j=0;j<512;j++){
            cin>>bishop_moves[i][j];
        }
    }
    */

}

void setBoard(string fen) {
    for (int i = 0; i < 12; i++) {
        bitboards[i] = 0;
    }
    for (int i = 0; i < 3; i++) {
        occupancies[i] = 0;
    }
    int i = 0;
    int file;
    for (int rank = 7; rank >= 0; rank--) {
        file = 0;
        while (true) {
            if (fen[i] - '0' >= 1 && fen[i] - '0' <= 8) {
                file += fen[i] - '0';
                i++;
            } else if (fen[i] == '/' || fen[i] == ' ') {
                i++;
                break;
            } else {
                for (int j = 0; j < 12; j++) {
                    if (name_string[j] == fen[i]) {
                        bitboards[j] |= 1ULL << (8 * rank + file);
                        occupancies[j / 6] |= 1ULL << (8 * rank + file);
                        file++;
                        i++;
                        break;
                    }
                }
            }
        }
    }
    occupancies[2] = occupancies[1] | occupancies[0];
    move_color = fen[i] != 'w';
    i += 2;
    while (fen[i] != ' ') {
        if (fen[i] == 'Q') {
            i++;
            castle += wq;
        } else if (fen[i] == 'K') {
            i++;
            castle += wk;
        } else if (fen[i] == 'q') {
            i++;
            castle += bq;
        } else if (fen[i] == 'k') {
            i++;
            castle += bk;
        } else if (fen[i] == '-') {
            i++;
        }
    }
    i++;
    if (fen[i] == '-') {
        i++;
    } else {
        string enpass;
        enpass += fen[i];
        enpass += fen[i + 1];
        enpassant_square = stringSquare(enpass);
    }

    // 50 move rule in the future
}

int main() {
    int start = timeNow();
    freopen("output.txt", "w", stdout);
    loadData();
    setBoard("2k4r/2pr3p/p1R2p2/1p3Ppb/8/3P1NQP/P5P1/2q4K w - - 0 24");
    printBoard();
    vector<int> move_list;
    generateMoves(&move_list);
    for(int i=0; i<move_list.size(); i++){
        copy_board;
        if(!makeMove(move_list[i], 0)){
            continue;
        }
        printBoard();
        restore_board;
    }
    cout<<"Time: "<<timeNow() - start<<"ms"<<endl;
    return 0;
}
