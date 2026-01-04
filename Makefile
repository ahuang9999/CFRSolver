
run:
	g++ -O3 -std=c++17 main.cpp my_game_engine.cpp cfr_solver.cpp PokerHandEvaluator/cpp/build/libphevalplo4.a -I PokerHandEvaluator/cpp/include -o solver