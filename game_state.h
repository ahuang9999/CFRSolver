#ifndef GAME_STATE_H
#define GAME_STATE_H

#include <vector>
#include <string>
#include <memory>
using namespace std;


struct Action {
    int target; //0 for top, 1 for bottom
    int bid;

    Action(int t, int b) : target(t), bid(b) {}

    Action() : target(-1), bid(-1) {}

    bool operator==(const Action& other) const {
        return target == other.target && bid == other.bid;
    }
    bool operator<(const Action& other) const {
        if (target != other.target) {
            return target < other.target;
        }
        return bid < other.bid;
    }
    string toString() {
        if (target == 0) return "top" + to_string(bid);
        return "bottom" + to_string(bid);
    }
    
};



class GameState {

public:
    virtual ~GameState() {}
    virtual int getCurrentPlayer() const = 0;

    virtual void getLegalActions(vector<Action>& actions) = 0;

    virtual bool isTerminal() = 0;

    virtual void applyAction(Action action) = 0;

    virtual double getUtility(int player_id) = 0;

    virtual std::string getInfoSet() = 0;

};

#endif