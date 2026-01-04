#include "my_game_engine.h"
#include <algorithm>
using namespace std;

random_device rd;
std::mt19937 gen(rd());
const int INITIAL_CAPITAL = 100, NUM_ROUNDS = 4;
const vector<int> abstract_bets {2,6,12};



void PloAuctionGame::reset() {
    is_player_0 = true;
    player0_capital = INITIAL_CAPITAL;
    player1_capital = INITIAL_CAPITAL;
    round = 1;
    history_count = 0;

    //make the deck
    array<Card,52> temp_deck;
    string ranks = "23456789TJQKA";
    string suits = "cdhs";
    for (int i=0; i<13; i++) {
        for (int j=0; j<4; j++) {
            temp_deck[4*i+j] = Card(i,j);
        }
    }

    for (int c1 = temp_deck.size()-1; c1 > 0; c1--) {
        uniform_int_distribution<> dist(0,c1);
        int c2 = dist(gen);
        swap(temp_deck[c1],temp_deck[c2]);
    }
    for (int i=0; i<13; i++) {
        deck[i] = temp_deck[i];
    }
    for (int i=0; i<8; i++) {
        int even = i%2 == 0 ? 0 : 1;
        board[even][i/2] = deck[i];
    }
}


void PloAuctionGame::resolveRound(Action p0, Action p1) {
    int player_to_alter = -1;
    if (p0 == p1) {
        uniform_int_distribution<> dist(0,1);
        player_to_alter = dist(gen);
    }
    else if (p0.target == p1.target) {
        p0 < p1 ? player_to_alter = 1 : player_to_alter = 0;
    }
    else {
        player_hands[0][round-1] = board[p0.target][round-1];
        player_hands[1][round-1] = board[p1.target][round-1];
        player0_capital -= p0.bid;
        player1_capital -= p1.bid;
    }

    if (player_to_alter != -1) { //both bid on the same thing
        int winner = 1 - player_to_alter;
        player_hands[winner][round-1] = board[p0.target][round-1];
        player_hands[1-winner][round-1] = board[1-p0.target][round-1];
        winner == 0 ? player0_capital -= p0.bid : player1_capital -= p1.bid;
        winner == 0 ? player1_capital -= min(1,player1_capital) : player0_capital -= min(1,player1_capital);
    }
    if (history_count < 8) {
        history[history_count++] = p0;
        history[history_count++] = p1;
    }

}

PloAuctionGame::PloAuctionGame() : player0_pending_action(0, 67) {
    reset();
}

//PloAuctionGame::PloAuctionGame(const PloAuctionGame& other) : board(other.board), player0_capital(other.player0_capital), player1_capital(other.player1_capital), round(other.round), is_player_0(other.is_player_0), deck(other.deck), player_hands(other.player_hands), player0_pending_action(other.player0_pending_action), history(other.history), history_count(other.history_count)
//{}


int PloAuctionGame::getCurrentPlayer() const {
    return is_player_0 ? 0 : 1;
}

void PloAuctionGame::getLegalActions(vector<Action>& actions) {
    for (int i=0; i<2; i++) {
        for (int bet: abstract_bets) {
            if (bet > (is_player_0 ? player0_capital : player1_capital) ) break;
            actions.push_back(Action(i,bet));
        }
    }
}

bool PloAuctionGame::isTerminal() {
    return (round>NUM_ROUNDS);
}

void PloAuctionGame::applyAction(Action action) {
    if (isTerminal()) return;


    if (is_player_0) {
        player0_pending_action = action;
        is_player_0 = false;
    }
    else {
        resolveRound(player0_pending_action,action);
        is_player_0 = true;
        round++;
    }
}



