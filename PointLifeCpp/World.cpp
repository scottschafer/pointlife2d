//
//  World.cpp
//  PointLifeCpp
//
//  Created by Scott SSE on 6/25/15.
//  Copyright (c) 2015 Scott SSE. All rights reserved.
//

#include <stdlib.h>
#include "World.h"
#include "globals.h"
#include "Random.h"
#include "CreatureConstructor.h"

#define NUM_CONNECTION_PHYSICS_ITERATIONS 5
//#undef  NUM_GENOMES_TO_TEST
//#define NUM_GENOMES_TO_TEST 20

Point startCritterPoint;

World::World() {
    Random::init();
    mCells = new Cell[NUM_CELLS];
    
    for (int i = 0; i < NUM_CELLS; i++) {
        mCells[i].mIndex = i;
        mCells[i].mEntityIndex = i;
    }
    
    randomize();
    mWorldSpace.setCells(mCells);
}

void World :: reset() {
    mNumCells = 0;
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
        mWorldSpace.insert(&cell);
    }
    mNumCells = NUM_CELLS;
}

#define MAX_VELOCITY (CELL_SIZE*.4)

void World :: applyPointPhysicsAndConnections(NUMBER fraction) {
    int i;
    Cell * pCell = mCells;
    for (i = 0; i < mNumCells; i++) {
        if (pCell->mOnBoard) {

            NUMBER velocityLen = pCell->mVelocity.fastLength();
            if (velocityLen > MAX_VELOCITY) {
                NUMBER scale = MAX_VELOCITY / velocityLen;
                pCell->mVelocity.x *= scale;
                pCell->mVelocity.y *= scale;
            }
            
            NUMBER x = pCell->mPos.x + pCell->mVelocity.x * fraction;
            NUMBER y = pCell->mPos.y + pCell->mVelocity.y * fraction;
            
            if (x < 0 || x >= WORLD_DIM) {
                pCell->mVelocity.x = - pCell->mVelocity.x * fraction;
                x = pCell->mPos.x;
            }
            
            if (y < 0 || y >= WORLD_DIM) {
                pCell->mVelocity.y = - pCell->mVelocity.y  * fraction;
                y = pCell->mPos.y;
            }

            for (int j = 0; j < pCell->mNumConnections; j++) {
                Cell * pToCell = pCell->mConnections[j];
                if (! pToCell || ! pToCell->mOnBoard) {
                    continue;
                }

                NUMBER x1 = pCell->mPos.x;
                NUMBER y1 = pCell->mPos.y;
                NUMBER x2 = pToCell->mPos.x;
                NUMBER y2 = pToCell->mPos.y;
                
                NUMBER xd = x1 - x2;
                NUMBER yd = y1 - y2;
                
                NUMBER distance = accurateDistance(xd, yd);
                if (distance == 0) { continue; }
                
                NUMBER minDist = CELL_SIZE;
                NUMBER maxDist = pCell->mMaxConnectionLength[j];
                
                if (pCell->mContractTurnCount) {
                    maxDist /= 2.0;
                }
                
                if (maxDist < minDist) {
                    maxDist = minDist;
                }
                NUMBER targestDistance;
                if (distance < minDist) {
                    targestDistance = minDist;
                }
                else if (distance > maxDist) {
                    targestDistance = maxDist;
                }
                else continue;

                NUMBER fractionAdjust = (targestDistance - distance) / distance;
                fractionAdjust /= 2.0;

                x1 += xd * fractionAdjust;
                y1 += yd * fractionAdjust;

                x2 -= xd * fractionAdjust;
                y2 -= yd * fractionAdjust;

                x = MIN(MAX(x1, 0),WORLD_DIM-1);
                y = MIN(MAX(y1, 0),WORLD_DIM-1);
                
                x2 = MIN(MAX(x2, 0),WORLD_DIM-1);
                y2 = MIN(MAX(y2, 0),WORLD_DIM-1);
                
                pCell->mVelocity.x += x1 - pCell->mPos.x;
                pCell->mVelocity.y += y1 - pCell->mPos.y;
                
                pToCell->mVelocity.x += x2 - pToCell->mPos.x;
                pToCell->mVelocity.y += y2 - pToCell->mPos.y;

//                mWorldSpace.move(pCell, x1, y1);
                mWorldSpace.move(pToCell, x2, y2);
            }

            mWorldSpace.move(pCell, x, y);
        }
        ++pCell;
    }
}

