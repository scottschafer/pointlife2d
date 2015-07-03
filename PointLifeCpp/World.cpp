//
//  World.cpp
//  PointLifeCpp
//
//  Created by Scott SSE on 6/25/15.
//  Copyright (c) 2015 Scott SSE. All rights reserved.
//

#include "World.h"
#include "Random.h"

#define CHECK_COLLISIONS_ONE_TIME 1

World::World() {
    Random::init();
    mCells = new Cell[NUM_CELLS];
    
    for (int i = 0; i < NUM_CELLS; i++) {
        mCells[i].mIndex = i;
        mCells[i].mEntityIndex = i;
    }
    
    randomize();
}

void World :: reset() {
    for (int i = 0; i < NUM_CELLS; i++) {
        if (mCells[i].mOnBoard) {
            mWorldSpace.remove(&mCells[i]);
        }
        mCells[i].mEntityIndex = i;
    }
}

void World :: randomize()
{
    reset();
    
    int i;
    for (i = 0; i < NUM_CELLS; i++) {
        Cell & cell = mCells[i];
        cell.randomize();
    }
}

void World :: applyPointPhysics() {
    int i;
    Cell * pCell = mCells;
    for (i = 0; i < NUM_CELLS; i++) {

        NUM_TYPE velocityLen = pCell->mVelocity.distance();
        if (velocityLen > MAX_VELOCITY) {
            NUM_TYPE scale = MAX_VELOCITY / velocityLen;
            pCell->mVelocity.x *= scale;
            pCell->mVelocity.y *= scale;
        }
        
#if CHECK_COLLISIONS_ONE_TIME
        pCell->mSavePos.x = pCell->mPos.x;
        pCell->mSavePos.y = pCell->mPos.y;
#endif
        
        NUM_TYPE x = pCell->mPos.x + pCell->mVelocity.x;
        NUM_TYPE y = pCell->mPos.y + pCell->mVelocity.y;
        
        if (x < 0 || x >= WORLD_DIM) {
            pCell->mVelocity.x = - pCell->mVelocity.x;
            x = pCell->mPos.x;
        }
        
        if (y < 0 || y >= WORLD_DIM) {
            pCell->mVelocity.y = - pCell->mVelocity.y;
            y = pCell->mPos.y;
        }
        
#if CHECK_COLLISIONS_ONE_TIME
        pCell->mPos.x = x;
        pCell->mPos.y = y;
#else
        mWorldSpace.move(pCell, x, y);
#endif
        ++pCell;
//        ++pPoint;
    }
}

void World :: applyNewPositions() {
    Cell * pCell = mCells;
//    Point *pPoint = mNewCellPoints;

    for (int i = 0; i < NUM_CELLS; i++) {
        mWorldSpace.move(pCell, pCell->mSavePos.x, pCell->mSavePos.y, pCell->mPos.x, pCell->mPos.y);
        
        ++pCell;
//        ++pPoint;
    }
}

void World :: applyConnectionPhysics() {
    Cell * pCell = mCells;
    for (int i = 0; i < NUM_CELLS; i++) {
        for (int j = 0; j < pCell->mNumConnections; j++) {
            if (! pCell->mOwnsConnection[j]) {
                continue;
            }
            Cell * pToCell = pCell->mConnections[j];
            
            NUM_TYPE x1 = pCell->mPos.x;
            NUM_TYPE y1 = pCell->mPos.y;
            NUM_TYPE x2 = pToCell->mPos.x;
            NUM_TYPE y2 = pToCell->mPos.y;
            
            NUM_TYPE xd = x1 - x2;
            NUM_TYPE yd = y1 - y2;
            NUM_TYPE distance = accurateDistance(xd, yd);
            NUM_TYPE minDist = CELL_SIZE;
            NUM_TYPE maxDist = pCell->mMaxConnectionLength[j];
            
            if (maxDist < minDist) {
                maxDist = minDist;
            }
            NUM_TYPE targestDistance;
            if (distance < minDist) {
                targestDistance = minDist;
            }
            else if (distance > maxDist) {
                targestDistance = maxDist;
            }
            else continue;

            //            NUM_TYPE adjustLength = NUM_TYPE(CELL_SIZE) - distance;
            NUM_TYPE adjustFactor = (NUM_TYPE(targestDistance) / distance) / 2;
            
            NUM_TYPE xc = (x1 + x2) / 2;
            NUM_TYPE yc = (y1 + y2) / 2;
            
            x1 = xc + xd * adjustFactor;
            y1 = yc + yd * adjustFactor;
            x2 = xc - xd * adjustFactor;
            y2 = yc - yd * adjustFactor;

//            NUM_TYPE distance2 = accurateDistance(x1, y1, x2, y2);

            //x1 -= xd * adjustFactor;
            //y1 -= yd * adjustFactor;
            //x2 += xd * adjustFactor;
            //y2 += yd * adjustFactor;

            
            x1 = MIN(MAX(x1, 0),WORLD_DIM-1);
            y1 = MIN(MAX(y1, 0),WORLD_DIM-1);
            x2 = MIN(MAX(x2, 0),WORLD_DIM-1);
            y2 = MIN(MAX(y2, 0),WORLD_DIM-1);
            
            pCell->mVelocity.x += x1 - pCell->mPos.x;
            pCell->mVelocity.y += y1 - pCell->mPos.y;
            pToCell->mVelocity.x += x2 - pToCell->mPos.x;
            pToCell->mVelocity.y += y2 - pToCell->mPos.y;

#if CHECK_COLLISIONS_ONE_TIME
            pCell->mPos.x = x1;
            pCell->mPos.y = y1;
            pToCell->mPos.x = x2;
            pToCell->mPos.y = y2;
#else
            mWorldSpace.move(pCell, x1, y1);
            mWorldSpace.move(pToCell, x2, y2);
#endif


        }
        ++pCell;
    }
}

