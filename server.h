#ifndef BOGSHAHMAT_SERVER_H
#define BOGSHAHMAT_SERVER_H

#include <fstream>
using namespace std;
using namespace filesystem;

path absolute_path;

void createFile(string filename = "connector.txt"){
    ofstream file(filename);
    absolute_path = absolute("connector.txt");
}

void deleteFile(){
    remove(absolute_path);
}

string readFromFile(){
    ifstream fin(absolute_path, ios_base::in);
    string client;
    fin >> client;
    if (client != "client"){
        fin.close();
        return "";
    }
    else{
        string str;
        fin >> str;
        fin.close();
        return str;
    }

}

void loadToFile(string message){
    ofstream fout(absolute_path, ios_base::out | ios_base::trunc);
    fout << "server\n" << message;
    fout.close();
}

#endif //BOGSHAHMAT_SERVER_H