void World :: applyPointPhysics(NUMBER fraction) {
    int i;
    Cell * pCell = mCells;
    for (i = 0; i < mNumCells; i++) {
        if (pCell->mOnBoard) {

            pCell->mSavePos.x = pCell->mPos.x;
            pCell->mSavePos.y = pCell->mPos.y;

            NUMBER vx = pCell->mVelocity.x;
            NUMBER vy = pCell->mVelocity.y;

            NUMBER velocityLen = pCell->mVelocity.fastLength();
            if (velocityLen > MAX_VELOCITY) {
                NUMBER scale = MAX_VELOCITY / velocityLen;
                fraction *= scale;
            }
            vx *= fraction;
            vy *= fraction;
            
            NUMBER x = pCell->mPos.x + vx;
            NUMBER y = pCell->mPos.y + vy;
            
            if (x < 0 || x >= WORLD_DIM) {
                pCell->mVelocity.x = - vx;
                x = pCell->mPos.x;
            }
            
            if (y < 0 || y >= WORLD_DIM) {
                pCell->mVelocity.y = - vy;
                y = pCell->mPos.y;
            }
            
    #if BATCH_MOVE_POINTS
            pCell->mPos.x = x;
            pCell->mPos.y = y;
    #else
            mWorldSpace.move(pCell, x, y);
    #endif
        }
        ++pCell;
    }
}

void World :: applyNewPositions() {
#if BATCH_MOVE_POINTS
    Cell * pCell = mCells;

    for (int i = 0; i < mNumCells; i++) {
        mWorldSpace.move(pCell, pCell->mSavePos.x, pCell->mSavePos.y, pCell->mPos.x, pCell->mPos.y);
        
        ++pCell;
    }
#endif
}

