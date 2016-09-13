//
//  CreatureConstructor.cpp
//  PointLifeCpp
//
//  Created by Scott SSE on 7/6/15.
//  Copyright (c) 2015 Scott SSE. All rights reserved.
//

#include "CreatureConstructor.h"
#include "Globals.h"
#include <set>

#define MAX_CELLS 100
#define MAX_TURNS 5000

#define DEFAULT_MAX_LEVEL 20


inline double angle2Radians(double angle) { return angle * 6.28318530717959 / 360.0; }

CreatureConstructor :: CreatureConstructor(World & world, Genome & genome) : mWorld(world), mGenome(genome) {
    mIndex = 0;
    mNumCells = 0;

    mTurns = 0;
    mAngle = 0;
    mEntityIndex = -1;
    
    mOffsetX = mOffsetY = 0;
}

Cell * CreatureConstructor :: addCell(NUMBER x, NUMBER y) {

    WorldSpace &wp = mWorld.getWorldSpace();
    
    NUMBER minDist = CELL_SIZE * .99;

    CellPtr buffer[16];
    int numCollisions = wp.getNearbyCells(Point(x, y), minDist, buffer);
    if (numCollisions > 0) {
        return NULL;
    }
    
    if (mAddedCells.size() > MAX_CELLS) {
        return NULL;
    }

    Cell * pResult = mWorld.addCell(x, y);
    if (pResult) {
        pResult->mGenome = &mGenome;
        mAddedCells.push_back(pResult);
        if (mEntityIndex == -1) {
            mEntityIndex = pResult->mEntityIndex;
        }
        else {
            pResult->mEntityIndex = mEntityIndex;
        }
        pResult->mEntityHead = &mWorld.mCells[mEntityIndex];
        
        pResult->mEntityHead->mEnergy += BASE_ENERGY_PER_CELL;
        if (pResult->mNumConnections == 0) {
            pResult->mEnergy *= 5.0;
        }
        ++ mNumCells;
    }

    return pResult;
}

Cell * CreatureConstructor :: addCell(BYTE g, NUMBER x, NUMBER y) {
    
    Cell * pResult = addCell(x, y);
    if (pResult) {
        BYTE instruction = (g >> 4);
        BYTE param = (g & 15);
        pResult->mParam = param;
        
        pResult->mAction = instruction & 3;
        pResult->mPhase = 10;//param + 1;
        switch (pResult->mAction) {
            case actionFlagellum:
                pResult->mPhase = (param < 7) ? (10 + param * 10) : 10000;
                break;
        }
        pResult->mNumAllowableConnections = MAX_CONNECTIONS;
    }
    return pResult;
}

bool addCellBasedOnType(BYTE g) {
    return g < 200;
}

