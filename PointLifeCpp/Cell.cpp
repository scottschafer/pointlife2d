//  Cell.cpp
//  PointLifeCpp
//
//  Created by Scott SSE on 6/30/15.
//  Copyright (c) 2015 Scott SSE. All rights reserved.
//

#include "Cell.h"
#include "Random.h"

void Cell::randomize() {
    reset();
    
    mPos.randomize();
    mVelocity.randomize(-INITIAL_VELOCITY, INITIAL_VELOCITY);
    mNumAllowableConnections = (int) MIN(Random::randRange(0, MAX_CONNECTIONS+1), MAX_CONNECTIONS);
 
    mPhase = Random::randRange(10, 200);
    mStrength = Random::randRange(NUM_TYPE(.5), NUM_TYPE(2.0));
    mAction = Random::randRange(0,20);
    
    for (int i = 0; i < MAX_CONNECTIONS; i++) {
        mMaxConnectionLength[i] = Random::randRange(NUM_TYPE(CELL_SIZE), NUM_TYPE(CELL_SIZE*2.0));
    }
}