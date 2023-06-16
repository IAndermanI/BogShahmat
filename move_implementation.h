#ifndef BOGSHAHMAT_MOVE_IMPLEMENTATION_H
#define BOGSHAHMAT_MOVE_IMPLEMENTATION_H

#include "bitboards_and_constants.h"
#include "move_logic.h"
#include "zobrist.h"

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
    return 0;
}


#endif //BOGSHAHMAT_MOVE_IMPLEMENTATION_H
