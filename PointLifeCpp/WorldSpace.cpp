//
//  WorldSpace.cpp
//  PointLifeCpp
//
//  Created by Scott SSE on 6/25/15.
//  Copyright (c) 2015 Scott SSE. All rights reserved.
//

#include "WorldSpace.h"

#define TRACE_FINDER if(false)TRACE
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
    
    pCell->mPrev = NULL;
    pCell->mNext = mDivisions[x][y];
    if (pCell->mNext != NULL) {
        pCell->mNext->mPrev = pCell;
    }
    mDivisions[x][y] = pCell;
    
    pCell->mOnBoard = true;
    
    HEAPCHECK;
}

void WorldSpace:: remove(Cell * pCell, int x, int y)
{
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

void WorldSpace:: move(Cell * pCell, NUM_TYPE oldX, NUM_TYPE oldY, NUM_TYPE newX, NUM_TYPE newY)
{
    HEAPCHECK;
    
    int ox = (int) oldX;
    int oy = (int) oldY;
    int nx = (int) newX;
    int ny = (int) newY;
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
    
    HEAPCHECK;
}

int WorldSpace::getNearbyCells(Point pt, NUM_TYPE distance, CellPtr *pResultArray, int maxResults, Cell *pExclude)
{
    HEAPCHECK;
    
    int result = 0;
    
    // detemine the square to search
    
    int fX = (int) MAX(pt.x - distance, 0);
    int tX = (int) MIN(pt.x + distance, MAX_COORD);
    int fY = (int) MAX(pt.y - distance, 0);
    int tY = (int) MIN(pt.y + distance, MAX_COORD);
    
    for (int x = fX; x <= tX; x++) {
        for (int y = fY; y <= tY; y++) {
            CellPtr pCell = mDivisions[x][y];
            while (pCell != NULL) {
                
                if (pExclude == NULL || pExclude != pCell) {
                    NUM_TYPE d = pt.distance(pCell->mPos);

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


int WorldSpace :: getNearbyCells(int excludeGeneration, Point pt, NUM_TYPE distance, CellPtr *pResultArray, int maxResults) {
    int result = 0;
    
    // detemine the square to search
    
    int fX = (int) MAX(pt.x - distance, 0);
    int tX = (int) MIN(pt.x + distance, MAX_COORD);
    int fY = (int) MAX(pt.y - distance, 0);
    int tY = (int) MIN(pt.y + distance, MAX_COORD);
    
    for (int x = fX; x <= tX; x++) {
        for (int y = fY; y <= tY; y++) {
            CellPtr pCell = mDivisions[x][y];
            while (pCell != NULL) {
                
                if (pCell->mGeneration != excludeGeneration) {
                    NUM_TYPE d = pt.distance(pCell->mPos);
                    
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
