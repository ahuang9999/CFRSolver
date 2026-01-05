#include <iostream>
#include <fstream>
#include <iomanip>
#include "cfr_solver.h"
#include "my_game_engine.h"
using namespace std;

int main() {
    int num_threads = 8;

    CFRSolver<PloAuctionGame> solver;
    int iterations = 200000000;
    cout << "Iterations starting\n";
    solver.train(iterations, num_threads);

    ofstream out("strategies.txt");

    auto& nodeMap = solver.getNodeMap();

    vector<string> keys;
    for (const auto& pair : nodeMap) {
        keys.push_back(pair.first);
    }
    sort(keys.begin(), keys.end());
    int z = 0;
    for (const string& key : keys) {
        Node& node = const_cast<Node&>(nodeMap.at(key));
        out << key << " " << node.toString() << "\n";
        if (z%10000 == 0) {
            cout << "InfoSet: " << key << endl;
            cout << "  Strategy: " << node.toString() << endl;
        }
        z++;
    }
    cout << keys.size() << endl;
}