double PloAuctionGame::getUtility(int player_id) {
    //todo, most annoying part
    if (!isTerminal()) return 0.0;
    phevaluator::Rank rank0 = phevaluator::EvaluatePlo4Cards(deck[8].toString(), deck[9].toString(), deck[10].toString(), deck[11].toString(), deck[12].toString(), player_hands[0][0].toString(), player_hands[0][1].toString(), player_hands[0][2].toString(), player_hands[0][3].toString());
    phevaluator::Rank rank1 = phevaluator::EvaluatePlo4Cards(deck[8].toString(), deck[9].toString(), deck[10].toString(), deck[11].toString(), deck[12].toString(), player_hands[1][0].toString(), player_hands[1][1].toString(), player_hands[1][2].toString(), player_hands[1][3].toString());
    int winner;
    if (rank0.value() == rank1.value()) {
        return 0.0;
    }
    else if (rank0.value() < rank1.value()) { //player 0 has a better hand
        winner = 0;
    }
    else {
        winner = 1;
    }

    int total_spent_p0 = INITIAL_CAPITAL - player0_capital;
    int total_spent_p1 = INITIAL_CAPITAL - player1_capital;
    int pot = total_spent_p0 + total_spent_p1;
    int effective_stack = min(player0_capital, player1_capital);
    pot += effective_stack;

    int final_wealth_p0 = (winner == 0) ? (player0_capital - effective_stack + pot) : (player0_capital - effective_stack);
    int final_wealth_p1 = (winner == 1) ? (player1_capital - effective_stack + pot) : (player1_capital - effective_stack);
    if (player_id == 0) return final_wealth_p0 - INITIAL_CAPITAL;
    return final_wealth_p1 - INITIAL_CAPITAL;


}



//infoset abstraction helper functions
string countBroadways(const Card& c1, const Card& c2) {
    if (c1.rank >= 9 && c2.rank >= 9) return "H2";
    else if (c1.rank < 9 && c2.rank < 9) return "H0";
    return "H1";
}

char capitalAbstraction(int capital) {
    if (capital == 100) return 'F';
    else if (capital >= 90) {
        return 'D';
    }
    else if (capital >= 75) {
        return 'M';
    }
    else if (capital >= 40) {
        return 'S';
    }
    return 'L';
}

char getRankBucket(int rank) {
    if (rank == 12) return 'A';
    if (rank >= 9) return 'H';
    else if (rank >= 5) return 'M';
    return 'L';
}

char getRankChar(int rank) {
    if (rank == 12) {
        return 'A';
    }
    else if (rank == 11) {
        return 'K';
    }
    else if (rank == 10) {
        return 'Q';
    }
    else if (rank >= 8) {
        return 'J';
    }
    else if (rank >= 5) {
        return '9';
    }
    return '6';
}


string PloAuctionGame::getInfoSet() {
    string result;
    result.reserve(64);
    result = "P" + to_string(getCurrentPlayer()) + "_";


    int suit_map[4] = {-1, -1, -1, -1};
    char next_suit = 'a';
    auto get_suit_char = [&](int real_suit) -> char {
        if (suit_map[real_suit] == -1) suit_map[real_suit] = next_suit++;
        return (char)suit_map[real_suit];
    };

    //sort cards
    
    int p_id = getCurrentPlayer();
    array<Card,4> sorted_hand = player_hands[p_id];
    sort(sorted_hand.begin(), sorted_hand.end(), [](const Card& a, const Card& b) {
        return a.rank > b.rank; 
    });
    for (auto& c: sorted_hand) {
        if (c.rank != -1) {
            result += getRankChar(c.rank);
            result += get_suit_char(c.suit);
        }
    }
    result += "_";

    int dead_A = 0, dead_H = 0, dead_M = 0, dead_L = 0;
    
    for (int r=1; r<=NUM_ROUNDS; r++) {
        Card c1 = deck[2*(r-1)];
        Card c2 = deck[2*(r-1)+1];
        if (r < round) {
            char b1 = getRankBucket(c1.rank);
            char b2 = getRankBucket(c2.rank);
            if (b1 == 'A') dead_A++; else if (b1 == 'H') dead_H++; else if (b1 == 'M') dead_M++; else dead_L++;
            if (b2 == 'A') dead_A++; else if (b2 == 'H') dead_H++; else if (b2 == 'M') dead_M++; else dead_L++;
        }
        else if (r == round) {
            result += "Cur:";
            result += getRankBucket(c1.rank);
            result += get_suit_char(c1.suit);
            result += getRankBucket(c2.rank);
            result += get_suit_char(c2.suit);
            result += "_";
        }
        else {
            result += countBroadways(c1,c2);
        }
    }
    result += "D:" + to_string(dead_A) + to_string(dead_H) + to_string(dead_M) + to_string(dead_L);

    result += "_C";
    result += is_player_0 ? capitalAbstraction(player0_capital) : capitalAbstraction(player1_capital);
    result += is_player_0 ? capitalAbstraction(player1_capital) : capitalAbstraction(player0_capital);
    return result;
}

