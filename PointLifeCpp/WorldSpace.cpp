//
//  WorldSpace.cpp
//  PointLifeCpp
//
//  Created by Scott SSE on 6/25/15.
//  Copyright (c) 2015 Scott SSE. All rights reserved.
//

#include "WorldSpace.h"

#define TRACE_FINDER if(false)TRACE
//#define HEAPCHECK testConsistency()
#define HEAPCHECK
#define TRACE(x,...)

WorldSpace::WorldSpace() {
}

void WorldSpace::clear()
{
    for (int x = 0; x < WORLD_DIM; x++) {
        for (int y = 0; y < WORLD_DIM; y++) {
            mDivisions[x][y] = NULL;
        }
    }
}

void WorldSpace:: insert(Cell * pCell, int x, int y)
{
    HEAPCHECK;

    if ((x < 0) || (y < 0) || (x >= WORLD_DIM) || (y >= WORLD_DIM)) {
        return;
    }
   
    pCell->mPrev = NULL;
    pCell->mNext = mDivisions[x][y];
    if (pCell->mNext != NULL) {
        pCell->mNext->mPrev = pCell;
    }
    mDivisions[x][y] = pCell;
    
    pCell->mOnBoard = true;
    
    HEAPCHECK;
}

void WorldSpace :: remove(Cell * pCell)
{
#if BATCH_MOVE_POINTS
    remove(pCell, (int) pCell->mSavePos.x, (int) pCell->mSavePos.y);
#else
    remove(pCell, (int) pCell->mPos.x, (int) pCell->mPos.y);
#endif
}

void WorldSpace:: remove(Cell * pCell, int x, int y)
{
    if ((x < 0) || (y < 0) || (x >= WORLD_DIM) || (y >= WORLD_DIM)) {
        return;
    }
    HEAPCHECK;
    
    if (! pCell->mOnBoard) {
        return;
    }
    pCell->mOnBoard = false;
    
    Cell * pNext = pCell->mNext;
    Cell * pPrev = pCell->mPrev;
    
    if (mDivisions[x][y] == pCell)
        mDivisions[x][y] = pNext;
    
    if (pNext != NULL)
        pNext->mPrev = pPrev;
    if (pPrev != NULL)
        pPrev->mNext = pNext;
    
    pCell->mNext = pCell->mPrev = NULL;
    
    HEAPCHECK;
}

void WorldSpace::move (Cell *pCell, NUMBER newX, NUMBER newY) {
    HEAPCHECK;

#if BATCH_MOVE_POINTS
    move(pCell, (int) pCell->mSavePos.x, (int) pCell->mSavePos.y, newX, newY);
#else
    move(pCell, pCell->mPos.x, pCell->mPos.y, newX, newY);
#endif
    
}

void WorldSpace:: move(Cell * pCell, NUMBER oldX, NUMBER oldY, NUMBER newX, NUMBER newY)
{
    HEAPCHECK;
    
    int ox = (int) oldX;
    int oy = (int) oldY;
    int nx = (int) newX;
    int ny = (int) newY;
    
    /*
    ox = MAX(0,MIN(ox, MAX_COORD));
    oy = MAX(0,MIN(oy, MAX_COORD));
    nx = MAX(0,MIN(nx, MAX_COORD));
    ny = MAX(0,MIN(ny, MAX_COORD));
*/
    if (ox != nx || oy != ny) {
        remove(pCell, ox, oy);
        pCell->mPos.x = newX;
        pCell->mPos.y = newY;
        insert(pCell, nx, ny);
    }
    else {
        pCell->mPos.x = newX;
        pCell->mPos.y = newY;
    }
    
#if BATCH_MOVE_POINTS
    pCell->mSavePos.x = newX;
    pCell->mSavePos.y = newY;
#endif
    HEAPCHECK;
}

int WorldSpace::getNearbyCells(Point pt, NUMBER distance, CellPtr *pResultArray, int maxResults, Cell *pExclude)
{
    HEAPCHECK;
    
    int result = 0;
    
    // detemine the square to search
    
    int fX = (int) MAX(pt.x - distance, 0);
    int tX = (int) MIN(pt.x + distance, MAX_COORD);
    int fY = (int) MAX(pt.y - distance, 0);
    int tY = (int) MIN(pt.y + distance, MAX_COORD);
    
    distance *= distance;
    
    for (int x = fX; x <= tX; x++) {
        for (int y = fY; y <= tY; y++) {
            CellPtr pCell = mDivisions[x][y];
            while (pCell != NULL) {
                
                if (pExclude == NULL || pExclude != pCell) {
                    NUMBER d = pt.squaredDistance(pCell->mPos);

                    if (d <= distance)
                    {
                        *pResultArray++ = pCell;
                        ++result;
                        //TRACE_FINDER("included (%f,%f), distance = %f, result count = %d\n",
                        //             pCell->mPos.x, pCell->mPos.y, d, result);
                        
                        if (result >= maxResults) {
                            HEAPCHECK;
                            return result;
                        }
                    }
                }
                pCell = pCell->mNext;
            }
        }
    }
    
    HEAPCHECK;
    
    return result;
}


int WorldSpace :: getNearbyCells(int excludeGeneration, Point pt, NUMBER distance, CellPtr *pResultArray, int maxResults) {
    int result = 0;
    
    // detemine the square to search
    
    int fX = (int) MAX(pt.x - distance, 0);
    int tX = (int) MIN(pt.x + distance, MAX_COORD);
    int fY = (int) MAX(pt.y - distance, 0);
    int tY = (int) MIN(pt.y + distance, MAX_COORD);
    
    distance *= distance;
    
    for (int x = fX; x <= tX; x++) {
        for (int y = fY; y <= tY; y++) {
            CellPtr pCell = mDivisions[x][y];
            while (pCell != NULL) {
                
                if (pCell->mGeneration != excludeGeneration) {
                    NUMBER d = pt.squaredDistance(pCell->mPos);
                    
                    if (d <= distance)
                    {
                        *pResultArray++ = pCell;
                        ++result;
                        //TRACE_FINDER("included (%f,%f), distance = %f, result count = %d\n",
                        //             pCell->mPos.x, pCell->mPos.y, d, result);
                        
                        if (result >= maxResults) {
                            HEAPCHECK;
                            return result;
                        }
                    }
                }
                pCell = pCell->mNext;
            }
        }
    }
    
    HEAPCHECK;
    
    return result;
}

static Cell * cells;

void WorldSpace::setCells(Cell *pCells) {
    cells = pCells;
}

void WorldSpace::testConsistency() {
    for (int i = 0; i < NUM_CELLS; i++) {
        Cell * pCell = &cells[i];
        if (pCell->mOnBoard) {
            int x = (int) pCell->mPos.x;
            int y = (int) pCell->mPos.y;
            
            bool bFound = false;
            Cell * pTestCell = mDivisions[x][y];
            if (pTestCell->mPrev)
                throw "head has prev";
            
            while (pTestCell) {
                if (pTestCell == pCell) {
                    bFound = true;
                    break;
                }
                if (pTestCell->mNext->mPrev != pTestCell)
                    throw "badlink";
                pTestCell = pTestCell->mNext;
            }
            
            if (! bFound)
                throw "not found";
        }
    }
    
    for (int x = 0; x < WORLD_DIM; x++) {
        for (int y = 0; y < WORLD_DIM; y++) {
            Cell * pTestCell = mDivisions[x][y];
            while (pTestCell) {
                if (! pTestCell->mOnBoard)
                    throw "not on board";
                pTestCell = pTestCell->mNext;
            }
        }
    }
}

