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
//25
#define CONTRACT_TURN_COUNT 100
#define INACTIVE_AFTER_BITE_COUNT 20
//50
#define VELOCITY_DAMPING 1

const NUMBER TURN_COST = 0.0001;
const NUMBER BITE_GAIN = 6;
const NUMBER BITE_COST = 6;
const NUMBER MOVE_ENERGY = .002;
const NUMBER FLAGELLUM_TURNS = 20;

#define MAX_VELOCITY (CELL_SIZE*.4)


//#undef  NUM_GENOMES_TO_TEST
//#define NUM_GENOMES_TO_TEST 20

Point startCritterPoint;

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


World::World() {
    Random::init();
    mCells = new Cell[NUM_CELLS];
    
    for (int i = 0; i < NUM_CELLS; i++) {
        mCells[i].mIndex = i;
        mCells[i].mEntityIndex = i;
    }
    
    mWorldSpace.setCells(mCells);
    endFitnessTest();
    
    mWorldTurn = 0;
    mNumEntities = 0;
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
}

void World::spawn() {
    
    if (mNumEntities < 100) {
        if (mNumEntities < 80) {
            Genome g;
            g.randomize();
            generateNewEntity(g);
        }
        else {
            Genome g(*getTopGenome());
            int numMutations = Random::randRange(0, 10);
            for (int i = 0; i < numMutations; i++) {
                g.mutate();
            }
            generateNewEntity(g);
        }
    }
    
}