/**
 * todo, maybe just check cells in each square
 */
void World :: detectCollisions() {
    
    static int numGeneration = 0;
    ++numGeneration;
    
    NUM_TYPE collisionDistance = CELL_SIZE;
    CellPtr results[1];
    int collisionLength = 100;
    
    int i;
    Cell * pCell = mCells;
    for (i = 0; i < NUM_CELLS; i++) {
        
        pCell->mGeneration = numGeneration;
        
        if (pCell->mLastCollision) {
            --pCell->mLastCollision;
        }

        CellPtr *pResultArray = results;

        int numResults =
        mWorldSpace.getNearbyCells(numGeneration, pCell->mPos, collisionDistance, pResultArray, (int) sizeof(results)/sizeof(results[0]));
        
        if (numResults) {
            for (int j = 0; j < numResults; j++) {
                Cell * pToCell = results[j];
                if (pToCell->mEntityIndex == pCell->mEntityIndex) {
//                    continue;
                }
                
                bool isConnected = false;
                for (int k = 0; k < pCell->mNumConnections; k++) {
                    if (pCell->mConnections[k] == pToCell) {
                        isConnected = true;
                        break;
                    }
                }
                
//                if (isConnected) { continue; }
                
                if (pCell->mNumConnections < pCell->mNumAllowableConnections &&
                    pToCell->mNumConnections < pToCell->mNumAllowableConnections) {
                    pCell->connect(pToCell);
                    pToCell->mEntityIndex = pCell->mEntityIndex;
                }
                else {
                    NUM_TYPE ovx = pCell->mVelocity.x;
                    NUM_TYPE ovy = pCell->mVelocity.y;
                    
                    pCell->mVelocity.x = pToCell->mVelocity.x;
                    pCell->mVelocity.y = pToCell->mVelocity.y;
                    pToCell->mVelocity.x = ovx;
                    pToCell->mVelocity.y = ovy;
                }
            }
            
            
            pCell->mLastCollision = collisionLength;
            for (int j = 0; j < numResults; j++) {
                results[j]->mLastCollision = collisionLength;
            }
        }
        
        ++pCell;
    }
}

void World :: processCellActivity() {
    Cell * pCell = mCells;
    static int turn = 0;
    ++turn;
    for (int i = 0; i < NUM_CELLS; i++) {
        int numConnections = pCell->mNumConnections;
        if ((numConnections == 2) && pCell->mAction == 0 && ((turn % pCell->mPhase) == 0)) {
            NUM_TYPE xd = 0, yd = 0;
            for (int j = 0; j < numConnections; j++) {
                xd += pCell->mPos.x - pCell->mConnections[j]->mPos.x;
                yd += pCell->mPos.y - pCell->mConnections[j]->mPos.y;
            }
            NUM_TYPE div = NUM_TYPE(numConnections) * 2;
            pCell->mVelocity.x += xd / div;
            pCell->mVelocity.y += yd / div;
        }
        ++pCell;
    }
}

void World :: turnCrank() {
    detectCollisions();
    applyPointPhysics();
    for (int i = 0; i < 2; i++) {
        applyConnectionPhysics();
    }
#if CHECK_COLLISIONS_ONE_TIME
    applyNewPositions();
#endif
//    processCellActivity();
}

