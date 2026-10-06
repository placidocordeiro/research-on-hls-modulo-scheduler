#ifndef MII_H
#define MII_H

#include "instance.h"

struct MIIResult {
    int resmii;
    int recmii;
    int mii;
    bool feasible;
    std::string message;
};

MIIResult compute_mii(const Instance& inst);

#endif
