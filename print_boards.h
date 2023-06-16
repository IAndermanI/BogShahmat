#ifndef BOGSHAHMAT_PRINT_BOARDS_H
#define BOGSHAHMAT_PRINT_BOARDS_H

#include "move_logic.h"

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
}

#endif //BOGSHAHMAT_PRINT_BOARDS_H