void CreatureConstructor :: processGenome(int genomeIndex, int iCellA, int iCellB, int level, int maxLevel,
                                          NUMBER elasticity) {
    
    if (maxLevel == 0) {
        maxLevel = DEFAULT_MAX_LEVEL;
    }
    
    if (++level > maxLevel) return;
    
    if (++mTurns > 1000) {
        return;
    }
    
    if (elasticity == 0) {
        elasticity = 1;
    }
    
    CellPtr buffer[16];
    WorldSpace &wp = mWorld.getWorldSpace();
    
    NUMBER dist = (1.0 + elasticity) * CELL_SIZE / 2.0;
    
    BYTE * genome = mGenome.genome;

    if (iCellA == iCellB) {
        NUMBER r = dist / 2.0;
        NUMBER c = NUMBER(WORLD_DIM/2);
        NUMBER a = 0;

        NUMBER x = mOffsetX;// c;
        NUMBER y = (Globals :: gravity) ? NUMBER(WORLD_DIM*.07) : mOffsetY;
        
        Cell * pCellA = addCell(genome[genomeIndex++], x, y);
        
        x += dist;
        
        Cell * pCellB = addCell(genome[genomeIndex++], x, y);
        
        a += angle2Radians(240);
        x += dist * cos(a);
        y += dist * sin(a);
        
        Cell * pCellC = addCell(genome[genomeIndex++], x, y);
        
        if (! pCellA || ! pCellB || ! pCellC) {
            return;
        }
        //.....

        pCellA->connect(pCellB, dist, genome[genomeIndex++]);
        pCellB->connect(pCellC, dist, genome[genomeIndex++]);
        pCellC->connect(pCellA, dist, genome[genomeIndex++]);
        
        pCellA->mClockwise = pCellB;
        pCellB->mClockwise = pCellC;
        pCellC->mClockwise = pCellA;
        

        int ai = pCellA->mIndex;
        int bi = pCellB->mIndex;
        int ci = pCellC->mIndex;
        processGenome(genomeIndex++, ai, bi, level, maxLevel);
        processGenome(genomeIndex++, bi, ci, level, maxLevel);
        processGenome(genomeIndex++, ci, ai, level, maxLevel);
    }
    else {
        BYTE g = genome[(genomeIndex++) % GENOME_LENGTH];
        BYTE instruction = (g >> 4);
        BYTE param = (g & 15);

        switch (instruction) {
            case 15: {
                int newIndex = (genomeIndex - param * 5 + GENOME_LENGTH) % GENOME_LENGTH;
                processGenome(newIndex, iCellA, iCellB, level, maxLevel, elasticity);
                break;
            }
            case 14: {
                maxLevel = DEFAULT_MAX_LEVEL + param * 3;
                processGenome(genomeIndex, iCellA, iCellB, level, maxLevel, elasticity);
                break;
            }
                
            case 13:
                elasticity = 1.0 + NUMBER(param) / 31.0;
                processGenome(genomeIndex, iCellA, iCellB, level, maxLevel, elasticity);
                break;

                /*
            case 12:
            case 11:
            case 10:
                break;
                
            case 9:
                if (param > 10)
                return;
                 */
                
            default: {
                Cell & a = mWorld.mCells[iCellA];
                Cell & b = mWorld.mCells[iCellB];
                
                NUMBER dx = b.mPos.x - a.mPos.x;
                NUMBER dy = b.mPos.y - a.mPos.y;
                
                NUMBER dist = SQRT(dx*dx+dy*dy);
                
                NUMBER cx = (a.mPos.x + b.mPos.x) / 2.0;
                NUMBER cy = (a.mPos.y + b.mPos.y) / 2.0;
                
                Point normal(-dy, dx);
                
                NUMBER h = (dist * sqrt(3.0) / 2.0);
                NUMBER scale = h / normal.distance();
                
                Point newPos(cx + normal.x * scale, cy + normal.y * scale);
                NUMBER distA = newPos.distance(a.mPos);
                NUMBER distB = newPos.distance(b.mPos);

                NUMBER maxDist = CELL_SIZE * elasticity;
                
                if (distA <= maxDist && distB <= maxDist) {
                    
                    Cell * pCellC = addCell(g, newPos.x, newPos.y);
                    if (pCellC) {
                        int c1 = genome[(genomeIndex++) % GENOME_LENGTH];
                        int c2 = genome[(genomeIndex++) % GENOME_LENGTH];
                        
                        a.connect(pCellC, maxDist, c1);
                        pCellC->connect(&b, maxDist, c2);

                        int iCellC = pCellC->mIndex;
                        processGenome(genomeIndex++, iCellA, iCellC, level, maxLevel, elasticity);
                        processGenome(genomeIndex++, iCellC, iCellB, level, maxLevel, elasticity);

                        pCellC->mClockwise = &b;
                    
                    }
                }
            }
        }
    }
}

bool CreatureConstructor::go(NUMBER x, NUMBER y) {
    
    if (x == -1) {
        x = WORLD_DIM / 2;
    }
    
    if (y == -1) {
        y = WORLD_DIM / 2;
    }
    mOffsetX = x;
    mOffsetY = y;
    processGenome(0);
    return finalize();
}

void CreatureConstructor :: deleteAddedCells() {
    for (int i = 0; i < mAddedCells.size(); i++) {
        Cell * pCell = mAddedCells[i];
        mWorld.removeCell(pCell);
    }
    mAddedCells.clear();
}

