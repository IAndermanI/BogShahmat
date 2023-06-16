#ifndef BOGSHAHMAT_INTERACTION_H
#define BOGSHAHMAT_INTERACTION_H

#include "searching.h"

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

#endif //BOGSHAHMAT_INTERACTION_H
