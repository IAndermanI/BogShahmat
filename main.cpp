#include "interaction.h"
#include "server.h"

void fillEverything() {
    fillAllMoves();
    fillHashKeys();
    fillEvaluationMasks();
}

void consolePlay(){
    fillEverything();
    setBoard("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    cout << "INPUT COLOR:\n";
    string color;
    cin >> color;
    cout << "OK. LETS START!\n";
    if (color == "white") {
        int depth = 6;
        searchBest(depth);
        makeMove(best_move, 0);
    }
    while (true) {
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
        printBoard();
    }
}
/*
void playWithLichess(){
    fillEverything();
    setBoard("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    createFile();
    string color;
    while(color.empty()){
        color = readFromFile();
    }
    int depth = 6;
    if (color == "white") {
        searchBest(depth);
        loadToFile(moveToString(best_move));
        makeMove(best_move, 0);
    }
    while (true) {
        string move_string;
        while(move_string.empty() || move_string == "black"){
            move_string = readFromFile();
        }
        copy_board
        int move = parse_move(move_string);
        if (move == 0) {
            restore_board
            continue;
        }
        makeMove(move, 0);
        history_table.push_back(hash_key);
        nodes = 0;
        searchBest(depth);
        if(nodes > 150000 && depth > 6){
            depth--;
        }
        if(nodes < 100000){
            depth++;
        }
        loadToFile(moveToString(best_move));
        makeMove(best_move, 0);
        history_table.push_back(hash_key);
    }
}
*/
// MAIN
int main() {
    int mode = 0;
    if (!mode) consolePlay();
    /*
    else if (mode == 1) playWithLichess();
    */
}
