#ifndef BOGSHAHMAT_PRINT_MOVES_H
#define BOGSHAHMAT_PRINT_MOVES_H

#include "print_boards.h"
#include "move_implementation.h"

void printAttacks(bool color) {
    u64 result = 0;
    for (int i = 0; i < 64; i++) {
        result |= u64(isAttacked(i, color)) << i;
    }
    cout << endl;
    printBitboard(result);
}

string moveToString(int move) {
    return squareString(getMoveSource(move)) + squareString(getMoveTarget(move)) +
            (getMovePromotedPiece(move) ? name_string[getMovePromotedPiece(move)] : ' ');
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

#endif //BOGSHAHMAT_PRINT_MOVES_H
