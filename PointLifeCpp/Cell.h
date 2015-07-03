//
//  Cell.h
//  PointLifeCpp
//
//  Created by Scott SSE on 6/25/15.
//  Copyright (c) 2015 Scott SSE. All rights reserved.
//

#ifndef PointLifeCpp_Cell_h
#define PointLifeCpp_Cell_h

#include "Point.h"
#include "Genome.h"

class Cell {
public:
    Point mPos;
    Point mSavePos;
    Point mVelocity;
    bool mOnBoard;
    Cell *mPrev, *mNext;
    
    int mIndex, mEntityIndex;
    int mLastCollision;
    int mGeneration;
    int mNumConnections;
    int mNumAllowableConnections;
    Cell* mConnections[MAX_CONNECTIONS];
    NUM_TYPE mMaxConnectionLength[MAX_CONNECTIONS];
    Genome * mGenome;
    bool mOwnsConnection[MAX_CONNECTIONS];
    
    int mAction;
    int mPhase;
    NUM_TYPE mStrength;
    
public:
    Cell() {
        reset();
    }
    
    void connect(Cell * toCell) {
        if (mNumConnections < (MAX_CONNECTIONS-1) && toCell->mNumConnections < (MAX_CONNECTIONS-1)) {
            mConnections[mNumConnections++] = toCell;
            toCell->mOwnsConnection[toCell->mNumConnections] = false;
            toCell->mConnections[toCell->mNumConnections++] = this;
        }
    }
    
    void reset() {
        mOnBoard = false;
        mGeneration = 0;
        mPrev = mNext = NULL;
        mLastCollision = 0;
        mNumConnections = 0;
        for (int i = 0; i < MAX_CONNECTIONS; i++) {
            mConnections[i] = NULL;
            mOwnsConnection[i] = true;
        }
    }
    
    void randomize();
};

typedef Cell * CellPtr;

#endif