Cell * CreatureConstructor :: createCell(int genomeIndex, NUMBER angle, NUMBER x, NUMBER y, int level) {

    if (++level > 8) return NULL;
    
    Cell * pResult = NULL;

    CellPtr buffer[16];
    WorldSpace &wp = mWorld.getWorldSpace();

    NUMBER dist = CELL_SIZE * 1.1f;//1.25;
    NUMBER minDist = CELL_SIZE * .95;
    int numCollisions = wp.getNearbyCells(Point(x, y), minDist, buffer);
    if (numCollisions > 0) {
        return NULL;
    }
    
    pResult = addCell(x, y);
    if (! pResult) {
        return NULL;
    }
    
    
    Cell & cell = *pResult;
    
    // now for this cell, assign its action and create additional cells and connections
    BYTE g = mGenome.genome[(genomeIndex++) % GENOME_LENGTH];
    BYTE instruction = (g >> 4);
    BYTE param = (g & 15);
    
    cell.mAction = instruction & 3;
    cell.mPhase = param * 5 + 1;
    if (cell.mAction == actionFlagellum) {
        if ( param < 7) {
            cell.mPhase = 1000;
         }
    }
    double elasticity = 1;

    for (int i = 0; cell.mNumConnections < (MAX_CONNECTIONS-1) && i < MAX_CONNECTIONS; ) {

        if (++mTurns > 10000) {
            return pResult;
        }

        BYTE g = mGenome.genome[(genomeIndex++) % GENOME_LENGTH];
        BYTE instruction = (g >> 4);
        BYTE param = (g & 15);

        Cell * pConnectWithCell = NULL;
        switch (instruction) {
            case 15:
                elasticity = 1 + NUMBER(param) / 15.0;
            case 0:
            case 1:
            case 2:
            case 3: {
                NUMBER x2 = x + dist * cos(angle);
                NUMBER y2 = y + dist * sin(angle);
                pConnectWithCell = createCell(genomeIndex, angle, x2, y2, level);
                ++i;
                break; }
                
            case 4:
            case 5:
            case 6:
            case 7:
            {
                break;
                int numNearby = wp.getNearbyCells(Point(x, y), dist, buffer);
                for (int j = 0; j < numNearby; j++) {
                    if (buffer[j]->mEntityIndex == mEntityIndex && (buffer[j]->mAction & 3) == (param & 3)) {
                        pConnectWithCell = buffer[j];
                        break;
                    }
                }
                ++i;
                
                break;
            }

            case 8:
            case 9:
            case 10:
                break;
                
            case 11:
                genomeIndex = (genomeIndex + GENOME_LENGTH - param*2) % GENOME_LENGTH;
                break;

            case 12:
                return pResult;
                break;
                
            default:
                ++i;
                break;
        }
        
                
        if (pConnectWithCell) {
            int index = cell.mNumConnections++;
            cell.mConnections[index] = pConnectWithCell;
            cell.mMaxConnectionLength[index] = elasticity * dist;
        }
        
        angle += angle2Radians(60);
    }
    
    return pResult;
}



void addCellsToSet(Cell *pCell, std::set<Cell*> & cellSet) {
    cellSet.insert(pCell);
    for (int i = 0; i < pCell->mNumConnections; i++) {
        addCellsToSet(pCell->mConnections[i], cellSet);
    }
}

bool CreatureConstructor :: finalize() {
    
    if (mAddedCells.size() < 3 )
        return false;
    
    int actioncount = 0;
    
    for (int i = 0; i < mAddedCells.size(); i++) {
        Cell * pCell = mAddedCells[i];
        
        pCell->mInitialEnergy = pCell->mEnergy;
        
        pCell->mDefaultPhase = pCell->mPhase;
        
        switch (pCell->mAction) {
            case actionBite:
            case actionContract:
            case actionFlagellum:
                ++actioncount;
        }
    }
    
    if (actioncount == 0) {
        for (int i = 0; i < mAddedCells.size(); i++) {
            Cell * pCell = mAddedCells[i];
            pCell->mAction = 0;
        }
        return false;
    }

        return true;
    CellPtr buffer[16];
    
    std::vector<CellPtr*> preservedCells;

//    std::vector<Cell*>::iterator i;
        /*
    std::vector<Cell*>::iterator i = mAddedCells.end();
    --i;
    while (i != mAddedCells.begin()) {
        Cell * pCell = *i;
        int numCollisions = wp.getNearbyCells(pCell, CELL_SIZE/2, buffer);
        if (numCollisions > 0) {
            wp.remove(pCell);
            i = mAddedCells.erase(i);
            if (mAddedCells.size() <= 1)
                break;
            --i;
        }
        --i;
    }
         */
    
    /*
    if (! mAddedCells.size())
        return;

    for (int i = 0; i < mAddedCells.size(); i++) {
        Cell * pCell1 = mAddedCells[i];
        for (int j = (i+1); j < mAddedCells.size(); j++) {
            Cell * pCell2 = mAddedCells[j];
            
            if (pCell1->mNumConnections < pCell1->mNumAllowableConnections &&
                pCell2->mNumConnections < pCell2->mNumAllowableConnections) {
            
                NUMBER distance = pCell1->mPos.distance(pCell2->mPos);
                
                if (distance <= pCell1->mMaxConnectionLength[pCell1->mNumConnections]) {
                    
                    pCell1->connect(pCell2);
                }
            }
        }
    }
     */
    
    /*
    if (mAddedCells.size() > 1) {
        i = mAddedCells.begin();
        ++i;
        while (i != mAddedCells.end()) {
            Cell * pCell = *i;
            if (pCell->mNumConnections == 0) {
                wp.remove(pCell);
                i = mAddedCells.erase(i);
            }
            else ++i;
        }
    }
    
    std::set<Cell*> connectedCells;
    addCellsToSet(mAddedCells[0], connectedCells);
    i = mAddedCells.begin();
    for ( ; i != mAddedCells.end(); ) {
        if (connectedCells.find(*i) == connectedCells.end()) {
            wp.remove(*i);
            i = mAddedCells.erase(i);
        } else {
            ++i;
        }
    }
     */
}

