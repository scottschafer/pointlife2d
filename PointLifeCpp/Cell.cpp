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
//    mNumAllowableConnections = (int) MIN(Random::randRange(0, MAX_CONNECTIONS+1), MAX_CONNECTIONS);
 
    mPhase = Random::randRange(10, 200);
    mStrength = Random::randRange(NUMBER(.5), NUMBER(2.0));
    mAction = Random::randRange(0,20);
    
    for (int i = 0; i < MAX_CONNECTIONS; i++) {
        mMaxConnectionLength[i] = Random::randRange(NUMBER(CELL_SIZE), NUMBER(CELL_SIZE*2.0));
    }
}


void Cell :: reset() {
    
    mNumAllConnections = 0;
    mActivated = false;
    
    mLastSentSignal = 0;
    
    mContractTurnCount = 0;
    mNumSecondaryConnections = 0;
    mAction = 0;
    mEnergy = 0;
    mVelocity.x = mVelocity.y = 0;
    mOnBoard = false;
    mGeneration = 0;
    mPrev = mNext = NULL;
    mLastFlagellum = mLastAte = mLastCollision = 0;
    mNumConnections = 0;
//    mNumAllowableConnections = 0;
    mEntityIndex = mIndex;
    for (int i = 0; i < MAX_CONNECTIONS; i++) {
        mConnections[i] = NULL;
        mAllConnections[i] = NULL;
        mOwnsConnection[i] = true;
        mConnectionCellType[i] = 0;
        mConnectionOptions[i] = 0;
    }
}