void World :: applyConnectionPhysics(NUMBER fraction) {
    Cell * pCell = mCells;
    for (int i = 0; i < mNumCells; i++) {
        if (! pCell->mOnBoard) {
            // TODO: break connections
            ++pCell;
            continue;
        }
        for (int j = 0; j < pCell->mNumConnections; j++) {
            Cell * pToCell = pCell->mConnections[j];
            if (! pToCell) {
                continue;
            }
            
            NUMBER x1 = pCell->mPos.x;
            NUMBER y1 = pCell->mPos.y;
            NUMBER x2 = pToCell->mPos.x;
            NUMBER y2 = pToCell->mPos.y;
            
            NUMBER xd = x1 - x2;
            NUMBER yd = y1 - y2;
            NUMBER distance = accurateDistance(xd, yd);
            if (distance == 0) { continue; }
            
            NUMBER minDist = CELL_SIZE;
            NUMBER maxDist = pCell->mMaxConnectionLength[j];
            
            if (pCell->mContractTurnCount) {
                maxDist /= 2.0;
            }
            
            if (maxDist < minDist) {
                maxDist = minDist;
            }
            NUMBER targestDistance;
            if (distance < minDist) {
                targestDistance = minDist;
            }
            else if (distance > maxDist) {
                targestDistance = maxDist;
            }
            else continue;

            // targetDistance = 60, distance = 30
            // adjust by (60 - 30) / 30 = 1 = 100% increase in length

            // targetDistance = 10, distance = 40
            // adjust by (10 - 40) / 40 = -.75 = 75% decrease in length

            NUMBER fractionAdjust = (targestDistance - distance) / distance;
            fractionAdjust /= 2.0;
            x1 += xd * fractionAdjust;
            x2 -= xd * fractionAdjust;
            y1 += yd * fractionAdjust;
            y2 -= yd * fractionAdjust;
            
            // targetDistance = 30, distance = 60
            // adjust by 60 / 30 = 2
            
            /*
            NUMBER adjFactor = (targestDistance - distance)
            NUMBER adjX =
            
            
            //            NUMBER adjustLength = NUMBER(CELL_SIZE) - distance;
            NUMBER adjustFactor = (NUMBER(targestDistance) / distance) / 2;
            
            if (distance < (CELL_SIZE/100000))
                continue;
            
            NUMBER xc = (x1 + x2) / 2;
            NUMBER yc = (y1 + y2) / 2;
            
            x1 = xc + xd * adjustFactor;
            y1 = yc + yd * adjustFactor;
            x2 = xc - xd * adjustFactor;
            y2 = yc - yd * adjustFactor;
*/

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

#if BATCH_MOVE_POINTS
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

void World :: sendSignal(Cell * pCell, int signal, int level) {
    if (pCell->mLastSentSignal) {
        return;
    }
    pCell->mLastSentSignal = 50;
    
    ++level;
    if (level > 25)
        return;

    for (int i = 0; i < pCell->mNumConnections; i++) {
        Cell * pToCell = pCell->mConnections[i];
        if (pCell->mConnectionOptions[i] & signal) {
            pToCell->mActivated = true;
            if (pToCell->mAction == actionFlagellum) {
                pToCell->mPhase = 2 + level * 5;
                return;
            }
            sendSignal(pToCell, signal, level + 1);
        }
    }
}

/**
 * todo, maybe just check cells in each square
 */
void World :: detectCollisions(NUMBER fraction) {
    
    static int numGeneration = 0;
    
    NUMBER collisionDistance = CELL_SIZE;
    CellPtr results[3];
    int collisionLength = 10;
    
    int i;
    Cell * pCell = mCells;
    ++numGeneration;
    for (i = 0; i < mNumCells; i++) {
        
        if (! pCell->mOnBoard)
            continue;
        
        pCell->mGeneration = numGeneration;

        CellPtr *pResultArray = results;

        int numResults =
        mWorldSpace.getNearbyCells(numGeneration, pCell->mPos, collisionDistance, pResultArray, (int) sizeof(results)/sizeof(results[0]));
        
        if (numResults) {

            sendSignal(pCell, 2);

            for (int j = 0; j < numResults; j++) {
                Cell * pToCell = results[j];
//                if (pToCell->mEntityIndex == pCell->mEntityIndex)
//                    continue; // for now
                
                if (/*pCell->mNumConnections == 2 && */
                    pCell->mAction == actionBite && pToCell->mEntityIndex != pCell->mEntityIndex) {
                    pCell->mEnergy += 10;
                    --pToCell->mEnergy;
                    sendSignal(pCell, 1);
                    if (pToCell->mEnergy < -5) {
                        mWorldSpace.remove(pToCell);
                        continue;
                    }
                    pCell->mLastAte = 1000;
                }
                
                if (pCell->isConnected(pToCell)) {
                    continue;
                }
                NUMBER vx1 = pToCell->mVelocity.x;
                NUMBER vy1 = pToCell->mVelocity.y;
                NUMBER vx2 = pCell->mVelocity.x;
                NUMBER vy2 = pCell->mVelocity.y;

                if (pToCell->mEntityIndex == pCell->mEntityIndex) {
                    NUMBER xd = pCell->mPos.x - pToCell->mPos.x;
                    NUMBER yd = pCell->mPos.y - pToCell->mPos.y;
                    NUMBER dist = sqrt(xd*xd+yd*yd);
                    if (dist < CELL_SIZE) {
                        NUMBER fraction = CELL_SIZE - dist;
                        xd *= fraction * 1.0;
                        yd *= fraction * 1.0;
                        vx1 += xd;
                        vy1 += yd;
                        vx2 -= xd;
                        vy2 -= yd;
                    }
//                NUMBER distSquared = xd * xd + yd * yd;
//                NUMBER mult = CELL_SIZE * CELL_SIZE / distSquared;
                    NUMBER mult = 1;//CELL_SIZE / MAX(ABS(xd), ABS(yd));
//                if (mult > 20)
///                    mult = 20.0;
                    
                    /*
                    double damp = .1;
                    vx1 *= damp;
                    vx2 *= damp;
                    vy1 *= damp;
                    vy2 *= damp;
                     */
                }
                
                pCell->mVelocity.x = vx1;
                pCell->mVelocity.y = vy1;
                pToCell->mVelocity.x = vx2;
                pToCell->mVelocity.y = vy2;
                
#if BATCH_MOVE_POINTS
                double f = .2;
                NUMBER x1 = MIN(MAX(pCell->mPos.x + pCell->mVelocity.x * f, 0),WORLD_DIM-1);
                NUMBER y1 = MIN(MAX(pCell->mPos.y + pCell->mVelocity.y * f, 0),WORLD_DIM-1);
                NUMBER x2 = MIN(MAX(pToCell->mPos.x + pToCell->mVelocity.x * f, 0),WORLD_DIM-1);
                NUMBER y2 = MIN(MAX(pToCell->mPos.y + pToCell->mVelocity.y * f, 0),WORLD_DIM-1);

                pCell->mPos.x = x1;//+= pCell->mVelocity.x;
                pCell->mPos.y = y1;//+= pCell->mVelocity.y;
                pToCell->mPos.x = x2;//+= pToCell->mVelocity.x;
                pToCell->mPos.y = y2;//+= pToCell->mVelocity.y;
#endif
                    /*
                    mWorldSpace.move(pCell, pCell->mPos.x + pCell->mVelocity.x, pCell->mPos.y + pCell->mVelocity.y);
                    mWorldSpace.move(pToCell, pToCell->mPos.x + pToCell->mVelocity.x,
                                     pToCell->mPos.y + pToCell->mVelocity.y);
                     */
            }
            
            
            pCell->mLastCollision = collisionLength;
            for (int j = 0; j < numResults; j++) {
                results[j]->mLastCollision = collisionLength;
            }
        }
        
        ++pCell;
    }
}

void enableMove(Cell *pCell, int * pTimeout) {
    if (*pTimeout > 0) {
        for (int i = 0; i < pCell->mNumConnections; i++) {
            Cell * pConnectedCell = pCell->mConnections[i];
            
//        for (int i = 0; i < pCell->mNumConnections; i++) {
//            Cell * pConnectedCell = pCell->mConnections[i];
            if (pConnectedCell && pConnectedCell->mAction == actionFlagellum) {
                pConnectedCell->mPhase = 2;
                *pTimeout = 0;
                return;
            }
            -- *pTimeout;
            enableMove(pConnectedCell,pTimeout);
        }
    }
}

void World :: processCellActivity() {
    Cell * pCell = mCells;
    static int turn = 0;
    ++turn;
    

    CellPtr results[5];
    
    
    for (int i = 0; i < NUM_CELLS; i++) {
        int numConnections = pCell->mNumConnections;
        int totalConnections = numConnections + pCell->mNumSecondaryConnections;
        
        int action = pCell->mAction;
        

        if (/*numConnections == 2 &&  */ (turn % pCell->mPhase) == 0) {
            
            /*if (action == actionContract) {
                pCell->mContractTurnCount = 3;
            }
            else */
            if ((pCell->mNumAllConnections <= 3) && (action == actionFlagellum || action == actionLook)) {
                NUMBER xd = 0, yd = 0;
                for (int j = 0; j < pCell->mNumAllConnections; j++) {
                    xd += pCell->mPos.x - pCell->mAllConnections[j]->mPos.x;
                    yd += pCell->mPos.y - pCell->mAllConnections[j]->mPos.y;
                }
                NUMBER length = pCell->mNumAllConnections + 2;
//                NUMBER length = Point(xd,yd).distance();
//                if (length <= 0)  continue;

                pCell->mPhase = 10000;
                Point v(CELL_SIZE * xd / length, CELL_SIZE * yd / length);

                switch (action) {
                    case actionFlagellum: {
                        
                    if (1 <= mWorldSpace.getNearbyCells(Point(pCell->mPos.x - v.x, pCell->mPos.y - v.y),
                                                        CELL_SIZE, results, (int) sizeof(results)/sizeof(results[0])))
                    {
                        pCell->mFlagellumVector = Point(v.x * 50, v.y * 50);
                     //   pCell->mVelocity.x -= v.x;
                     //   pCell->mVelocity.y -= v.y;
                        pCell->mLastFlagellum = 100;
                        pCell->mEnergy -= 5;
                    }
                    else {
//                        pCell->mAction = 0;
                    }
                        
                    break; }

                    case actionLook: {
                        NUMBER x = pCell->mPos.x, y = pCell->mPos.y;
                        for (int j = 0; j < 10; j++) {
                            x += v.x * 2;
                            y += v.y * 2;

                            int numCells =
                            mWorldSpace.getNearbyCells(pCell, CELL_SIZE * 2, results, (int) sizeof(results)/sizeof(results[0]));
                            
                            for (int k = 0; k < numCells; k++) {
                                if (results[k]->mEntityIndex != pCell->mEntityIndex) {
                                    int timeout = 200;
                                    enableMove(pCell, &timeout);
                                    /*
                                    for (int l = 0; l < pCell->mNumConnections; l++) {
                                        pCell->mConnections[l]->mPhase = 2;
                                    }*/
                                    break;
                                }
                            }
                            
                        }
                        break; }
                }
            }
        }
        
        /*
        if ((numConnections == 2) && pCell->mAction == actionFlagellum && ((turn % pCell->mPhase) == 0)) {
            NUMBER xd = 0, yd = 0;
            for (int j = 0; j < numConnections; j++) {
                xd += pCell->mPos.x - pCell->mConnections[j]->mPos.x;
                yd += pCell->mPos.y - pCell->mConnections[j]->mPos.y;
            }
            Point v(xd, yd);
            NUMBER len = v.distance();
            if (len > 0) {
                v.x *= CELL_SIZE / len;
                v.y *= CELL_SIZE / len;
            }
        }
         */
        ++pCell;
    }
}

class GenomeWithScore {
public:
    GenomeWithScore() { score = 0; }
    Genome genome;
    double score;
    
    static int compareFunc(const void * a, const void * b) {
        return ((GenomeWithScore*)b)->score - ((GenomeWithScore*)a)->score;
    }
};

static GenomeWithScore genepool[NUM_GENOMES_TO_TEST];

void World :: applyFluidMechanics() {
    Cell * pCell = mCells;
    for (int i = 0; i < mNumCells; i++) {
        if (pCell->mOnBoard && pCell->mNumConnections == 2) {
        NUMBER vx = pCell->mPos.x - pCell->mSavePos.x;
        NUMBER vy = pCell->mPos.y - pCell->mSavePos.y;
        
        pCell->mVelocity.x -= vx * 20.0;
        pCell->mVelocity.y -= vy * 20.0;
        }
        ++pCell;
    }
}

void World :: turnCrank() {
        
    if (mInFitnessTest != Globals::inFitnessTest) {
        if (Globals::inFitnessTest) {
            beginFitnessTest();
        }
        else {
            endFitnessTest();
        }
    }
    
    if (mInFitnessTest && mFitnessTestTurn == 0) {
        prepareGenomeForTest();
    }
    
    Cell * pCell = mCells;
    NUMBER vDamper = .998;
    
    for (int i = 0; i < mNumCells; i++) {
        pCell->mVelocity.x *= vDamper;
        pCell->mVelocity.y *= vDamper;
        if (pCell->mContractTurnCount) {
            --pCell->mContractTurnCount;
        }
        if (pCell->mLastSentSignal) {
            -- pCell->mLastSentSignal;
        }
        if (pCell->mLastFlagellum) {
            --pCell->mLastFlagellum;
            
            NUMBER xd = 0, yd = 0;
            for (int j = 0; j < pCell->mNumAllConnections; j++) {
                xd += pCell->mPos.x - pCell->mAllConnections[j]->mPos.x;
                yd += pCell->mPos.y - pCell->mAllConnections[j]->mPos.y;
            }
            NUMBER length = pCell->mNumAllConnections + 25.0;
            Point v(CELL_SIZE * xd / length, CELL_SIZE * yd / length);
            pCell->mFlagellumVector = Point(v.x * 100, v.y * 100);
            pCell->mVelocity.x -= v.x;
            pCell->mVelocity.y -= v.y;
            
        }
        else
            if (pCell->mLastCollision) {
                --pCell->mLastCollision;
            }
            else
                if (pCell->mLastAte) {
                    --pCell->mLastAte;
                }
#if BATCH_MOVE_POINTS
        pCell->mSavePos.x = pCell->mPos.x;
        pCell->mSavePos.y = pCell->mPos.y;
#endif
        ++pCell;
    }
    
#if 0
    applyPointPhysicsAndConnections();
    //detectCollisions();

#else
    
#if 0
    detectCollisions();
    applyConnectionPhysics(1);
#else
    for (int i = 0; i < NUM_CONNECTION_PHYSICS_ITERATIONS; i++) {
        double f = NUMBER(i+1) / NUMBER(NUM_CONNECTION_PHYSICS_ITERATIONS);
        detectCollisions(f);
        applyPointPhysics(f);
        applyConnectionPhysics(f);
    }
#endif
#endif
    
#if BATCH_MOVE_POINTS
    applyNewPositions();
#endif
    processCellActivity();
    //applyFluidMechanics();
    
    if (mInFitnessTest) {
        ++mFitnessTestTurn;
        
        // if we haven't moved, skip testing
        if (mFitnessTestTurn > 10 && startCritterPoint.x == mCells[0].mPos.x && startCritterPoint.y == mCells[0].mPos.y) {
            mFitnessTestTurn = NUM_TURNS_PER_GENOME;
        }

        //
        if (mFitnessTestTurn == NUM_TURNS_PER_GENOME) {
            genepool[mFitnessTestIndex].score = calcFitness();
            mFitnessTestTurn = 0;
            ++mFitnessTestIndex;
            if (mFitnessTestIndex == NUM_GENOMES_TO_TEST) {
                ++mFitnessGeneration;
                prepareGenerationForTest();
            }
        }
    }
}

Cell * World :: addCell(NUMBER x, NUMBER y) {
    Cell * result = NULL;
    if (mNumCells < (NUM_CELLS-1)) {
        result = &mCells[mNumCells++];
        result->reset();
        x = MAX(0, MIN(x, MAX_COORD));
        y = MAX(0, MIN(y, MAX_COORD));
        result->mPos.x = x;
        result->mPos.y = y;
        mWorldSpace.insert(result);
    }
    return result;;
}


void World :: beginFitnessTest() {
    reset();
    
    mFitnessGeneration = 0;
    mFitnessTestIndex = 0;
    mFitnessTestTurn = 0;
    
    prepareGenerationForTest();
    
    mInFitnessTest = true;
}

void World :: endFitnessTest() {
    mInFitnessTest = false;
    randomize();
}

NUMBER calcDistance(Cell * pCell) {
    NUMBER result = 0;
    
    Point p(WORLD_DIM/2, WORLD_DIM/2);
    result += pCell->mPos.distance(p);
    
    /*
     for (int i = 0; i < pCell->mNumConnections; i++) {
     
     NUMBER dist = calcDistance(pCell->mConnections[i]);
     if (i)
     dist /= 4;
     result += dist;
     }
     */
    return result;
}

NUMBER calcEnergy(Cell * pCell) {
    NUMBER result = 0;
    
    result += pCell->mEnergy - 1.0;
    
     for (int i = 0; i < pCell->mNumConnections; i++) {
     
        result += calcEnergy(pCell->mConnections[i]);
     }
    return result;
}

double World :: calcFitness() {
    if (mNumCells == 0)
        return 0;
    
    int entityIndex = mCells[0].mEntityIndex;
    
    int ate = 0;
    
    NUMBER result = 0;
    for (int i = 0; i < NUM_CELLS; i++) {
        if (mCells[i].mOnBoard && mCells[i].mEntityIndex == entityIndex) {
            
            if (mCells[i].mEnergy > 0)
                ++ate;
            switch (mCells[i].mAction) {
                case actionBite:
                    result -= 2;
                    break;
                case actionFlagellum:
//                    result -= 1;
                    break;
                default:
//                    result -= 1;
                    break;
            }
            result += mCells[i].mEnergy;
        }
    }
    
    if (ate == 0)
        result = -1000;
//    result = calcEnergy(&mCells[0]);
//    NUMBER result = calcDistance(&mCells[0]);
    return result;
}

void World :: prepareGenomeForTest() {
    reset();

    // create the critter
    Genome & genome = genepool[mFitnessTestIndex].genome;
    CreatureConstructor constructor(*this, genome);
    constructor.go();
    startCritterPoint = mCells[0].mPos;
    
    // create some food

    if (false && (mFitnessGeneration % 2)) {
        for (int i = 0; i < 200; i++) {
            
            NUMBER lb = .4 * WORLD_DIM;
            NUMBER ub = .6 * WORLD_DIM;
            
            NUMBER x = Random::randRange(lb, ub);
            NUMBER y = Random::randRange(lb, ub);
            Cell * pNewCell = addCell(x,y);
        }
    }
    else {
        NUMBER radius = NUMBER(WORLD_DIM) / NUMBER(90 - (mFitnessGeneration % 50));
        NUMBER angle = NUMBER(mFitnessGeneration % 77) / 10.0;

        radius = NUMBER(WORLD_DIM) / NUMBER(50);
        angle = 0;
        
        for (int i = 0; i < 200; i++) {
            NUMBER x = WORLD_DIM / 2 + cos(angle) * radius;
            NUMBER y = WORLD_DIM / 2 + sin(angle) * radius;
            
            angle += 6.28 / 80.0;
            radius *= 1.007f;
            
            Cell * pNewCell = addCell(x,y);
        }
    }
}

int randomIndex() {
    double r = Random::rand();
    r *= r;
    return int(r * NUM_GENOMES_TO_TEST);
}

void World :: prepareGenerationForTest() {
    int i;
    
    mFitnessTestIndex = 0;
    if (mFitnessGeneration == 0) {
        for (i = 0; i < NUM_GENOMES_TO_TEST; i++) {
            genepool[i].genome.randomize();
        }
    }
    else {
        // sort genepool by fitness function
        qsort(genepool, NUM_GENOMES_TO_TEST, sizeof(GenomeWithScore), GenomeWithScore::compareFunc);

        mFitnessTestIndex = NUM_GENOMES_TO_TEST / 10; // don't bother testing the top 10% again

        int j = 4 * NUM_GENOMES_TO_TEST / 10;
        for ( ; j < NUM_GENOMES_TO_TEST; j++) {
            int g1 = randomIndex();
            int g2 = randomIndex();
            genepool[j].genome = genepool[g1].genome.breed(genepool[g2].genome);
        }
        
        
        // create mutant forms of the top critters
        j = NUM_GENOMES_TO_TEST / 10;
        for (i = 0; i < NUM_GENOMES_TO_TEST / 10; i++) {
            for (int n = 0; n < 3; n++) {
                genepool[j].genome = genepool[i].genome;
                genepool[j].genome.mutate();
                ++j;
            }
        }
        
    }
    
    for (i = 0; i < NUM_GENOMES_TO_TEST; i++) {
        genepool[i].score = 0;
    }
}

int World :: getGeneration() {
    return mInFitnessTest ? (mFitnessGeneration + 1) : 0;
}

int World :: getTestIndex () {
    return mInFitnessTest ? (mFitnessTestIndex + 1) : 0;
}