#if 0
#if 1
void CreatureConstructor :: go(Cell * pFromCell, NUMBER angle) {
    NUMBER x, y;
    
    if (++mTurns > 10000) {
        return;
    }
    
    if (pFromCell) {
        x = pFromCell->mPos.x;
        y = pFromCell->mPos.y;
    }
    else {
        x = WORLD_DIM/2;
        y = WORLD_DIM/2;
    }
    
    NUMBER dist = CELL_SIZE * 1.5;
    
    int numAdded = 0;
    
    
    CellPtr buffer[16];
    WorldSpace &wp = mWorld.getWorldSpace();
    
    // first, add a cell
    Cell * pCell = pFromCell;
    if (! pCell) {
        int numCollisions = wp.getNearbyCells(Point(x, y), CELL_SIZE/2, buffer);
        if (numCollisions > 0) {
            return;
        }
        pCell = mWorld.addCell(x, y);
        if (! pCell) {
            return;
        }
    }
    
    // now populate its ports
    int portIndex = 0;
    while (mIndex < mNumCells && portIndex < MAX_CONNECTIONS) {
        BYTE g = mGenome.genome[mIndex++];
        
        BYTE instruction = (g >> 4);
        BYTE param = (g & 15);
        
        switch (instruction) {
            default:
                // skip this port
                angle += angle2Radians(60);
                ++portIndex;
                if (portIndex >= MAX_CONNECTIONS) {
                    return;
                }
                break;
                
            case 0: {
                // add a connection to the port without a cell attached
                pCell->mConnectionCellType[portIndex] = param;
                break;
            }
                
            case 1: {
                // add a cell at the port
                pCell->mConnectionCellType[portIndex] = param;
                
                if (pFromCell && pFromCell->mNumConnections >= pFromCell->mNumAllowableConnections) {
                    return;
                }
                
                NUMBER newX = x + cos(angle) * dist;
                NUMBER newY = y + sin(angle) * dist;

                // if there's no room for this cell, skip it
                int numCollisions = wp.getNearbyCells(Point(newX,newY), CELL_SIZE/2, buffer);
                if (numCollisions > 0) {
                    break;
                }
                
                Cell * pNewCell = mWorld.addCell(newX, newY);
                if (pNewCell == NULL) {
                    return;
                }
                if (mAddedCells.size() == 0) {
                    pNewCell->mEntityIndex = pNewCell->mIndex;
                }
                else {
                    pNewCell->mEntityIndex = mAddedCells[0]->mIndex;
                }
                
                mAddedCells.push_back(pNewCell);
                if (maxConnections >= MAX_CONNECTIONS) {
                    maxConnections = MAX_CONNECTIONS - 1;
                }
                pNewCell->mNumAllowableConnections = maxConnections;
                for (int i = 0; i < MAX_CONNECTIONS; i++) {
                    pNewCell->mMaxConnectionLength[i] = elasticity * CELL_SIZE;
                }
                
                pNewCell->mAction = param;
                pNewCell->mPhase = 10;
                
                if (pFromCell) {
                    pFromCell->connect(pNewCell);
                }
                go(pNewCell, angle + 6.28318530717959 * 135.0 / 360.0);
                
                if (pFromCell && pFromCell->mNumConnections >= pFromCell->mNumAllowableConnections) {
                    return;
                }
                
                if (! pFromCell) {
                    return;
                }
                //                mAngle = 0;
                break; }
                
            case 6:
                maxConnections = 1 + (param % MAX_CONNECTIONS);
                break;
                
            case 7:
                elasticity = 1.1 + NUMBER(param) / 5.0;
                break;
                
            case 8:
            case 9:
                flagellum = true;
                break;
                
            case 10:
            case 11:
                //                mAngle += 6.28318530717959 * 135.0 / 360.0;
                break;
                
            case 12:
            case 13:
                //                mAngle -= 6.28318530717959 * 135.0 / 360.0;
                break;
                
            case 15:
                return;
                //                mNumCells = mIndex;
                break;
                
        }
    }
}
#else
void CreatureConstructor :: go(Cell * pFromCell, NUMBER angle) {
    NUMBER x, y;
    
    if (++mTurns > 10000) {
        return;
    }

    if (pFromCell) {
        x = pFromCell->mPos.x;
        y = pFromCell->mPos.y;
    }
    else {
        x = WORLD_DIM/2;
        y = WORLD_DIM/2;
    }
    
    NUMBER dist = CELL_SIZE;// * 1.25;
    
    int numAdded = 0;
    

    CellPtr buffer[16];
    WorldSpace &wp = mWorld.getWorldSpace();

    
    while (mIndex < mNumCells) {
        BYTE g = mGenome.genome[mIndex++];
        
        BYTE instruction = (g >> 4);
        BYTE param = (g & 15);
        
        switch (instruction) {
            default:
                break;
                
            case 0:
                angle -= 6.28318530717959 * 270.0 / 360.0;
            case 1:
                angle += 6.28318530717959 * 135.0 / 360.0;
            case 2:
            case 3:
            case 4:
            case 5: {
                if (pFromCell && pFromCell->mNumConnections >= pFromCell->mNumAllowableConnections) {
                    return;
                }
                NUMBER newX = x + cos(angle) * dist;
                NUMBER newY = y + sin(angle) * dist;
                
                int numCollisions = wp.getNearbyCells(Point(newX,newY), CELL_SIZE/2, buffer);
                if (numCollisions > 0) {
                    break;
                }
                
                Cell * pNewCell = mWorld.addCell(newX, newY);
                if (pNewCell == NULL) {
                    return;
                }
                if (mAddedCells.size() == 0) {
                    pNewCell->mEntityIndex = pNewCell->mIndex;
                }
                else {
                    pNewCell->mEntityIndex = mAddedCells[0]->mIndex;
                }
                
                mAddedCells.push_back(pNewCell);
                if (maxConnections >= MAX_CONNECTIONS) {
                    maxConnections = MAX_CONNECTIONS - 1;
                }
                pNewCell->mNumAllowableConnections = maxConnections;
                for (int i = 0; i < MAX_CONNECTIONS; i++) {
                    pNewCell->mMaxConnectionLength[i] = elasticity * CELL_SIZE;
                }
                
                pNewCell->mAction = param;
                pNewCell->mPhase = 10;

                if (pFromCell) {
                    pFromCell->connect(pNewCell);
                }
                go(pNewCell, angle + 6.28318530717959 * 135.0 / 360.0);

                if (pFromCell && pFromCell->mNumConnections >= pFromCell->mNumAllowableConnections) {
                    return;
                }
                
                if (! pFromCell) {
                    return;
                }
//                mAngle = 0;
                break; }
                
            case 6:
                maxConnections = 1 + (param % MAX_CONNECTIONS);
                break;
                
            case 7:
                elasticity = 1.1 + NUMBER(param) / 5.0;
                break;
                
            case 8:
            case 9:
                flagellum = true;
                break;
                
            case 10:
            case 11:
//                mAngle += 6.28318530717959 * 135.0 / 360.0;
                break;
                
            case 12:
            case 13:
//                mAngle -= 6.28318530717959 * 135.0 / 360.0;
                break;
                
            case 15:
                return;
//                mNumCells = mIndex;
                break;

        }
    }
}
#endif
#endif
