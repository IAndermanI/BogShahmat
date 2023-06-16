#include "interaction.h"

void fillEverything() {
    fillAllMoves();
    fillHashKeys();
    fillEvaluationMasks();
}

// MAIN
int main() {
    fillEverything();
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
}