void World :: applyPointPhysics(NUMBER fraction) {
    NUMBER wallDamper = .3;
    
    int i;
    Cell * pCell = mCells;
    for (i = 0; i < NUM_CELLS; i++) {
        if (pCell->mOnBoard) {

            NUMBER vx = pCell->mVelocity.x * fraction;
            NUMBER vy = pCell->mVelocity.y * fraction;

            if (pCell->mNumConnections) {
                double squaredDist = vx * vx + vy * vy;
                if (squaredDist > MAX_VELOCITY * MAX_VELOCITY) {
                    double dist = sqrt(squaredDist);
                    double scale = MAX_VELOCITY / dist;
                    vx *= scale;
                    vy *= scale;
                }
            }

            NUMBER x = pCell->mPos.x + vx;
            NUMBER y = pCell->mPos.y + vy;
            
            NUMBER threshhold = 1;
            
            if (x < threshhold) {
                if (pCell->mVelocity.x < 0) {
                    pCell->mVelocity.x = -pCell->mVelocity.x * wallDamper;
                }
                
                if (x < 0) {
                    x = 0;
                }
            }
            if (x > (WORLD_DIM - threshhold)) {
                if (pCell->mVelocity.x > 0) {
                    pCell->mVelocity.x = -pCell->mVelocity.x * wallDamper;
                }
                
                if (x > WORLD_DIM) {
                    x = WORLD_DIM + pCell->mVelocity.x;
                }
            }
            
            if (y < threshhold) {
                if (pCell->mVelocity.y < 0) {
                    pCell->mVelocity.y = -pCell->mVelocity.y * wallDamper;
                }
                
                if (y < 0) {
                    y = 0;
                }
            }
            if (y > (WORLD_DIM - threshhold)) {
                if (pCell->mVelocity.y > 0) {
                    pCell->mVelocity.y = -pCell->mVelocity.y * wallDamper;
                }
                
                if (y > WORLD_DIM) {
                    y = WORLD_DIM + pCell->mVelocity.y;
                }
            }
            
            /*
            if (x < threshhold && pCell->mVelocity.x < 0) {
                pCell->mVelocity.x = -pCell->mVelocity.x;
            }
            
            if (x < 0 || x >= WORLD_DIM) {
                pCell->mVelocity.x = -pCell->mVelocity.x * wallDamper;
                x = pCell->mPos.x;
            }
            
            if (y < 0 || y >= WORLD_DIM) {
                pCell->mVelocity.y = -pCell->mVelocity.y * wallDamper;
                y = pCell->mPos.y;
            }
             */
            
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

    for (int i = 0; i < NUM_CELLS; i++) {
        if (pCell->mOnBoard) {
            mWorldSpace.move(pCell, pCell->mSavePos.x, pCell->mSavePos.y, pCell->mPos.x, pCell->mPos.y);
        }
        ++pCell;
    }
#endif
}

int World:: getEntityLength(Cell *pCell)
{
    Cell *pEntity = pCell->mEntityHead;
    if (pEntity->mEntityCount)
        return pEntity->mEntityCount;

    int result = 0;
    int entityIndex = pCell->mEntityIndex;

    pCell = mCells;
    
    for (int i = 0; i < NUM_CELLS; i++) {
        if (pCell->mOnBoard && pCell->mEntityIndex == entityIndex) {
            ++result;
        }
        ++pCell;
    }

    pEntity->mEntityCount = result;
    return result;
}

void World :: applyConnectionPhysics(NUMBER fraction) {
    int i, j;
    
    Cell * pCell = mCells;
    
    for (i = 0; i < NUM_CELLS; i++) {
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
            
            fraction = 1.0;
            NUMBER minDist = CELL_SIZE;
            NUMBER maxDist = pCell->mMaxConnectionLength[j];
            
            if (pCell->mContractTurnCount) {
                //-.5 to .5
                double fraction = double(1.0 + pCell->mContractTurnCount - CONTRACT_TURN_COUNT / 2) / double(CONTRACT_TURN_COUNT);
                fraction *= fraction * 4; // 1 to 0 to 1
                
                //maxDist = maxDist * fraction + (1.0 - fraction) * CELL_SIZE;
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

//            targestDistance = distance * (1.0 - fraction) + fraction * targestDistance;
            NUMBER fractionAdjust = (targestDistance - distance) / distance;
            fractionAdjust /= 2.0;
            x1 += xd * fractionAdjust;
            x2 -= xd * fractionAdjust;
            y1 += yd * fractionAdjust;
            y2 -= yd * fractionAdjust;
            
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
                pToCell->mPhase = 2;// + level * 5;
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
    for (i = 0; i < NUM_CELLS; i++) {
        
        if (pCell->mOnBoard) {
            
            collisionDistance = CELL_SIZE;
            if (pCell->mNumConnections) {
                collisionDistance *= 1.05;
            }
            /*
            for (int j = 0; j < pCell->mNumConnections; j++) {
                collisionDistance = MAX(collisionDistance, pCell->mConnections[j]->mPos.fastDistance(pCell->mPos));
            }*/
            
            ++numGeneration;
            pCell->mGeneration = numGeneration;

            CellPtr *pResultArray = results;

            int numResults =
            mWorldSpace.getNearbyCells(-1 /*numGeneration*/, pCell->mPos, collisionDistance, pResultArray, (int) sizeof(results)/sizeof(results[0]));
            
            if (numResults) {


                for (int j = 0; j < numResults; j++) {
                    Cell * pToCell = results[j];
                    
                    NUMBER scaleToVelocity = 1.0;
                    if (pCell->mNumConnections > 1 &&
                        pCell->mAction == actionBite &&
                        pToCell->mEntityIndex != pCell->mEntityIndex &&
                        // pToCell->mAction != actionBite &&
                        ! pCell->mInactiveCount) {
                        
                        // the more that we are moving in the direction of the attack, the stronger the bite
                        double biteStrength = 1.0;
                        
                        /*
                        Point pt(pToCell->mPos.x - pCell->mPos.x + pCell->mVelocity.x,
                                 pToCell->mPos.y - pCell->mPos.y + pCell->mVelocity.y);
                        
                        biteStrength += pt.distance();// * pCell->mVelocity.distance() / CELL_SIZE;
                        */
                        
                        double maxEnergy = BASE_ENERGY_PER_CELL * getEntityLength(pCell) * 1.5;
                        double newEnergy = pCell->mEntityHead->mEnergy + BITE_GAIN * biteStrength;
                        if (newEnergy < maxEnergy) {
                            pToCell->mEntityHead->mEnergy -= BITE_COST * biteStrength;
                            
                            scaleToVelocity = 0.7;
                            sendSignal(pCell, 1);
                            sendSignal(pToCell, 2);
                            pCell->mLastAte = 10;
                        }
                        
//                        pToCell->mInactiveCount = INACTIVE_AFTER_BITE_COUNT;
                        pCell->mInactiveCount = INACTIVE_AFTER_BITE_COUNT;
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
                        NUMBER dist = SQRT(xd*xd+yd*yd);
                        if (dist < CELL_SIZE) {
                            NUMBER fraction = CELL_SIZE - dist;
                            xd *= fraction;
                            yd *= fraction;
                            vx1 += xd;
                            vy1 += yd;
                            vx2 -= xd;
                            vy2 -= yd;
                            
                            NUMBER avgVX = (vx1 + vx2) / 2.0;
                            NUMBER avgVY = (vy1 + vy2) / 2.0;
                            
                            NUMBER avgF = .5;
                            
                            vx1 = avgF * avgVX + (1.0 - avgF) * vx1;
                            vy1 = avgF * avgVY + (1.0 - avgF) * vy1;
                            vx2 = avgF * avgVX + (1.0 - avgF) * vx2;
                            vy2 = avgF * avgVY + (1.0 - avgF) * vy2;
                            
                        }
                    }
                    
                    pCell->mVelocity.x = vx1;
                    pCell->mVelocity.y = vy1;
                    pToCell->mVelocity.x = vx2 * scaleToVelocity;
                    pToCell->mVelocity.y = vy2 * scaleToVelocity;
                    
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
                }
                
                
                pCell->mLastCollision = collisionLength;
                for (int j = 0; j < numResults; j++) {
                    results[j]->mLastCollision = collisionLength;
                }
            }
        }
        
        ++pCell;
    }
}

void enableMove(Cell *pCell, int * pTimeout) {
    if (*pTimeout > 0) {
        for (int i = 0; i < pCell->mNumConnections; i++) {
            Cell * pConnectedCell = pCell->mConnections[i];
            
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

void contract(Cell * pCell) {
    int mask = 1;
    if (! pCell->mContractTurnCount) {
        
        pCell->mContractTurnCount = CONTRACT_TURN_COUNT;
        for (int i = 0; i < pCell->mNumConnections; i++) {
            Cell *pToCell = pCell->mConnections[i];
            if (! pToCell->mContractTurnCount) {
                if ((pCell->mParam & mask) == (pToCell->mParam & mask)) {
                    contract(pToCell);
                }
            }
        }
    }
}

void World :: processCellActivity() {
    Cell * pCell = mCells;
    static int turn = 0;
    ++turn;
    

    CellPtr results[5];
    
    for (int i = 0; i < NUM_CELLS; i++) {
        if (! pCell->mOnBoard)
            continue;
        
        int numConnections = pCell->mNumConnections;
        int totalConnections = numConnections + pCell->mNumSecondaryConnections;
        
        int action = pCell->mAction;
        

        if (numConnections > 1 && (turn % pCell->mPhase) == 0) {
            
            if (action == actionContract) {
                contract(pCell);
            }
            else
            if ((pCell->mNumAllConnections <= 3) && (action == actionFlagellum || action == actionLook)) {
                NUMBER xd = 0, yd = 0;
                for (int j = 0; j < pCell->mNumAllConnections; j++) {
                    xd += pCell->mPos.x - pCell->mAllConnections[j]->mPos.x;
                    yd += pCell->mPos.y - pCell->mAllConnections[j]->mPos.y;
                }
                NUMBER length = pCell->mNumAllConnections + 2;

                pCell->mPhase = pCell->mDefaultPhase;
                Point v(CELL_SIZE * xd / length, CELL_SIZE * yd / length);

                switch (action) {
                    case actionFlagellum: {
                        
                    if (1 <= mWorldSpace.getNearbyCells(Point(pCell->mPos.x - v.x, pCell->mPos.y - v.y),
                                                        CELL_SIZE, results, (int) sizeof(results)/sizeof(results[0])))
                    {
                        pCell->mFlagellumVector = Point(v.x * 50, v.y * 50);
                        pCell->mLastFlagellum = FLAGELLUM_TURNS;
                        pCell->mEntityHead->mEnergy -= MOVE_ENERGY;
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
                                    
                                    if (((pCell->mParam & 1) == 0) == (pCell->mNumConnections > 0)) {
                                        int timeout = 200;
                                        sendSignal(pCell, 4);
//                                        enableMove(pCell, &timeout);
                                        break;
                                    }
                                }
                            }
                            
                        }
                        break; }
                }
            }
        }

        ++pCell;
    }
}

void World :: applyFluidMechanics() {
    Cell * pCell = mCells;
    for (int i = 0; i < NUM_CELLS; i++) {
        if (pCell->mOnBoard && pCell->mNumConnections == 2) {
            NUMBER vx = pCell->mPos.x - pCell->mSavePos.x;
            NUMBER vy = pCell->mPos.y - pCell->mSavePos.y;
            
            pCell->mVelocity.x -= vx * 20.0;
            pCell->mVelocity.y -= vy * 20.0;
        }
        ++pCell;
    }
}

Genome * World :: getTopGenome() {
    
    NUMBER maxEnergy = -100000;
    
    Genome * pResult = NULL;

    Cell * pCell = mCells;
    for (int i = 0; i < NUM_CELLS; i++) {
        if (pCell->mOnBoard)
        {
            double e = pCell->mEntityHead->mEnergy / double(pCell->mEntityHead->mEntityCount);
            if (pResult == NULL || e > maxEnergy) {
                maxEnergy = e;
                pResult = pCell->mEntityHead->mGenome;
            }
        }
        ++pCell;
    }
    return pResult;
}


void World :: turnCrank() {
    
    ++mWorldTurn;
    
    spawn();
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
    NUMBER vDamper = VELOCITY_DAMPING;
    Cell * pLastEntityCell = NULL;
    
    for (int i = 0; i < NUM_CELLS; i++) {
        if (pCell->mOnBoard)
        {
            pCell->mEntityHead->mEnergy -= TURN_COST;
            if (pCell->mEntityHead->mEnergy < 0) {
                if (pCell->mEntityHead->mNumConnections) {
                    killEntity(pCell);
                }
                else {
                    removeCell(pCell);
                }
                continue;
            }
            
            pCell->mVelocity.x *= vDamper;
            pCell->mVelocity.y *= vDamper;
            if (Globals::gravity) {
                pCell->mVelocity.y -= NUMBER(CELL_SIZE / 100.0);
            }
            
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
                NUMBER length = pCell->mNumAllConnections;// + 2.0;
                if (length) {
                Point v(CELL_SIZE * xd / length, CELL_SIZE * yd / length);
                pCell->mFlagellumVector = Point(v.x * 50, v.y * 50);
                pCell->mVelocity.x -= v.x;
                pCell->mVelocity.y -= v.y;
                }
                else {
                    printf("hey");
                }
                
            }
            if (pCell->mLastCollision) {
                --pCell->mLastCollision;
            }
            if (pCell->mLastAte) {
                --pCell->mLastAte;
            }
            if (pCell->mInactiveCount) {
                --pCell->mInactiveCount;
            }

#if BATCH_MOVE_POINTS
            pCell->mSavePos.x = pCell->mPos.x;
            pCell->mSavePos.y = pCell->mPos.y;
    #endif
        }
        ++pCell;
    }
    
    for (int i = 0; i < NUM_CONNECTION_PHYSICS_ITERATIONS; i++) {
        double f = NUMBER(i+1) / NUMBER(NUM_CONNECTION_PHYSICS_ITERATIONS);
        detectCollisions(f);
        applyPointPhysics(f);
        applyConnectionPhysics(f);
    }
    
#if BATCH_MOVE_POINTS
    applyNewPositions();
#endif
    processCellActivity();
    //applyFluidMechanics();
    
    if (mInFitnessTest) {
        ++mFitnessTestTurn;
        
        // if we haven't moved, skip testing
        if (mFitnessTestTurn > 10 && startCritterPoint.x == mCells[0].mPos.x && startCritterPoint.y == mCells[0].mPos.y) {
            mFitnessTestTurn = NUM_TURNS_PER_GENOME * 10;
        }

        //
        if (mFitnessTestTurn >= NUM_TURNS_PER_GENOME) {
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

void World :: removeCell(Cell *pCell) {
    if (pCell && pCell->mOnBoard) {
        mWorldSpace.remove(pCell);
        pCell->reset();
        --mNumCells;
    }
}

void World :: killEntity(Cell *pCell) {
    if (pCell && pCell->mOnBoard) {
        int entityIndex = pCell->mEntityIndex;
        if (pCell->mNumConnections) {
            --mNumEntities;
        }
        Cell * pCell = mCells;
        for (int i = 0; i < NUM_CELLS; i++) {
            if (pCell->mOnBoard && pCell->mEntityIndex == entityIndex) {
                NUMBER vx = pCell->mVelocity.x / 4.0;
                NUMBER vy = pCell->mVelocity.y / 4.0;
                NUMBER x = pCell->mPos.x;
                NUMBER y = pCell->mPos.y;
                mWorldSpace.remove(pCell);
                pCell->reset();
                pCell->mEnergy = Random::randRange(0.0f,BASE_ENERGY_PER_CELL/20.0);
                mWorldSpace.insert(pCell);
                /*
                pCell->mEntityIndex = pCell->mIndex;
                pCell->mEntityHead = pCell;
                pCell->mVelocity.x = vx;
                pCell->mVelocity.y = vy;
                 */
            }
            ++pCell;
        }
    }
}


Cell * World :: addCell(NUMBER x, NUMBER y, NUMBER minDistTest) {
    if (minDistTest) {
        CellPtr results[16];
        if (mWorldSpace.getNearbyCells(Point(x,y), minDistTest, results, 1)) {
            return NULL;
        }
    }
    
    Cell * result = mCells;
    for (int i = 0; i < NUM_CELLS; i++) {
        if (! result->mOnBoard) {
            ++mNumCells;
            result->reset();
            x = MAX(0, MIN(x, MAX_COORD));
            y = MAX(0, MIN(y, MAX_COORD));
            result->mPos.x = x;
            result->mPos.y = y;
            result->mIndex = i;
            result->mEnergy = Random::randRange(0, 4); // TODO
            mWorldSpace.insert(result);
            return result;
        }
        ++result;
    }
    return NULL;
    
    if (mNumCells < (NUM_CELLS-1)) {
        result = &mCells[mNumCells++];
        result->reset();
        x = MAX(0, MIN(x, MAX_COORD));
        y = MAX(0, MIN(y, MAX_COORD));
        result->mPos.x = x;
        result->mPos.y = y;
        mWorldSpace.insert(result);
    }
    return result;
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

    mFitnessTestIndex = 0;
    
    int numToTest = 100;
    
    int dim = sqrt(numToTest);
    
    reset();
}

void World::generateNewEntity(Genome g) {
    // step #1, find empty spot
    CellPtr resultArray[20];
    
    NUMBER x, y;
    NUMBER size = CELL_SIZE * 20.0;
    bool found = false;
    for (int i = 0; i < 20; i++) {
        x = Random::randRange(size/2.0, WORLD_DIM - size/2.0);
        y = Random::randRange(size/2.0, WORLD_DIM - size/2.0);
        
        if (0 == mWorldSpace.getNearbyCells(Point(x,y), size, resultArray)) {
            found = true;
            break;
        }
    }
    
    int count = 0;
    if (found) {
        while (true) {
            if (++count > 100) {
                break;
            }
            
            CreatureConstructor c(*this, g);
            if (c.go(x, y)) {
                ++mNumEntities;
                break;
            }
            else {
                c.deleteAddedCells();
                g.mutate();
            }
        }
    }
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
    if (false)
    for (int i = 0; i < NUM_CELLS; i++) {
        if (mCells[i].mOnBoard && mCells[i].mEntityIndex == entityIndex) {
            
            if (mCells[i].mEnergy > 0)
                ++ate;
            result -= .1;
            switch (mCells[i].mAction) {
                case actionBite:
                    result -= 3;
                    break;
                case actionFlagellum:
                    result -= 2;
                    break;
                default:
                    break;
            }
            result += mCells[i].mEnergy;
        }
    }
    
//    if (ate == 0)
//        result = -100000;
//    result = calcEnergy(&mCells[0]);
//    NUMBER result = calcDistance(&mCells[0]);
    return result;
}

void World :: prepareGenomeForTest() {
    
    if (Globals::pause) {
        Globals::minMsPerTurn = MAX(10, Globals::minMsPerTurn);
    }
    reset();

    // create the critter
    Genome & genome = genepool[mFitnessTestIndex].genome;
    CreatureConstructor constructor(*this, genome);
    constructor.go();
    startCritterPoint = mCells[0].mPos;
//    return;
    
//    for (int i = 0; i < 100; i++) {
//        this->turnCrank();
//    }
    // create some food

    if (false && (mFitnessGeneration % 2) == 0)
    {
        NUMBER radius = NUMBER(WORLD_DIM) / NUMBER(90 - (mFitnessGeneration % 50));
        NUMBER angle = NUMBER(mFitnessGeneration % 77) / 10.0;
        
        radius = NUMBER(WORLD_DIM) / NUMBER(50);
        angle = 0;
        
        for (int i = 0; i < 200; i++) {
            NUMBER x = WORLD_DIM / 2 + cos(angle) * radius;
            NUMBER y = WORLD_DIM / 2 + sin(angle) * radius;
            
            angle += 6.28 / 80.0;
            radius *= 1.007f;
            
            Cell * pNewCell = addCell(x,y, CELL_SIZE);
        }
    }
    else {
        double step = .004;
        for (double x = .4; x< .6; x += step) {
            for (double y = .4; y < .6; y += step) {
                Cell * pNewCell = addCell(x * WORLD_DIM, y * WORLD_DIM, CELL_SIZE * 3);
            }
        }
    }
    /*
    else {
        for (int i = 0; i < 200; i++) {
            
            NUMBER lb = .4 * WORLD_DIM;
            NUMBER ub = .6 * WORLD_DIM;
            
            NUMBER x = Random::randRange(lb, ub);
            NUMBER y = Random::randRange(lb, ub);
            
            Cell * pNewCell = addCell(x,y, CELL_SIZE);
        }
    }
     */
}

int randomIndex() {
    double r = Random::rand();
    r *= r;
    
    return int(r * NUM_GENOMES_TO_TEST / 2.0);
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

#if 1
        mFitnessTestIndex = NUM_GENOMES_TO_TEST / 20; // don't bother testing the top 20% again
        
        // create mutant forms of the top critters
        int j = NUM_GENOMES_TO_TEST / 20;
        for (i = 0; i < NUM_GENOMES_TO_TEST / 20; i++) {
            for (int n = 0; n < 4; n++) {
                genepool[j].genome = genepool[i].genome;
                for (int m = 0; m < (n + 1) * (n + 1); m++) {
                    genepool[j].genome.mutate();
                }
                ++j;
            }
        }
#else
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
                for (int m = 0; m < (n + 1) * 5; m++) {
                    genepool[j].genome.mutate();
                }
                ++j;
            }
        }
#endif
        
    }
}

int World :: getGeneration() {
    return mInFitnessTest ? (mFitnessGeneration + 1) : 0;
}

int World :: getTestIndex () {
    return mInFitnessTest ? (mFitnessTestIndex + 1) : 0;
}


