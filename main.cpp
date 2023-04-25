//Created by IAndermanI. Version: 1.0
#include <bits/stdc++.h>

using namespace std;

enum typeOfPiece {
    PAWN = 'p',
    KNIGHT = 'k',
    BISHOP = 'b',
    ROOK = 'r',
    QUEEN = 'q',
    KING = 'K'
};

class square {
    int x; //from a to h
    int y; //from 1 to 8
public:
    square() {
        x = 1;
        y = 1;
    }

    square(int x, int y) {
        this->x = x;
        this->y = y;
    }

    int getX() {
        return x;
    }

    int getY() {
        return y;
    }

    void operator=(square sq) {
        x = sq.x;
        y = sq.y;
    }

    bool operator==(square sq) {
        return x == sq.x && y == sq.y;
    }

    friend ostream &operator<<(ostream &stream, square sq) {
        stream << sq.x << ' ' << sq.y << ' ';
    }

    bool isValid() {
        return this->x >= 1 && this->x <= 8 && this->y >= 1 && this->y <= 8;
    }
};

class board;

class figure {
    square sq;
    bool color; // False = white, True = black
    typeOfPiece type;
    vector<figure *> possibleMoves;
public:
    figure() {}

    figure(square sq, bool color) {
        this->sq = sq;
        this->color = color;
    }

    square getSquare() {
        return sq;
    }

    bool getColor() {
        return color;
    }
    typeOfPiece getType() {
        return type;
    }
    void setColor(bool color) {
        this->color = color;
    }

    void setSquare(square sq) {
        this->sq = sq;
    }
    void setType(typeOfPiece t) {
        this->type = t;
    }
    void insertMove(figure *fig) {
        possibleMoves.push_back(fig);
    }

    virtual void findMoves(board &) = 0;

    void clearMoves() {
        possibleMoves.clear();
    }

    vector<figure *> getPossibleMoves() {
        return possibleMoves;
    }
    int cost(){
        if(type == PAWN){
            return 1;
        }
        else if(type == KNIGHT){
            return 3;
        }
        else if(type == BISHOP){
            return 3;
        }
        else if(type == ROOK){
            return 5;
        }
        else if(type == QUEEN){
            return 9;
        }
        else if(type == KING){
            return 200;
        }
        else{
            return 200;
        }
    }

};


class board {
    vector<figure *> figs;
public:
    board() {}

    board(vector<figure *> figs) {
        for (int i = 0; i < figs.size(); i++) {
            this->figs.push_back(figs[i]);
        }
    }

    figure *find(square &sq) {
        for (int i = 0; i < figs.size(); i++) {
            if (figs[i]->getSquare() == sq) {
                return figs[i];
            }
        }
        return nullptr;
    }

    void remove(square* sq) {
        for (int i = 0; i < figs.size(); i++) {
            if (figs[i]->getSquare() == *sq) {
                figs.erase(figs.begin() + i);
                return;
            }
        }
    }
    void append(figure* fig){
        figs.push_back(fig);
    }
    void print() {
        char a[9][9];
        for (int i = 0; i < 9; i++) {
            for (int j = 0; j < 9; j++) {
                if (i == 0 && j == 0) {
                    a[i][j] = ' ';
                } else if (j == 0) {
                    a[i][j] = i + 'a' - 1;
                } else if (i == 0) {
                    a[i][j] = j + '0';
                } else {
                    a[i][j] = '.';
                }
            }
        }
        for (int i = 0; i < figs.size(); i++) {
            a[figs[i]->getSquare().getX()][figs[i]->getSquare().getY()] = figs[i]->getType();
        }
        for (int i = 8; i >= 0; i--) {
            for (int j = 0; j < 9; j++) {
                cout << a[j][i] << ' ';
            }
            cout << endl;
        }
    }
    int evaluate(){
        int eval = 0;
        for(int i=0;i<figs.size();i++){
            if(figs[i]->getColor()){
                eval-=figs[i]->cost();
            }
            else{
                eval+=figs[i]->cost();
            }
        }
        return eval;
    }
};


class pawn : public figure {
public:
    pawn() {
        this->setType(PAWN);
    }

    pawn(square sq, bool color) {
        this->setSquare(sq);
        this->setColor(color);
    }

    void findMoves(board &b) {
        int x = this->getSquare().getX();
        int y = this->getSquare().getY();
        bool color = this->getColor();
        for (int i = max(1, x - 1); i <= min(8, x + 1); i++) {
            square sq = square(i, y + (color ? -1 : 1));
            if (y < 7 && !color || y > 2 && color) {
                if (b.find(sq) != nullptr && i != x && b.find(sq)->getColor() != this->getColor()) {
                    b.remove(&sq);
                    pawn* p = new pawn();
                    p->setSquare(sq);
                    p->setColor(color);
                    this->insertMove(p);
                }
                if (b.find(sq) == nullptr && i == x) {
                    pawn* p = new pawn();
                    p->setSquare(sq);
                    p->setColor(color);
                    this->insertMove(p);
                }
            }
        }
    }
};

