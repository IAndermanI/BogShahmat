// DISCLAIMER
//
// Property of IAndermanI.
// Creator used Windows x86-64. This may work not as expected on other architectures.
// If this program lost your ELO, this is not the fault of mine :).
//
// DESCRIPTION
//
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
//
//

#include <bits/stdc++.h>
#include <windows.h>
#include <x86intrin.h>

using namespace std;
#define get_bit(bitboard, square) ((bitboard) & (1ULL << square))
#define pop_bit(bitboard, square) (get_bit(bitboard, square) ? ((bitboard) ^= (1ULL << square)) : 0ULL)
#define set_bit(bitboard, square) ((bitboard) |= (1ULL << square))
typedef uint64_t u64;



// CONSTANTS

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
int rotated_board[64] = {
        a8, b8, c8, d8, e8, f8, g8, h8,
        a7, b7, c7, d7, e7, f7, g7, h7,
        a6, b6, c6, d6, e6, f6, g6, h6,
        a5, b5, c5, d5, e5, f5, g5, h5,
        a4, b4, c4, d4, e4, f4, g4, h4,
        a3, b3, c3, d3, e3, f3, g3, h3,
        a2, b2, c2, d2, e2, f2, g2, h2,
        a1, b1, c1, d1, e1, f1, g1, h1
};
u64 not_a_file = 0xfefefefefefefefe;
u64 a_file = ~not_a_file;
u64 not_b_file = 0xfdfdfdfdfdfdfdfd;
u64 not_h_file = 0x7f7f7f7f7f7f7f7f;
u64 h_file = ~not_h_file;
u64 not_g_file = 0xbfbfbfbfbfbfbfbf;
u64 g_file = ~not_g_file;
u64 not_1_row = 0xffffffffffffff00;
u64 not_8_row = 0x00ffffffffffffff;


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
u64 hash_key = 0ULL;

// To count possible moves amount for sliding attack pieces
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

// Evaluation array
const int evaluation_array[6]{
    100, 270, 290, 430, 890, 10000
};

const int square_score[64]{
    1, 1, 1, 1, 1, 1, 1, 1,
    1, 2, 2, 2, 2, 2, 2, 1,
    1, 2, 3, 3, 3, 3, 2, 1,
    1, 2, 3, 4, 4, 3, 2, 1,
    1, 2, 3, 4, 4, 3, 2, 1,
    1, 2, 3, 3, 3, 3, 2, 1,
    1, 2, 2, 2, 2, 2, 2, 1,
    1, 1, 1, 1, 1, 1, 1, 1
};

// Increment to score of a pieces depends on a position (queen = rook + bishop)
const int pawn_eval[64] = {
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, -10, -10, 0, 0, 0,
        0, 0, 0, 5, 5, 0, 0, 0,
        5, 5, 10, 20, 20, 5, 5, 5,
        10, 10, 10, 20, 20, 10, 10, 10,
        20, 20, 20, 30, 30, 30, 20, 20,
        30, 30, 30, 40, 40, 30, 30, 30,
        90, 90, 90, 90, 90, 90, 90, 90
};

// knight positional score
const int knight_eval[64] = {
        -5, -10, 0, 0, 0, 0, -10, -5,
        -5, 0, 0, 0, 0, 0, 0, -5,
        -5, 5, 20, 10, 10, 20, 5, -5,
        -5, 10, 20, 30, 30, 20, 10, -5,
        -5, 10, 20, 30, 30, 20, 10, -5,
        -5, 5, 20, 20, 20, 20, 5, -5,
        -5, 0, 0, 10, 10, 0, 0, -5,
        -5, 0, 0, 0, 0, 0, 0, -5
};

// bishop positional score
const int bishop_eval[64] = {
        0, 0, -10, 0, 0, -10, 0, 0,
        0, 30, 0, 0, 0, 0, 30, 0,
        0, 10, 0, 0, 0, 0, 10, 0,
        0, 0, 10, 20, 20, 10, 0, 0,
        0, 0, 10, 20, 20, 10, 0, 0,
        0, 20, 0, 10, 10, 0, 20, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0
};

// rook positional score
const int rook_eval[64] = {
        0, 0, 0, 20, 20, 0, 0, 0,
        0, 0, 10, 20, 20, 10, 0, 0,
        0, 0, 10, 20, 20, 10, 0, 0,
        0, 0, 10, 20, 20, 10, 0, 0,
        0, 0, 10, 20, 20, 10, 0, 0,
        0, 0, 10, 20, 20, 10, 0, 0,
        50, 50, 50, 50, 50, 50, 50, 50,
        50, 50, 50, 50, 50, 50, 50, 50
};

// king positional score
const int king_eval[64] = {
        0, 0, 5, -15, -15, 0, 10, 0,
        0, 5, 5, -5, -5, 0, 5, 0,
        0, 0, 5, 10, 10, 5, 0, 0,
        0, 5, 10, 20, 20, 10, 5, 0,
        0, 5, 10, 20, 20, 10, 5, 0,
        0, 5, 5, 10, 10, 5, 5, 0,
        0, 0, 5, 5, 5, 5, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0
};

// Used to sort the move order
// Most valuable victim and less valuable attacker [attacker][victim]
// Example: mvv_lva[P][q] (55) > mvv_lva[N][q] (54) means that taking black queen with the pawn is better than with the knight

const int mvv_lva[12][12] = {
        15, 25, 35, 45, 55, 65, 15, 25, 35, 45, 55, 65,
        14, 24, 34, 44, 54, 64, 14, 24, 34, 44, 54, 64,
        13, 23, 33, 43, 53, 63, 13, 23, 33, 43, 53, 63,
        12, 22, 32, 42, 52, 62, 12, 22, 32, 42, 52, 62,
        11, 21, 31, 41, 51, 61, 11, 21, 31, 41, 51, 61,
        10, 20, 30, 40, 50, 60, 10, 20, 30, 40, 50, 60,
        15, 25, 35, 45, 55, 65, 15, 25, 35, 45, 55, 65,
        14, 24, 34, 44, 54, 64, 14, 24, 34, 44, 54, 64,
        13, 23, 33, 43, 53, 63, 13, 23, 33, 43, 53, 63,
        12, 22, 32, 42, 52, 62, 12, 22, 32, 42, 52, 62,
        11, 21, 31, 41, 51, 61, 11, 21, 31, 41, 51, 61,
        10, 20, 30, 40, 50, 60, 10, 20, 30, 40, 50, 60
};

