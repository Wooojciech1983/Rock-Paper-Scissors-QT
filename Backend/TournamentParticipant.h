#pragma once
#include "IBotStrategy.h"
#include <memory>
#include <string>

struct TournamentParticipant {
    std::string name;
    bool isHuman = false;
    std::shared_ptr<IBotStrategy> strategy; // null for human
};
