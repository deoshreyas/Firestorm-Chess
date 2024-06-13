#ifndef UCI_H
#define UCI_H

#include <chrono>
#include <thread>
#include <vector>
#include <iostream>
#include <string>
#include <algorithm>
#include "search.h"
#include "evaluation.h"
#include "chess.hpp"
#include "transposition.h"
#include "timeman.h"

class UCI {
    unsigned int wtime = 0;
	unsigned int btime = 0;
	unsigned int winc = 0;
	unsigned int binc = 0;
	unsigned int movestogo = 1;
    chess::Board board;
    std::thread thr;
    public:
        UCI();
        void loop();
        void findMove(int max);
        void timer(int milliseconds);
};

#endif