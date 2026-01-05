#ifndef CFR_SOLVER_H
#define CFR_SOLVER_H

#include <iostream>
#include <vector>
#include <unordered_map>
#include <random>
#include <string>
#include <mutex>
#include <shared_mutex>
#include <memory>
#include <omp.h>
#include "game_state.h"
using namespace std;

const int NUM_PLAYERS = 2;


struct Node { //represents information set
    vector<double> regretSum;
    vector<double> strategySum;


    vector<double> getStrategy(double realizationWeight); //realizationWeight: probability of player playing to reach node, pi_i
    vector<double> getAverageStrategy();
    string toString();

};

 //info set string to actual info set


template<typename GameType>
class CFRSolver {
    unordered_map<string, Node> nodeMap;

    mutable shared_mutex map_mutex;
    static const int NUM_SHARDS = 4096;
    vector<unique_ptr<mutex>> value_mutexes;

    mutex& getShardMutex(const string& key) {
        size_t hash_val = hash<string>{}(key);
        return *value_mutexes[hash_val % NUM_SHARDS];
    }
    

public:
    CFRSolver() {
        for(int i=0; i<NUM_SHARDS; i++) {
            value_mutexes.push_back(make_unique<mutex>());
        }
        nodeMap.reserve(20000000);
    };

    ~CFRSolver() {};

    double cfr(GameType state, int update_player, double pi_player);

    void train(int iterations, int num_threads);

    const unordered_map<string,Node>& getNodeMap() const;
};



template<typename GameType>
double CFRSolver<GameType>::cfr(GameType state, int update_player, double pi_player) {
    if (state.isTerminal()) {
        return state.getUtility(update_player);
    }

    int current_player = state.getCurrentPlayer();
    string infoSet = state.getInfoSet();
    vector<Action> legal_actions;
    state.getLegalActions(legal_actions);


    Node* nodePtr = nullptr;
    {
        shared_lock<shared_mutex> read_lock(map_mutex);
        auto it = nodeMap.find(infoSet);
        if (it != nodeMap.end()) {
            nodePtr = &it->second;
        }
    }

    if (!nodePtr) {
        unique_lock<shared_mutex> write_lock(map_mutex);
        auto it = nodeMap.find(infoSet);
        if (it != nodeMap.end()) {
            nodePtr = &it->second;
        }
        else {
            auto& nodeRef = nodeMap[infoSet];
            nodePtr = &nodeRef;
        }
    }
    nodePtr->regretSum.resize(legal_actions.size(), 0.0);
    nodePtr->strategySum.resize(legal_actions.size(), 0.0);

    vector<double> strategy;
    {
        lock_guard<mutex> lock(getShardMutex(infoSet));
        strategy = nodePtr->getStrategy((current_player == update_player) ? pi_player : 0);
    }



    

    

    if (current_player != update_player) {
        static thread_local mt19937 gen(random_device{}());
        uniform_real_distribution<> dist(0.0,1.0);

        double cumulative = 0.0;
        double x = dist(gen);
        int a_idx = -1;
        for (size_t i = 0; i < strategy.size(); i++) {
            cumulative += strategy[i];
            if (x < cumulative) {
                a_idx = i;
                break;
            }
        }
        if (a_idx == -1) a_idx = strategy.size() - 1;

        GameType nextState = state;
        nextState.applyAction(legal_actions[a_idx]);

        return cfr(nextState, update_player, pi_player);
    }

    vector<double> util(legal_actions.size(),0);
    double nodeUtil = 0;

    for (int a=0; a<legal_actions.size(); a++) {
        GameType nextState = state;
        nextState.applyAction(legal_actions[a]);

        util[a] = cfr(nextState, update_player, pi_player*strategy[a]);

        nodeUtil += strategy[a]*util[a];
    }


    {
        lock_guard<mutex> lock(getShardMutex(infoSet));
        for (size_t a = 0; a < legal_actions.size(); a++) {
            double regret = util[a] - nodeUtil;
            nodePtr->regretSum[a] += regret;
            if (current_player == update_player) {
                nodePtr->strategySum[a] += pi_player * strategy[a];
            }
        }
    }



    return nodeUtil;

}



template<typename GameType>
const unordered_map<string,Node>& CFRSolver<GameType>::getNodeMap() const {
    return nodeMap;
}




template<typename GameType>
void CFRSolver<GameType>::train(int iterations, int num_threads) {

    #pragma omp parallel for schedule(dynamic) num_threads(num_threads)
    for (int z=0; z<iterations; z++) {
        GameType root;

        for (int i=0; i<NUM_PLAYERS; i++) {
            cfr(root,i,1.0);
        }

        if (z%10000 == 0 && omp_get_thread_num() == 0) {
            #pragma omp critical
            cout << "Iteration " << z << endl;
            cout << nodeMap.size() << endl;
        }

    }
}


#endif









