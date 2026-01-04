#include "cfr_solver.h"
using namespace std;



vector<double> Node::getStrategy(double realizationWeight) { //realizationWeight: probability of player playing to reach node, pi_i
    double normalizingSum = 0;
    vector<double> strategy(regretSum.size());
    int num_actions = regretSum.size();
    for (int a=0; a<num_actions; a++) {
        strategy[a] = regretSum[a] > 0 ? regretSum[a] : 0;
        normalizingSum += strategy[a];
    }
    for (int a=0; a<num_actions; a++) {
        if (normalizingSum > 0) {
            strategy[a] /= normalizingSum;
        }
        else {
            strategy[a] = 1.0/num_actions;
        }
        strategySum[a] += realizationWeight*strategy[a];
    }
    return strategy;
}

vector<double> Node::getAverageStrategy() {
    int num_actions = regretSum.size();
    vector<double> avgStrategy(num_actions,0);
    double normalizingSum = 0;
    for (int a=0; a<num_actions; a++) {
        normalizingSum += strategySum[a];
    }
    for (int a=0; a<num_actions; a++) {
        if (normalizingSum == 0) {
            avgStrategy[a] = 1.0/num_actions;
        }
        else {
            avgStrategy[a] = strategySum[a]/normalizingSum;
        }
    }
    return avgStrategy;
}

string Node::toString() {
    string result = "[";
    const auto& v = getAverageStrategy();
    for (size_t i = 0; i < v.size(); ++i) {
        result += to_string(v[i]);
        if (i + 1 < v.size()) {
            result += ", ";
        }
    }
    result += "]";
    return result;
}

