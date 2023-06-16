#ifndef BOGSHAHMAT_EVALUATION_H
#define BOGSHAHMAT_EVALUATION_H

#include "move_implementation.h"


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

u64 rank_masks[64];
u64 file_masks[64];
u64 isolated_masks[64];
u64 passed_pawns_masks[2][64];
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
int double_pawns_penalty = 20;
int isolated_pawn_score = -15;
int passed_pawn_score[8] = {0, 5, 10, 20, 35, 60, 100, 200};
const int evaluation_array[12]{100, 270, 290, 430, 890, 10000, -100, -270, -290, -430, -890, -10000};
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
        /*while(target_squares){
            int target_square = getLSBIndex(target_squares);
            pop_bit(target_squares, target_square);
            score += square_score[target_square];
            if((1ULL << target_square) & occupancies[color] & ~bitboards[color ? k : K]){
                score += defense_score;
            }
        }
         */
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
        /*while(target_squares){
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
         */
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
        /*while(target_squares){
            bits++;
            int target_square = getLSBIndex(target_squares);
            pop_bit(target_squares, target_square);
            score += square_score[target_square];
            if((1ULL << target_square) & occupancies[color] & ~bitboards[color ? k : K]){
                score += defense_score;
            }
        }
         */
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
        /*while(target_squares){
            int target_square = getLSBIndex(target_squares);
            pop_bit(target_squares, target_square);
            score += square_score[target_square];
            if((1ULL << target_square) & occupancies[color] & ~bitboards[color ? k : K]){
                score += defense_score;
            }
        }
         */
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

/* "Smarter" version of evaluation
 * static inline int evaluate(){
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
 */

int double_pawns_score = 20;

static inline int evaluate() {
    int score = 0;
    for (int i = P; i <= k; i++) {
        u64 bitboard_copy = bitboards[i];
        while (bitboard_copy) {
            int square = getLSBIndex(bitboard_copy);
            score += evaluation_array[i];
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
#endif //BOGSHAHMAT_EVALUATION_H
