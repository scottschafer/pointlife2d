//
//  WorldSpace.h
//  PointLifeCpp
//
//  Created by Scott SSE on 6/25/15.
//  Copyright (c) 2015 Scott SSE. All rights reserved.
//

#ifndef PointLifeCpp_WorldSpace_h
#define PointLifeCpp_WorldSpace_h

#include "Cell.h"
#include "Point.h"

class WorldSpace {
public:
    WorldSpace();
    
    void clear();
    void insert(Cell* pCell) { insert(pCell, (int) pCell->mPos.x, (int) pCell->mPos.y); }
    void insert(Cell*, int x, int y);
    void remove(Cell * pCell) { remove(pCell, (int) pCell->mPos.x, (int) pCell->mPos.y); }
    void remove(Cell *, int x, int y);
    void move(Cell *pCell, NUM_TYPE newX, NUM_TYPE newY) { move(pCell, pCell->mPos.x, pCell->mPos.y, newX, newY); }
    void move(Cell *pCell, NUM_TYPE oldX, NUM_TYPE oldY, NUM_TYPE newX, NUM_TYPE newY);
    
    int getNearbyCells(Point pt, NUM_TYPE distance, CellPtr *pResultArray, int maxResults = 16, Cell *pExclude = NULL);
    
    int getNearbyCells(Cell * pNearCell, NUM_TYPE distance, CellPtr *pResultArray, int maxResults = 16) {
        return getNearbyCells(pNearCell->mPos, distance, pResultArray, maxResults, pNearCell);
    }
    
    int getNearbyCells(int excludeGeneration, Point pt, NUM_TYPE distance, CellPtr *pResultArray, int maxResults = 16);
    
private:
    CellPtr mDivisions[WORLD_DIM][WORLD_DIM];
};

#endif
