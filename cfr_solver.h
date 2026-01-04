#ifndef CFR_SOLVER_H
#define CFR_SOLVER_H

#include <iostream>
#include <vector>
#include <unordered_map>
#include <random>
#include <string>
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
    

public:
    CFRSolver() {};

    ~CFRSolver() {};

    double cfr(GameType state, int update_player, double pi_player);

    void train(int iterations);

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

    Node& node = nodeMap[infoSet];
    if (node.regretSum.empty()) {
        node.regretSum.resize(legal_actions.size(), 0.0);
        node.strategySum.resize(legal_actions.size(), 0.0);

        //node.infoSet = infoSet;
    }

    //double current_player_reach = reach_probabilities[current_player];
    vector<double> strategy = node.getStrategy((current_player == update_player) ? pi_player : 0);
    

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

    for (int a=0; a<legal_actions.size(); a++) {
        double regret = util[a] - nodeUtil;
        node.regretSum[a] += regret;
    }



    return nodeUtil;

}



template<typename GameType>
const unordered_map<string,Node>& CFRSolver<GameType>::getNodeMap() const {
    return nodeMap;
}




template<typename GameType>
void CFRSolver<GameType>::train(int iterations) {
    double totalUtil = 0;

    for (int z=0; z<iterations; z++) {
        GameType root;

        for (int i=0; i<NUM_PLAYERS; i++) {
            totalUtil += cfr(root,i,1.0);
        }

        if (z%1000 == 0) {
             cout << "Iteration " << z << endl;
        }
        if (z%10000 == 0) cout << nodeMap.size() << endl;

    }
}


#endif








/*template<typename GameType>
double CFRSolver<GameType>::cfr(GameType state, int update_player, vector<double> reach_probabilities) {
        if (state.isTerminal()) {
            return state.getUtility(update_player);
        }

        int current_player = state.getCurrentPlayer();
        string infoSet = state.getInfoSet();
        vector<Action> legal_actions;
        state.getLegalActions(legal_actions);

        Node& node = nodeMap[infoSet];
        if (node.regretSum.empty()) {
            node.regretSum.resize(legal_actions.size(), 0.0);
            node.strategy.resize(legal_actions.size(), 0.0);
            node.strategySum.resize(legal_actions.size(), 0.0);
            node.infoSet = infoSet;
        }

        double current_player_reach = reach_probabilities[current_player];
        vector<double> strategy = node.getStrategy(current_player_reach);
        vector<double> util(legal_actions.size(),0);

        double nodeUtil = 0;
        for (int a=0; a<legal_actions.size(); a++) {
            GameType nextState = state;

            nextState.applyAction(legal_actions[a]);

            vector<double> reach_copy = reach_probabilities;
            reach_copy[current_player] *= strategy[a];

            util[a] = cfr(nextState, update_player, reach_copy);

            nodeUtil += strategy[a] * util[a];
        }

        if (current_player == update_player) {
            double opponent_reach = 1.0;
            for (int r=0; r<reach_probabilities.size(); r++) {
                if (r != current_player) {
                    opponent_reach *= reach_probabilities[r];
                }
            }
            for (int a=0; a<legal_actions.size(); a++) {
                double regret = util[a] - nodeUtil;
                node.regretSum[a] += opponent_reach*regret;
            }
        }


        return nodeUtil;

}*/


/*
template<typename GameType>
void CFRSolver<GameType>::pruneNegativeRegrets() {
    for (auto& pair : nodeMap) {
        Node& node = pair.second;
        for (int a = 0; a < node.regretSum.size(); a++) {
            if (node.regretSum[a] < 0) {
                node.regretSum[a] = 0;  // Discard negative regret
            }
        }
    }
}
*/