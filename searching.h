#ifndef BOGSHAHMAT_SEARCHING_H
#define BOGSHAHMAT_SEARCHING_H

#include "move_implementation.h"
#include "evaluation.h"
#include "print_moves.h"

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
    /*for (int i = 1; i <= depth; i++) {
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
        cout << endl << endl;
        if (i == depth) {
            return score;
        }
    }
    */
    follow_pv = 1;
    int score = negamax(alpha, beta, depth);
    alpha = score - 200;
    beta = score + 200;
    cout << "BEST MOVE: ";
    printMove(best_move);
    cout << endl;
    cout << "BEST LINE: ";
    for (int i = 0; i < pv_length[0]; i++) {
        printMove(pv_table[0][i]);
    }
    cout << endl << endl;
    return score;
}

#endif //BOGSHAHMAT_SEARCHING_H
