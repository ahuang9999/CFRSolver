#include <iostream>
#include <cassert>
#include "phevaluator/phevaluator.h"

int main() {
    phevaluator::Rank rank1 = phevaluator::EvaluatePlo4Cards("4c","4h","4d","Ac","2c","Ah","Kh","Kc","9h");
    assert(rank1.category() == FULL_HOUSE);
    assert(rank1.describeSampleHand() == "444KK");
}