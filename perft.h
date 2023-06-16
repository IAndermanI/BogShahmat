#ifndef BOGSHAHMAT_PERFT_H
#define BOGSHAHMAT_PERFT_H

#include <windows.h>
#include "move_implementation.h"

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


#endif //BOGSHAHMAT_PERFT_H