class knight : public figure {
public:
    knight() {
        this->setType(KNIGHT);
    }

    void findMoves(board &b) {
        int x = this->getSquare().getX();
        int y = this->getSquare().getY();
        for (int i = -2; i <= 2; i++) {
            if (i == 0) continue;
            int j = 3 - abs(i);
            square sq = square(x + i, y + j);
            if (sq.isValid() &&
                (b.find(sq) != nullptr && b.find(sq)->getColor() != this->getColor() || b.find(sq) == nullptr)) {
                knight* k = new knight();
                k->setSquare(sq);
                k->setColor(this->getColor());
                this->insertMove(k);
            }
            sq = square(x + i, y - j);
            if (sq.isValid() &&
                (b.find(sq) != nullptr && b.find(sq)->getColor() != this->getColor() || b.find(sq) == nullptr)) {
                knight* k = new knight();
                k->setSquare(sq);
                k->setColor(this->getColor());
                this->insertMove(k);
            }
        }
    }
};

class bishop : public figure {
public:
    bishop() {
        this->setType(BISHOP);
    }

    void findMoves(board &b) {
        int x = this->getSquare().getX();
        int y = this->getSquare().getY();
        for (int dir = 0; dir < 4; dir++) {
            int i = 1;
            bool notStuck = 1;
            square sq = square(x + i * int(pow(-1, dir)), y + i * int(pow(-1, dir / 2)));
            while (sq.isValid() && notStuck) {
                if (b.find(sq) != nullptr && b.find(sq)->getColor() != this->getColor() ||
                    b.find(sq) == nullptr) {
                    notStuck = b.find(sq) == nullptr;
                    bishop* B = new bishop();
                    B->setSquare(sq);
                    B->setColor(this->getColor());
                    this->insertMove(B);
                }
                i++;
                sq = square(x + i * int(pow(-1, dir)), y + i * int(pow(-1, dir / 2)));
            }
        }
    }
};

class rook : public figure {
public:
    rook() {
        this->setType(ROOK);
    }

    void findMoves(board &b) {
        int x = this->getSquare().getX();
        int y = this->getSquare().getY();
        for (int dir = 0; dir < 4; dir++) {
            int i = 1;
            square sq = square(x + i * int(pow(-1, dir / 2)) * (dir % 2), y + i * int(pow(-1, dir / 2)) * ((dir + 1) % 2));
            bool notStuck = 1;
            while (sq.isValid() && notStuck) {
                if (b.find(sq) != nullptr && b.find(sq)->getColor() != this->getColor() ||
                    b.find(sq) == nullptr) {
                    notStuck = b.find(sq) == nullptr;
                    rook* r = new rook();
                    r->setSquare(sq);
                    r->setColor(this->getColor());
                    this->insertMove(r);
                }
                i++;
                sq = square(x + i * int(pow(-1, dir / 2)) * (dir % 2), y + i * int(pow(-1, dir / 2)) * ((dir + 1) % 2));

            }
        }
    }
};

class queen : public figure {
public:
    queen() {
        this->setType(QUEEN);
    }
    void findMoves(board &b) {
        int x = this->getSquare().getX();
        int y = this->getSquare().getY();
        for (int dir = 0; dir < 4; dir++) {
            int i = 1;
            bool notStuck = 1;
            square sq = square(x + i * int(pow(-1, dir)), y + i * int(pow(-1, dir / 2)));
            while (sq.isValid() && notStuck) {
                if (b.find(sq) != nullptr && b.find(sq)->getColor() != this->getColor() ||
                    b.find(sq) == nullptr) {
                    notStuck = b.find(sq) == nullptr;
                    queen* q = new queen();
                    q->setSquare(sq);
                    q->setColor(this->getColor());
                    this->insertMove(q);
                }
                i++;
                sq = square(x + i * int(pow(-1, dir)), y + i * int(pow(-1, dir / 2)));
            }
        }
        for (int dir = 0; dir < 4; dir++) {
            int i = 1;
            square sq = square(x + i * int(pow(-1, dir / 2)) * (dir % 2), y + i * int(pow(-1, dir / 2)) * ((dir + 1) % 2));
            bool notStuck = 1;
            while (sq.isValid() && notStuck) {
                if (b.find(sq) != nullptr && b.find(sq)->getColor() != this->getColor() ||
                    b.find(sq) == nullptr) {
                    notStuck = b.find(sq) == nullptr;
                    queen* q = new queen();
                    q->setSquare(sq);
                    q->setColor(this->getColor());
                    this->insertMove(q);
                }
                i++;
                sq = square(x + i * int(pow(-1, dir / 2)) * (dir % 2), y + i * int(pow(-1, dir / 2)) * ((dir + 1) % 2));
            }
        }
    }
};