// PRE-COUNTED MOVES
u64 pawn_attacks[64][2];
u64 knight_moves[64];
u64 king_moves[64];
u64 rook_masks[64];
u64 bishop_masks[64];
u64 bishop_moves[64][8192];
u64 rook_moves[64][16384];


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

// ZOBRIST HASHING

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

// PRINT BOARDS

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
    for (int i = P; i <= k; i++) {
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
    hash_key = generateHashKey();
    cout << "HASH KEY: " << hex << hash_key << dec << endl;
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
        int castle_copy = castle; \
        u64 hash_key_copy = hash_key;

#define restore_board memcpy(bitboards, bitboard_copy, 96); \
        memcpy(occupancies, occupancies_copy, 24); \
        move_color = move_color_copy; \
        enpassant_square = enpassant_copy; \
        castle = castle_copy; \
        hash_key = hash_key_copy; \


static inline void generateMoves(vector<int> &moves) {
    int source, target;
    u64 bitboard, attacks;
    for (int i = P; i <= k; i++) {
        bitboard = bitboards[i];
        if (!move_color) {
            if (i == P) {
                while (bitboard) {
                    source = getLSBIndex(bitboard);
                    target = source + 8;
                    if (target <= h8 && !get_bit(occupancies[2], target)) {
                        if (target >= a8) {
                            moves.push_back(setMove(source, target, P, N, 0, 0, 0, 0));
                            moves.push_back(setMove(source, target, P, B, 0, 0, 0, 0));
                            moves.push_back(setMove(source, target, P, R, 0, 0, 0, 0));
                            moves.push_back(setMove(source, target, P, Q, 0, 0, 0, 0));

                        } else {
                            moves.push_back(setMove(source, target, P, 0, 0, 0, 0, 0));
                        }
                        if (source < a3 && source >= a2 && !get_bit(occupancies[2], (target + 8))) {
                            moves.push_back(setMove(source, target + 8, P, 0, 0, 1, 0, 0));
                        }
                    }
                    attacks = pawn_attacks[source][0] & (occupancies[1] | (1ULL << enpassant_square));
                    while (attacks) {
                        int attack_square = getLSBIndex(attacks);
                        if (get_bit(occupancies[1], attack_square)) {
                            if (attack_square >= a8) {
                                moves.push_back(setMove(source, attack_square, P, N, 1, 0, 0, 0));
                                moves.push_back(setMove(source, attack_square, P, B, 1, 0, 0, 0));
                                moves.push_back(setMove(source, attack_square, P, R, 1, 0, 0, 0));
                                moves.push_back(setMove(source, attack_square, P, Q, 1, 0, 0, 0));
                            } else {
                                moves.push_back(setMove(source, attack_square, P, 0, 1, 0, 0, 0));
                            }
                        }
                        if (attack_square == enpassant_square) {
                            moves.push_back(setMove(source, enpassant_square, P, 0, 1, 0, 1, 0));
                        }
                        pop_bit(attacks, attack_square);
                    }
                    pop_bit(bitboard, source);
                }
            }
            if (i == N) {
                while (bitboard) {
                    source = getLSBIndex(bitboard);
                    attacks = knight_moves[source] & ~occupancies[0];
                    while (attacks) {
                        target = getLSBIndex(attacks);
                        if (!get_bit(occupancies[2], target)) {
                            moves.push_back(setMove(source, target, N, 0, 0, 0, 0, 0));
                        } else {
                            moves.push_back(setMove(source, target, N, 0, 1, 0, 0, 0));
                        }
                        pop_bit(attacks, target);
                    }
                    pop_bit(bitboard, source);
                }
            }
            if (i == B) {
                while (bitboard) {
                    source = getLSBIndex(bitboard);
                    attacks = getBishopAttacks(source, occupancies[2]) & ~occupancies[0];
                    while (attacks) {
                        target = getLSBIndex(attacks);
                        if (!get_bit(occupancies[2], target)) {
                            moves.push_back(setMove(source, target, B, 0, 0, 0, 0, 0));
                        } else {
                            moves.push_back(setMove(source, target, B, 0, 1, 0, 0, 0));
                        }
                        pop_bit(attacks, target);
                    }
                    pop_bit(bitboard, source);
                }
            }
            if (i == R) {
                while (bitboard) {
                    source = getLSBIndex(bitboard);
                    attacks = getRookAttacks(source, occupancies[2]) & ~occupancies[0];
                    while (attacks) {
                        target = getLSBIndex(attacks);
                        if (!get_bit(occupancies[2], target)) {
                            moves.push_back(setMove(source, target, R, 0, 0, 0, 0, 0));
                        } else {
                            moves.push_back(setMove(source, target, R, 0, 1, 0, 0, 0));
                        }
                        pop_bit(attacks, target);
                    }
                    pop_bit(bitboard, source);
                }
            }
            if (i == Q) {
                while (bitboard) {
                    source = getLSBIndex(bitboard);
                    attacks = getQueenAttacks(source, occupancies[2]) & ~occupancies[0];
                    while (attacks) {
                        target = getLSBIndex(attacks);
                        if (!get_bit(occupancies[2], target)) {
                            moves.push_back(setMove(source, target, Q, 0, 0, 0, 0, 0));
                        } else {
                            moves.push_back(setMove(source, target, Q, 0, 1, 0, 0, 0));
                        }
                        pop_bit(attacks, target);
                    }
                    pop_bit(bitboard, source);
                }
            }
            if (i == K) {
                if (castle & wk && !get_bit(occupancies[2], f1) && !get_bit(occupancies[2], g1) && !isAttacked(e1, 1) &&
                    !isAttacked(f1, 1) && !isAttacked(g1, 1)) {
                    moves.push_back(setMove(e1, g1, K, 0, 0, 0, 0, 1));
                }
                if (castle & wq && !get_bit(occupancies[2], b1) && !get_bit(occupancies[2], c1) &&
                    !get_bit(occupancies[2], d1) && !isAttacked(c1, 1) && !isAttacked(d1, 1) &&
                    !isAttacked(e1, 1)) {
                    moves.push_back(setMove(e1, c1, K, 0, 0, 0, 0, 1));
                }
                while (bitboard) {
                    source = getLSBIndex(bitboard);
                    attacks = king_moves[source] & ~occupancies[0];
                    while (attacks) {
                        target = getLSBIndex(attacks);
                        if (!isAttacked(target, 1)) {
                            if (!get_bit(occupancies[2], target)) {
                                moves.push_back(setMove(source, target, K, 0, 0, 0, 0, 0));
                            } else {
                                moves.push_back(setMove(source, target, K, 0, 1, 0, 0, 0));
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
                    source = getLSBIndex(bitboard);
                    target = source - 8;
                    if (target >= a1 && !get_bit(occupancies[2], target)) {
                        if (target < a2) {
                            moves.push_back(setMove(source, target, p, n, 1, 0, 0, 0));
                            moves.push_back(setMove(source, target, p, b, 1, 0, 0, 0));
                            moves.push_back(setMove(source, target, p, r, 1, 0, 0, 0));
                            moves.push_back(setMove(source, target, p, q, 1, 0, 0, 0));
                        } else {
                            moves.push_back(setMove(source, target, p, 0, 0, 0, 0, 0));
                        }
                        if (source < a8 && source >= a7 && !get_bit(occupancies[2], (target - 8))) {
                            moves.push_back(setMove(source, target - 8, p, 0, 0, 1, 0, 0));
                        }
                    }
                    attacks = pawn_attacks[source][1] & (occupancies[0] | (1ULL << enpassant_square));
                    while (attacks) {
                        int attack_square = getLSBIndex(attacks);
                        if (get_bit(occupancies[0], attack_square)) {
                            if (attack_square < a2) {
                                moves.push_back(setMove(source, attack_square, p, n, 1, 1, 0, 0));
                                moves.push_back(setMove(source, attack_square, p, b, 1, 1, 0, 0));
                                moves.push_back(setMove(source, attack_square, p, r, 1, 1, 0, 0));
                                moves.push_back(setMove(source, attack_square, p, q, 1, 1, 0, 0));
                            } else {
                                moves.push_back(setMove(source, attack_square, p, 0, 1, 0, 0, 0));
                            }
                        }
                        if (attack_square == enpassant_square) {
                            moves.push_back(setMove(source, enpassant_square, p, 0, 1, 0, 1, 0));
                        }
                        pop_bit(attacks, attack_square);
                    }
                    pop_bit(bitboard, source);
                }
            }
            if (i == n) {
                while (bitboard) {
                    source = getLSBIndex(bitboard);
                    attacks = knight_moves[source] & ~occupancies[1];
                    while (attacks) {
                        target = getLSBIndex(attacks);
                        if (!get_bit(occupancies[2], target)) {
                            moves.push_back(setMove(source, target, n, 0, 0, 0, 0, 0));
                        } else {
                            moves.push_back(setMove(source, target, n, 0, 1, 0, 0, 0));
                        }
                        pop_bit(attacks, target);
                    }
                    pop_bit(bitboard, source);
                }
            }
            if (i == b) {
                while (bitboard) {
                    source = getLSBIndex(bitboard);
                    attacks = getBishopAttacks(source, occupancies[2]) & ~occupancies[1];
                    while (attacks) {
                        target = getLSBIndex(attacks);
                        if (!get_bit(occupancies[2], target)) {
                            moves.push_back(setMove(source, target, b, 0, 0, 0, 0, 0));
                        } else {
                            moves.push_back(setMove(source, target, b, 0, 1, 0, 0, 0));
                        }
                        pop_bit(attacks, target);
                    }
                    pop_bit(bitboard, source);
                }
            }
            if (i == r) {
                while (bitboard) {
                    source = getLSBIndex(bitboard);
                    attacks = getRookAttacks(source, occupancies[2]) & ~occupancies[1];
                    while (attacks) {
                        target = getLSBIndex(attacks);
                        if (!get_bit(occupancies[2], target)) {
                            moves.push_back(setMove(source, target, r, 0, 0, 0, 0, 0));
                        } else {
                            moves.push_back(setMove(source, target, r, 0, 1, 0, 0, 0));
                        }
                        pop_bit(attacks, target);
                    }
                    pop_bit(bitboard, source);
                }
            }
            if (i == q) {
                while (bitboard) {
                    source = getLSBIndex(bitboard);
                    attacks = getQueenAttacks(source, occupancies[2]) & ~occupancies[1];
                    while (attacks) {
                        target = getLSBIndex(attacks);
                        if (!get_bit(occupancies[2], target)) {
                            moves.push_back(setMove(source, target, q, 0, 0, 0, 0, 0));
                        } else {
                            moves.push_back(setMove(source, target, q, 0, 1, 0, 0, 0));
                        }
                        pop_bit(attacks, target);
                    }
                    pop_bit(bitboard, source);
                }
            }
            if (i == k) {
                if (castle & bk && !get_bit(occupancies[2], f8) && !get_bit(occupancies[2], g8) && !isAttacked(e8, 0) &&
                    !isAttacked(f8, 0) && !isAttacked(g8, 0)) {
                    moves.push_back(setMove(e8, g8, k, 0, 0, 0, 0, 1));
                }
                if (castle & bq && !get_bit(occupancies[2], b8) && !get_bit(occupancies[2], c8) &&
                    !get_bit(occupancies[2], d8) && !isAttacked(c8, 0) && !isAttacked(d8, 0) && !isAttacked(e8, 0)) {
                    moves.push_back(setMove(e8, c8, k, 0, 0, 0, 0, 1));
                }
                while (bitboard) {
                    source = getLSBIndex(bitboard);
                    attacks = king_moves[source] & ~occupancies[1];
                    while (attacks) {
                        target = getLSBIndex(attacks);
                        if (!isAttacked(target, 0)) {
                            if (!get_bit(occupancies[2], target)) {
                                moves.push_back(setMove(source, target, k, 0, 0, 0, 0, 0));
                            } else {
                                moves.push_back(setMove(source, target, k, 0, 1, 0, 0, 0));
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

static inline bool makeMove(int move, int only_captures) {
    if (!only_captures) {
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

        hash_key ^= piece_keys[piece][source];
        hash_key ^= piece_keys[piece][target];

        if (capture) {
            int start = move_color ? P : p;
            int end = move_color ? K : k;
            for (int i = start; i <= end; i++) {
                if (get_bit(bitboards[i], target)) {
                    pop_bit(bitboards[i], target);
                    pop_bit(occupancies[!move_color], target);
                    hash_key ^= piece_keys[i][target];
                    break;
                }
            }
        }
        if (promoted) {
            pop_bit(bitboards[piece], target);
            set_bit(bitboards[promoted], target);
            hash_key ^= piece_keys[piece][target];
            hash_key ^= piece_keys[promoted][target];
        }
        if (en_passant) {
            pop_bit(bitboards[move_color ? P : p], (move_color ? enpassant_square + 8 : enpassant_square - 8));
            pop_bit(occupancies[!move_color], (move_color ? enpassant_square + 8 : enpassant_square - 8));
            pop_bit(occupancies[2], (move_color ? enpassant_square + 8 : enpassant_square - 8));
            hash_key ^= piece_keys[move_color ? P : p][move_color ? enpassant_square + 8 : enpassant_square - 8];
        }
        if (enpassant_square != empty_square) {
            hash_key ^= enpassant_keys[enpassant_square];
        }

        enpassant_square = empty_square;

        if (double_push) {
            enpassant_square = move_color ? target + 8 : target - 8;
            hash_key ^= enpassant_keys[enpassant_square];
        }
        if (castling) {
            if (target == g1) {
                pop_bit(bitboards[R], h1);
                set_bit(bitboards[R], f1);
                pop_bit(occupancies[0], h1);
                set_bit(occupancies[0], f1);
                pop_bit(occupancies[2], h1);
                set_bit(occupancies[2], f1);
                hash_key ^= piece_keys[R][h1];
                hash_key ^= piece_keys[R][f1];
            } else if (target == c1) {
                pop_bit(bitboards[R], a1);
                set_bit(bitboards[R], d1);
                pop_bit(occupancies[0], a1);
                set_bit(occupancies[0], d1);
                pop_bit(occupancies[2], a1);
                set_bit(occupancies[2], d1);
                hash_key ^= piece_keys[R][a1];
                hash_key ^= piece_keys[R][d1];
            } else if (target == g8) {
                pop_bit(bitboards[r], h8);
                set_bit(bitboards[r], f8);
                pop_bit(occupancies[1], h8);
                set_bit(occupancies[1], f8);
                pop_bit(occupancies[2], h8);
                set_bit(occupancies[2], f8);
                hash_key ^= piece_keys[r][h8];
                hash_key ^= piece_keys[r][f8];
            } else if (target == c8) {
                pop_bit(bitboards[r], a8);
                set_bit(bitboards[r], d8);
                pop_bit(occupancies[1], a8);
                set_bit(occupancies[1], d8);
                pop_bit(occupancies[2], a8);
                set_bit(occupancies[2], d8);
                hash_key ^= piece_keys[r][a8];
                hash_key ^= piece_keys[r][d8];
            }
        }

        hash_key ^= castle_keys[castle];
        castle &= castling_rights[source];
        castle &= castling_rights[target];
        hash_key ^= castle_keys[castle];

        move_color = !move_color;
        hash_key ^= color_key;

        if (isAttacked((move_color ? getLSBIndex(bitboards[K]) : getLSBIndex(bitboards[k])), move_color)) {
            restore_board;
            return 0;
        } else {
            return 1;
        }

    } else {
        if (getMoveCaptureFlag(move)) {
            makeMove(move, 0);
        } else {
            return 0;
        }
    }
}

// PRINT EVERYTHING NECESSARY (EXCEPT BOARDS)

void printAttacks(bool color) {
    u64 result = 0;
    for (int i = 0; i < 64; i++) {
        result |= u64(isAttacked(i, color)) << i;
    }
    cout << endl;
    printBitboard(result);
}

void printFullMove(int move) {
    cout << "MOVE: " << squareString(getMoveSource(move)) << squareString(getMoveTarget(move))
         << (getMovePromotedPiece(move) ? name_string[getMovePromotedPiece(move)] : ' ') << endl;
    cout << "PIECE: " << name_string[getMovePiece(move)] << " CAPTURE: " << getMoveCaptureFlag(move)
         << " DOUBLE PUSH: " << getMoveDoublePushFlag(move) << " EN PASSANT: " << getMoveEnPassantFlag(move)
         << " CASTLE: " << getMoveCastlingFlag(move) << endl;
    cout << endl;
}

void printMove(int move) {
    cout << squareString(getMoveSource(move)) << squareString(getMoveTarget(move))
         << (getMovePromotedPiece(move) ? name_string[getMovePromotedPiece(move)] : ' ');
}

void printMoveList(vector<int> moves) {
    for (int i = 0; i < moves.size(); i++) {
        printFullMove(moves[i]);
    }
    cout << "TOTAL MOVES: " << moves.size() << endl;
}

// PERFORMANCE TEST (PERFT)

int timeNow() {
    return GetTickCount();
}

static inline u64 perft(int depth) {
    if (depth == 0) {
        return 1ULL;
    }
    vector<int> move_list;
    u64 ans = 0;
    generateMoves(move_list);
    for (int i = 0; i < move_list.size(); i++) {
        copy_board;
        if (!makeMove(move_list[i], 0)) {
            continue;
        }
        ans += perft(depth - 1);
        restore_board;
    }
    return ans;
}

static inline void perft_moves(int depth) {
    if (depth == 0) {
        return;
    }
    vector<int> move_list;
    u64 ans_total = 0;
    generateMoves(move_list);
    for (int i = 0; i < move_list.size(); i++) {
        copy_board;
        if (!makeMove(move_list[i], 0)) {
            continue;
        }
        u64 ans = perft(depth - 1);
        ans_total += ans;
        restore_board;
        cout << squareString(getMoveSource(move_list[i])) << squareString(getMoveTarget(move_list[i]))
             << (getMovePromotedPiece(move_list[i]) ? name_string[getMovePromotedPiece(move_list[i])] : '-')
             << ": " << ans << endl;
    }
    cout << "Nodes: " << ans_total << endl;
}


// EVALUATION

static inline int distanceFromKing(int king_square, int piece_square){
    return max(abs((king_square >> 3) - (piece_square >> 3)), abs((king_square % 8) - (piece_square % 8)));
}
static inline u64 kingArea(int square){
    if(square % 8 == 0){
        square++;
    }
    if(square % 8 == 7){
        square--;
    }
    if(square > a7){
        square -= 8;
    }
    if(square < a2){
        square += 8;
    }
    return king_moves[square];
}

u64 rank_masks[64];
u64 file_masks[64];
u64 isolated_masks[64];
u64 passed_pawns_masks[2][64];

int double_pawns_penalty = 20;
int isolated_pawn_score = -15;
int passed_pawn_score[8] = {0, 5, 10, 20, 35, 60, 100, 200};

void fillRankMasks() {
    u64 rank = 255ULL;
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            rank_masks[8 * i + j] = rank;
        }
        rank <<= 8;
    }
}

void fillFileMasks() {
    u64 file = a_file;
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            file_masks[8 * j + i] = file;
        }
        file <<= 9;
        file += 1 << (i + 1);
    }
}

void fillIsolatedMasks() {
    for (int i = 0; i < 64; i++) {
        if (i % 8 == 0) {
            isolated_masks[i] = file_masks[i + 1];
        } else if (i % 8 == 7) {
            isolated_masks[i] = file_masks[i - 1];
        } else {
            isolated_masks[i] = file_masks[i + 1] | file_masks[i - 1];
        }
    }
}

void fillPassedPawnsMasks() {
    for (int i = 0; i < 64; i++) {
        passed_pawns_masks[0][i] = file_masks[i] | isolated_masks[i];
        passed_pawns_masks[1][i] = file_masks[i] | isolated_masks[i];
    }
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            for (int k = 0; k <= i; k++) {
                passed_pawns_masks[0][8 * i + j] &= ~rank_masks[8 * k + j];
                passed_pawns_masks[1][8 * (7 - i) + j] &= ~rank_masks[8 * (7 - k) + j];
            }
        }
    }
}

void fillEvaluationMasks() {
    fillRankMasks();
    fillFileMasks();
    fillIsolatedMasks();
    fillPassedPawnsMasks();
}

const int outpost_score = 16;
const int defense_score = 4;
const int fianchetto_bonus = 25;
const int pawn_attack_penalty = 4;
int attacking_pieces_count[2];
int attacks_value[6] = {0, 20, 20, 40, 80, 0};
int value_of_attack[2];
int attack_weight[7] = {0, 50, 75, 88, 94, 97, 99};
const int stacked_rooks_score = 25;
const int xray_king_score = 25;
const int xray_queen_score = 20;
const int seventh_rank_score = 45;
const int open_rook = 30;
const int no_queen_score = 15;
const int open_file_penalty = 25;

static inline int pawnEvaluation(bool color){
    u64 bitboard_copy = bitboards[color ? p : P];
    int score = 0;
    while(bitboard_copy) {
        int square = getLSBIndex(bitboard_copy);
        pop_bit(bitboard_copy, square);
        score += evaluation_array[P];
        int double_pawns = countBits(bitboards[color ? p : P] & file_masks[square] & passed_pawns_masks[color][square]);
        score -= min(90, double_pawns * double_pawns_penalty);
        if ((bitboards[color ? p : P] & isolated_masks[square]) == 0) {
            score += isolated_pawn_score;
        }

        if ((bitboards[color ? P : p] & passed_pawns_masks[color][square]) == 0) {
            score += passed_pawn_score[square >> 3];
        }
        score -= distanceFromKing(getLSBIndex(bitboards[color ? k : K]), square);
    }
    return color ? -score : score;
}

static inline int knightEvaluation(bool color){
    u64 bitboard_copy = bitboards[color ? n : N];
    int score = 0;
    while(bitboard_copy){
        int square = getLSBIndex(bitboard_copy);
        pop_bit(bitboard_copy, square);
        score += evaluation_array[N];
        u64 target_squares = knight_moves[square];
        int king_area_attacks = countBits(kingArea(getLSBIndex(bitboards[color ? K : k])) & target_squares);
        if(king_area_attacks){
            attacking_pieces_count[color]++;
            value_of_attack[color] += king_area_attacks * attacks_value[N];
        }
        while(target_squares){
            int target_square = getLSBIndex(target_squares);
            pop_bit(target_squares, target_square);
            score += square_score[target_square];
            if((1ULL << target_square) & occupancies[color] & ~bitboards[color ? k : K]){
                score += defense_score;
            }
        }
        score -= distanceFromKing(getLSBIndex(bitboards[color ? k : K]), square);
        if ((pawn_attacks[!color][square] & bitboards[color ? p : P]) && !(pawn_attacks[color][square] & bitboards[color ? P : p]) && (color == (square <= h4))){
            score += outpost_score;
        }
    }
    return color ? -score : score;
}

static inline int bishopEvaluation(bool color){
    u64 bitboard_copy = bitboards[color ? b : B];
    int score = 0;
    while(bitboard_copy){
        int square = getLSBIndex(bitboard_copy);
        pop_bit(bitboard_copy, square);
        score += evaluation_array[B];
        u64 target_squares = getBishopAttacks(square, occupancies[2]);
        int king_area_attacks = countBits(kingArea(getLSBIndex(bitboards[color ? K : k])) & target_squares);
        if(king_area_attacks){
            attacking_pieces_count[color]++;
            value_of_attack[color] += king_area_attacks * attacks_value[B];
        }
        while(target_squares){
            int target_square = getLSBIndex(target_squares);
            pop_bit(target_squares, target_square);
            score += square_score[target_square];
            if((1ULL << target_square) & occupancies[color] & ~bitboards[color ? k : K]){
                score += defense_score;
            }
            int pawn_chain = countBits(bishop_masks[target_square] & bitboards[color ? P : p]);
            if(pawn_chain > 1){
                score -= pawn_attack_penalty * pawn_chain;
            }
        }
        score -= distanceFromKing(getLSBIndex(bitboards[color ? k : K]), square);
        if ((pawn_attacks[!color][square] & bitboards[color ? p : P]) && !(pawn_attacks[color][square] & bitboards[color ? P : p]) && (color == (square <= h4))){
            score += outpost_score;
        }
        if((color ? square == g7 && getLSBIndex(bitboards[k]) == g8 : square == g2 && getLSBIndex(bitboards[k]) == g1)){
            score += fianchetto_bonus;
        }
    }
    return color ? -score : score;
}

static inline int rookEvaluation(bool color){
    u64 bitboard_copy = bitboards[color ? r : R];
    int score = 0;
    while(bitboard_copy){
        int square = getLSBIndex(bitboard_copy);
        pop_bit(bitboard_copy, square);
        score += evaluation_array[R];
        u64 target_squares = getRookAttacks(square, occupancies[2]);
        int king_area_attacks = countBits(kingArea(getLSBIndex(bitboards[color ? K : k])) & target_squares);
        if(king_area_attacks){
            attacking_pieces_count[color]++;
            value_of_attack[color] += king_area_attacks * attacks_value[R];
        }
        if(target_squares & bitboards[color ? r : R]){
            score += stacked_rooks_score;
        }
        if(rook_masks[square] & bitboards[color ? K : k]){
            score += xray_king_score;
        }

        if(rook_masks[square] & bitboards[color ? Q : q]){
            score += xray_queen_score;
        }

        if( (color ? (square >= a2 && square <= h2) : (square >= a7 && square <= h7)) && ((color ? bitboards[P] &
                getRookAttacks(square, occupancies[2]) : bitboards[p] & getRookAttacks(square, occupancies[2])) ||
                (color ? getLSBIndex(bitboards[K]) <= h1 : getLSBIndex(bitboards[k] >= a8))) ){
            score += seventh_rank_score;
        }
        int bits = 0;
        while(target_squares){
            bits++;
            int target_square = getLSBIndex(target_squares);
            pop_bit(target_squares, target_square);
            score += square_score[target_square];
            if((1ULL << target_square) & occupancies[color] & ~bitboards[color ? k : K]){
                score += defense_score;
            }
        }
        if(bits > 8){
            score += open_rook;
        }

        if(bits < 3){
            score -= open_rook;
        }

    }
    return color ? -score : score;
}

static inline int queenEvaluation(bool color){
    u64 bitboard_copy = bitboards[color ? q : Q];
    int score = 0;
    while(bitboard_copy){
        int square = getLSBIndex(bitboard_copy);
        pop_bit(bitboard_copy, square);
        score += evaluation_array[Q];
        u64 target_squares = getQueenAttacks(square, occupancies[2]);
        int king_area_attacks = countBits(kingArea(getLSBIndex(bitboards[color ? K : k])) & target_squares);
        if(king_area_attacks){
            attacking_pieces_count[color]++;
            value_of_attack[color] += king_area_attacks * attacks_value[Q];
        }
        while(target_squares){
            int target_square = getLSBIndex(target_squares);
            pop_bit(target_squares, target_square);
            score += square_score[target_square];
            if((1ULL << target_square) & occupancies[color] & ~bitboards[color ? k : K]){
                score += defense_score;
            }
        }
    }
    return color ? -score : score;
}

static inline int kingEvaluation(bool color){
    int score = evaluation_array[K];
    int square = getLSBIndex(bitboards[color ? k : K]);
    if(bitboards[color ? Q : q] == 0){
        score += no_queen_score;
    }
    if(countBits(getRookAttacks(square, occupancies[2]) & rank_masks[square]) > 4){
        score -= open_file_penalty;
    }
    return color ? -score : score;
}

static inline int calculateAttackingKings(bool color){
    int score = value_of_attack[color] * attack_weight[min(6, attacking_pieces_count[color])] / 100;
    attacking_pieces_count[0] = attacking_pieces_count[1] = 0;
    value_of_attack[0] = value_of_attack[1] = 0;
    return color ? -score : score;
}

static inline int evaluate(){
    int score = 0;
    score += pawnEvaluation(0) + pawnEvaluation(1);
    score += knightEvaluation(0) + knightEvaluation(1);
    score += bishopEvaluation(0) + bishopEvaluation(1);
    score += rookEvaluation(0) + rookEvaluation(1);
    score += queenEvaluation(0) + queenEvaluation(1);
    score += kingEvaluation(0) + kingEvaluation(1);
    score += calculateAttackingKings(0) + calculateAttackingKings(1);
    return move_color ? -score : score;
}

/*static inline int evaluate() {
    int score = 0;
    for (int i = P; i <= k; i++) {
        score += countBits(bitboards[i]) * evaluation_array[i];
        u64 bitboard_copy = bitboards[i];
        while (bitboard_copy) {
            int square = getLSBIndex(bitboard_copy);
            pop_bit(bitboard_copy, square);
            switch (i) {
                case P: {
                    score += pawn_eval[square];
                    int double_pawns = countBits(bitboards[P] & file_masks[square] & passed_pawns_masks[0][square]);
                    score += max(-90, double_pawns * double_pawns_score);
                    if ((bitboards[P] & isolated_masks[square]) == 0) {
                        score += isolated_pawn_score;
                    }
                    if((bitboards[p] & passed_pawns_masks[0][square]) == 0){
                        score += passed_pawn_score[square >> 3];
                    }
                        break;
                }
                case N:
                    score += knight_eval[square];
                    break;
                case B:
                    score += bishop_eval[square];
                    break;
                case R:
                    score += rook_eval[square];
                    break;
                case Q:
                    score += bishop_eval[square] + rook_eval[square];
                    break;
                case K:
                    score += king_eval[square];
                    break;
                case p: {
                    score -= pawn_eval[rotated_board[square]];
                    int double_pawns = countBits(bitboards[p] & file_masks[square] & passed_pawns_masks[1][square]);
                    score -= max(-90, double_pawns * double_pawns_score);
                    if ((bitboards[p] & isolated_masks[square]) == 0) {
                        score -= isolated_pawn_score;
                    }
                    if((bitboards[P] & passed_pawns_masks[1][square]) == 0){
                        score -= passed_pawn_score[7 - (square >> 3)];
                    }
                    break;
                }
                case n:
                    score -= knight_eval[rotated_board[square]];
                    break;
                case b:
                    score -= bishop_eval[rotated_board[square]];
                    break;
                case r:
                    score -= rook_eval[rotated_board[square]];
                    break;
                case q:
                    score -= bishop_eval[rotated_board[square]] + rook_eval[rotated_board[square]];
                    break;
                case k:
                    score -= king_eval[rotated_board[square]];
                    break;
                default:
                    score += 0;
            }
        }
    }
    return move_color ? -score : score;
}
*/

// TRANSPOSITION TABLE

struct transTable {
    u64 hash_key;
    int depth;
    int flag;
    int score;
};

const int TT_size = 0x10000;
const int no_entry = 100000;

int MINIMUM_SCORE = -100000;
int MAXIMUM_SCORE = 100000;
int best_move, ply;

transTable transposition_table[TT_size];

void clearTT() {
    for (int i = 0; i < TT_size; i++) {
        transposition_table[i].hash_key = 0;
        transposition_table[i].depth = 0;
        transposition_table[i].flag = 0;
        transposition_table[i].score = 0;
    }
}

static inline int readEntry(int alpha, int beta, int depth) {
    transTable *entry = &transposition_table[hash_key % TT_size];
    if (entry->hash_key == hash_key) {
        if (entry->depth >= depth) {
            int score = entry->score;
            if (score < MINIMUM_SCORE + 2000) {
                score += ply;
            }
            if (score > MAXIMUM_SCORE - 2000) {
                score -= ply;
            }
            if (entry->flag == 0) {
                return score;
            }
            if (entry->flag == 1 && entry->score <= alpha) {
                return alpha;
            }
            if (entry->flag == 2 && entry->score >= beta) {
                return beta;
            }
        }
    }
    return no_entry;
}

static inline void writeEntry(int score, int depth, int flag) {
    transTable *entry = &transposition_table[hash_key % TT_size];
    if (score < MINIMUM_SCORE + 2000) {
        score -= ply;
    }
    if (score > MAXIMUM_SCORE - 2000) {
        score += ply;
    }
    entry->hash_key = hash_key;
    entry->depth = depth;
    entry->flag = flag;
    entry->score = score;
}

// SEARCHING BEST MOVES

const int MAX_PLY = 64;
int killers[2][MAX_PLY];
int history[12][64];
int follow_pv, score_pv;
int pv_length[MAX_PLY];
int pv_table[MAX_PLY][MAX_PLY];
vector<u64> history_table;

static inline int is_repetitions() {
    int repetitions_count = 0;
    for (int i = 0; i < history_table.size(); i++) {
        if (history_table[i] == hash_key) {
            repetitions_count++;
        }
    }
    return repetitions_count;
}

static inline void enablePV(vector<int> &move_list) {
    follow_pv = 0;
    for (int i = 0; i < move_list.size(); i++) {
        if (pv_table[0][ply] == move_list[i]) {
            score_pv = 1;
            follow_pv = 1;
        }
    }
}

static inline int moveScore(int move) {
    if (score_pv) {
        if (pv_table[0][ply] == move) {
            score_pv = 0;
            return 200;
        }
    }
    if (getMoveCaptureFlag(move)) {
        int source_piece = getMovePiece(move), captured_piece = P;
        int start = move_color ? P : p, end = move_color ? K : k;
        int target = getMoveTarget(move);
        for (int i = start; i <= end; i++) {
            if (get_bit(bitboards[i], target)) {
                captured_piece = i;
                break;
            }
        }
        return mvv_lva[source_piece][captured_piece] + 100;
    } else {
        if (killers[0][ply] == move) {
            return 90;
        } else if (killers[1][ply] == move) {
            return 80;
        } else {
            return history[getMovePiece(move)][getMoveTarget(move)];
        }

    }
    return 0;
}

static inline void sortMoves(vector<int> &move_list) {
    vector<pair<int, int>> captures, quite;
    for (int i = 0; i < move_list.size(); i++) {
        pair<int, int> pr;
        pr.first = moveScore(move_list[i]);
        pr.second = move_list[i];
        if (pr.first != 0) {
            captures.push_back(pr);
        } else {
            quite.push_back(pr);
        }
    }
    sort(captures.begin(), captures.end());
    reverse(captures.begin(), captures.end());
    for (int i = 0; i < captures.size(); i++) {
        move_list[i] = captures[i].second;
    }
    for (int i = captures.size(); i < move_list.size(); i++) {
        move_list[i] = quite[i - captures.size()].second;
    }
}

int nodes;

static inline int quiescence(int alpha, int beta) {
    nodes++;
    int evaluation = evaluate();
    if (ply > MAX_PLY - 1) {
        return evaluation;
    }

    if (evaluation > alpha) {
        alpha = evaluation;
        if (evaluation >= beta) {
            return beta;
        }
    }
    vector<int> move_list;
    generateMoves(move_list);
    sortMoves(move_list);

    for (int i = 0; i < move_list.size(); i++) {
        copy_board;
        ply++;
        history_table.push_back(hash_key);
        if (makeMove(move_list[i], 1) == 0) {
            ply--;
            history_table.pop_back();
            continue;
        }
        int score = -quiescence(-beta, -alpha);
        restore_board;
        ply--;
        history_table.pop_back();
        if (score >= beta) {
            return beta;
        }
        if (score > alpha) {
            alpha = score;
        }
    }
    return alpha;
}

static inline int negamax(int alpha, int beta, int depth) {
    int score = readEntry(alpha, beta, depth);
    int flag = 1;
    bool is_PV_node = beta - alpha > 1;
    if (ply && is_repetitions() >= 2) {
        return 0;
    }
    if (ply && score != no_entry && !is_PV_node) {
        return score;
    }
    int found_pv = 0;
    pv_length[ply] = ply;
    int current_best_move, alpha_copy = alpha;
    if (depth <= 0) {
        return quiescence(alpha, beta);
    }
    if (ply > MAX_PLY - 1) {
        return evaluate();
    }
    bool in_check = isAttacked(move_color ? getLSBIndex(bitboards[k]) : getLSBIndex(bitboards[K]), !move_color);
    nodes++;
    if (depth >= 3 && !in_check && ply) {
        copy_board;
        ply++;
        history_table.push_back(hash_key);
        move_color = !move_color;
        hash_key ^= color_key;
        if (enpassant_square != empty_square) {
            hash_key ^= enpassant_keys[enpassant_square];
        }
        enpassant_square = empty_square;
        score = -negamax(-beta, -beta + 1, depth - 3);
        restore_board
        ply--;
        history_table.pop_back();
        if (score >= beta) {
            return beta;
        }
    }
    vector<int> move_list;
    generateMoves(move_list);
    if (follow_pv) {
        enablePV(move_list);
    }
    int moves_searched = 0;
    sortMoves(move_list);
    int legal_moves = 0;

    if (in_check) {
        depth++;
    }
    for (int i = 0; i < move_list.size(); i++) {
        copy_board;
        ply++;
        history_table.push_back(hash_key);
        if (makeMove(move_list[i], 0) == 0) {
            ply--;
            history_table.pop_back();
            continue;
        }
        legal_moves++;
        int score;
        if (found_pv) {
            score = -negamax(-alpha - 1, -alpha, depth - 1);
            if (score > alpha && score < beta) {
                score = -negamax(-beta, -alpha, depth - 1);
            }
        } else {
            score = -negamax(-beta, -alpha, depth - 1);
        }
        ply--;
        history_table.pop_back();
        restore_board;

        if (score > alpha) {
            flag = 0;
            if (!getMoveCaptureFlag(move_list[i])) {
                history[getMovePiece(move_list[i])][getMoveTarget(move_list[i])] += depth;
            }
            alpha = score;
            found_pv = 1;
            pv_table[ply][ply] = move_list[i];
            for (int j = ply + 1; j < pv_length[ply + 1]; j++) {
                pv_table[ply][j] = pv_table[ply + 1][j];
            }
            pv_length[ply] = pv_length[ply + 1];
            if (ply == 0) {
                current_best_move = move_list[i];
            }

            if (score >= beta) {
                writeEntry(beta, depth, 2);
                if (!getMoveCaptureFlag(move_list[i])) {
                    killers[1][ply] = killers[0][ply];
                    killers[0][ply] = move_list[i];
                }
                return beta;
            }
        }
    }
    if (legal_moves == 0) {
        if (in_check) {
            return MINIMUM_SCORE + 1000 + ply;
        } else {
            return 0;
        }
    }
    if (alpha_copy != alpha) {
        best_move = current_best_move;
    }
    writeEntry(alpha, depth, flag);
    return alpha;
}

int searchBest(int depth) {
    follow_pv = 0;
    score_pv = 0;
    memset(killers, 0, sizeof(killers));
    memset(history, 0, sizeof(history));
    memset(pv_length, 0, sizeof(pv_length));
    memset(pv_table, 0, sizeof(pv_table));
    int alpha = MINIMUM_SCORE, beta = MAXIMUM_SCORE;
    for (int i = 1; i <= depth; i++) {
        follow_pv = 1;
        int score = negamax(alpha, beta, i);
        if ((score <= alpha) || (score >= beta)) {
            alpha = MINIMUM_SCORE;
            beta = MAXIMUM_SCORE;
            continue;
        }
        alpha = score - 200;
        beta = score + 200;
        cout << "DEPTH: " << i << endl;
        cout << "BEST MOVE: ";
        printMove(best_move);
        cout << endl;
        cout << "BEST LINE: ";
        for (int i = 0; i < pv_length[0]; i++) {
            printMove(pv_table[0][i]);
        }
        cout << nodes << endl;
        cout << endl;
        if (i == depth) {
            return score;
        }
    }
}

// INTERFACE (MAYBE TO BE DONE LATER)

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
    clearTT();
    // 50 move rule in the future
}

int parse_move(string move_string) {
    vector<int> move_list;
    generateMoves(move_list);
    int source = (move_string[0] - 'a') + 8 * (move_string[1] - '1');
    int target = (move_string[2] - 'a') + 8 * (move_string[3] - '1');
    int promoted = 0;
    for (int i = 0; i < move_list.size(); i++) {
        if (getMoveSource(move_list[i]) == source && getMoveTarget(move_list[i]) == target) {
            if (move_string.size() == 4) {
                return move_list[i];
            }
            if (getMovePromotedPiece(move_list[i]) == n ||
                getMovePromotedPiece(move_list[i]) == N && move_string[4] == 'n'
                || getMovePromotedPiece(move_list[i]) == b ||
                getMovePromotedPiece(move_list[i]) == B && move_string[4] == 'b'
                || getMovePromotedPiece(move_list[i]) == r ||
                getMovePromotedPiece(move_list[i]) == R && move_string[4] == 'r'
                || getMovePromotedPiece(move_list[i]) == q ||
                getMovePromotedPiece(move_list[i]) == Q && move_string[4] == 'q')
                return move_list[i];
        }
    }
    return 0;
}

// INIT EVERYTHING

void fillEverything() {
    fillAllMoves();
    fillHashKeys();
    fillEvaluationMasks();
}

/*setBoard("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    while(1){
        string move_string;
        int depth;
        cin>>move_string>>depth;
        copy_board
        int move = parse_move(move_string);
        if(move == 0){
            restore_board;
            continue;
        }
        makeMove(move, 0);
        searchBest(depth);
        makeMove(best_move, 0);
    }
     */

// MAIN
int main() {
    fillEverything();
    //rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1
    //r3k2r/pppppppp/8/8/8/8/PPPPPPPP/R3K2R w KQkq - 0 1
    setBoard("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    printBoard();
    cout << "INPUT COLOR:\n";
    string color;
    cin >> color;
    cout << "OK. LETS START!\n";
    if (color == "white") {
        int depth = 6;
        searchBest(depth);
        makeMove(best_move, 0);
    }
    while (1) {
        string move_string;
        int depth;
        cin >> move_string >> depth;
        copy_board
        int move = parse_move(move_string);
        if (move == 0) {
            restore_board;
            continue;
        }
        makeMove(move, 0);
        history_table.push_back(hash_key);
        searchBest(depth);
        makeMove(best_move, 0);
        history_table.push_back(hash_key);
    }
    return 0;
}
