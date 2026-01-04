#ifndef GAME_ENGINE
#define GAME_ENGINE

#include <iostream>
#include <array>
#include <map>
#include <random>
#include <string>
#include "game_state.h"
#include "phevaluator/phevaluator.h"
using namespace std;


struct Card {
    int rank; //23456789TJQKA
    int suit; //cdhs
    string toString() {
        string ranks = "23456789TJQKA";
        string suits = "cdhs";
        return string(1,ranks[rank]) + string(1,suits[suit]);
    }
    int get_id() const {
        return suit*13 + rank;
    }
    Card(int r, int s) : rank(r), suit(s) {}
    Card() : rank(-1), suit(-1) {}
};


class PloAuctionGame : public GameState {

    array<array<Card,4>,2> board;
    int player0_capital;
    int player1_capital;
    int round; //1 to 4
    bool is_player_0;
    array<Card,13> deck;
    array<array<Card,4>,2> player_hands;
    Action player0_pending_action;
    
    array<Action, 8> history; 
    int history_count;
    
    void reset();

    void resolveRound(Action p0, Action p1);

public:
    PloAuctionGame();

    //PloAuctionGame(const PloAuctionGame& other);

    int getCurrentPlayer() const override;

    void getLegalActions(vector<Action>& actions) override;

    bool isTerminal() override;

    void applyAction(Action action) override;

    double getUtility(int player_id) override;

    string getInfoSet() override;

};

#endif