class king : public figure {
public:
    king() {
        this->setType(KING);
    }
    void findMoves(board &b) {
        int x = this->getSquare().getX();
        int y = this->getSquare().getY();
        for (int dir = 0; dir < 4; dir++) {
            square sq = square(x + int(pow(-1, dir)), y + int(pow(-1, dir / 2)));
            if(!sq.isValid()){
                continue;
            }
            if (b.find(sq) != nullptr && b.find(sq)->getColor() != this->getColor() ||
                b.find(sq) == nullptr) {
                king* k = new king();
                k->setSquare(sq);
                k->setColor(this->getColor());
                this->insertMove(k);
            }
        }
        for (int dir = 0; dir < 4; dir++) {
            square sq = square(x + int(pow(-1, dir / 2)) * (dir % 2), y + int(pow(-1, dir / 2)) * ((dir + 1) % 2));
            if(!sq.isValid()){
                continue;
            }
            if (b.find(sq) != nullptr && b.find(sq)->getColor() != this->getColor() ||
                b.find(sq) == nullptr) {
                king* k = new king();
                k->setSquare(sq);
                k->setColor(this->getColor());
                this->insertMove(k);
            }
        }
    }
};

class position {
    position* prev;
    board b;
    bool color;
    vector<position *> next;
    int eval;
public:
    position(){}
    position(position* prev, board& b, bool color){
        this->prev = prev;
        this->b = b;
        this->color = color;
        eval = b.evaluate();
    }
    void doAllMoves() {
        for (int i = 1; i < 9; i++) {
            for (int j = 1; j < 9; j++) {
                square sq(i, j);
                figure *fig = b.find(sq);
                if (fig != nullptr && fig->getColor() == color) {
                    fig->findMoves(b);
                    vector<figure *> possible = fig->getPossibleMoves();
                    fig->clearMoves();
                    board* bNew = new board();
                    *bNew = b;
                    bNew->remove(&sq);
                    if(possible.size() == 0){
                        bNew->append(fig);
                    }
                    for (int k = 0; k < possible.size(); k++) {
                        board* toNext = new board();
                        *toNext = *bNew;
                        square sq = possible[k]->getSquare();
                        if(toNext->find(sq)){
                            toNext->remove(&sq);
                        }
                        toNext->append(possible[k]);
                        position* newPosition = new position(this, *toNext, !color);
                        next.push_back(newPosition);
                    }

                }
            }
        }
    }
    int getEval(){
        return eval;
    }
    bool getColor(){
        return color;
    }
    int nextSize(){
        return next.size();
    }
    void print(){
        b.print();
    }
    position* getNext(int i){
        return next[i];
    }
};


int calculate(position* p, int depth = 5){
    if(depth == 0) {
        return p->getEval();
    }
    p->doAllMoves();
    int cost = INT_MAX + (1 - int(p->getColor()));
    position theBest = *p;
    for(int i=0;i<p->nextSize();i++){
        int calc = calculate(p->getNext(i), depth-1);
        if(p->getColor()){
            if(calc < cost){
                cost = calc;
                theBest = *p->getNext(i);
            }
        }
        else{
            if(calc > cost){
                cost = calc;
                theBest = *p->getNext(i);
            }
        }
    }
    return cost;
}

void inputBoard(board* b){
    string s;
    while(getline(cin, s)){
        figure* fig;
        bool color;
        if(s[0] == 'B'){
            color = 1;
        }
        else if(s[0] == 'W'){
            color = 0;
        }
        else{
            cout<<"Incorrect input: "<<s<<". Please, try again\n";
            continue;
        }

        if(s[1] == 'p'){
            fig = new pawn();
        }
        else if(s[1] == 'k'){
            fig = new knight();
        }
        else if(s[1] == 'b'){
            fig = new bishop();
        }
        else if(s[1] == 'r'){
            fig = new rook();
        }
        else if(s[1] == 'q'){
            fig = new queen();
        }
        else if(s[1] == 'K'){
            fig = new king();
        }
        else{
            cout<<"Incorrect input: "<<s<<". Please, try again\n";
            continue;
        }
        fig->setColor(color);
        int x = s[2] - 'a' + 1;
        int y = s[3] - '0';
        square sq = square(x, y);
        if(!sq.isValid()){
            cout<<"Incorrect input: "<<s<<". Please, try again\n";
            continue;
        }
        fig->setSquare(sq);
        b->append(fig);
    }
}
int main() {
    freopen("input.txt", "r", stdin);
    freopen("output.txt", "w", stdout);
    board b;
    inputBoard(&b);
    b.print();
    position p(nullptr, b, 0);
    int score = calculate(&p, 3);
    cout<<score<<endl;
    return 0;
}