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
#include "constants.h"

enum {
    actionFlagellum = 1,
    actionBite = 2,
    actionLook = 3,
    actionContract = 4
};

enum {
    colorRed = 0,
    colorBlue = 1,
    colorYellow = 2,
    colorNumColors,
    colorAny,
    colorNone
};
/**
 
 Each cell has a color (red, blue, green, yellow, purple)
 Each cell has a number of connections
 Each connection can bind to a color
 Each connection can specify that it binds to a 'child' cell, or the nearest cell of the specified color
 
 **/


class Cell {
public:
    Point mPos;
    Point mSavePos;
    Point mVelocity;

    bool mActivated;
    bool mOnBoard;
    
    Cell *mPrev, *mNext;

    double mEnergy;

    int mIndex, mEntityIndex;
    int mGeneration;
    int mNumConnections;
    int mNumSecondaryConnections;
    int mNumAllowableConnections;
    Cell* mConnections[MAX_CONNECTIONS];
    int mConnectionOptions[MAX_CONNECTIONS];

    
    Cell* mAllConnections[MAX_CONNECTIONS];
    int   mNumAllConnections;
    
    int mConnectionCellType[MAX_CONNECTIONS];
    NUMBER mMaxConnectionLength[MAX_CONNECTIONS];
    Genome * mGenome;
    bool mOwnsConnection[MAX_CONNECTIONS];
    
    int mAction;
    int mPhase;
    NUMBER mStrength;

    int mLastCollision;
    int mLastFlagellum;
    int mLastImpulse;
    int mLastAte;
    int mLastSentSignal;
    int mContractTurnCount;
    
    Point mFlagellumVector;
    
    
public:
    Cell() {
        reset();
    }
    
    void reset();
    
    void connect(Cell * toCell, NUMBER maxConnectionLength = CELL_SIZE, int connectionProperty = 0) {
        
        if (toCell == this) {
            return;
        }
        
        for (int i = 0; i < mNumConnections; i++) {
            if (mConnections[i] == toCell) {
                return;
            }
        }
        
        bool connectedToUs = false;
        for (int i = 0; i < toCell->mNumConnections; i++) {
            if (toCell->mConnections[i] == toCell) {
                connectedToUs = true;
                break;
            }
        }
        
        if (mNumAllConnections < MAX_CONNECTIONS) {
            mAllConnections[mNumAllConnections++] = toCell;
        }
        
        if (connectedToUs) return;
        
        if (mNumConnections < mNumAllowableConnections &&
            toCell->mNumConnections < toCell->mNumAllowableConnections) {
            mMaxConnectionLength[mNumConnections] = maxConnectionLength;
            mConnectionOptions[mNumConnections] = connectionProperty;
            mConnections[mNumConnections++] = toCell;
            toCell->connect(this, maxConnectionLength);
            
//            --toCell->mNumAllowableConnections;
            ++toCell->mNumSecondaryConnections;
        }
    }
    
    bool isConnected(Cell *toCell) {
        if (toCell == this) {
            return true;
        }
        for (int i = 0; i < mNumConnections; i++) {
            if (mConnections[i] == toCell) {
                return true;
            }
        }
        for (int i = 0; i < toCell->mNumConnections; i++) {
            if (toCell->mConnections[i] == this) {
                return true;
            }
        }
        return false;
    }
    
    void randomize();
};

typedef Cell * CellPtr;

#endif
