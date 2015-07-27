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
    void remove(Cell * pCell);
    void remove(Cell *, int x, int y);
    void move(Cell *pCell, NUMBER newX, NUMBER newY);
    void move(Cell *pCell, NUMBER oldX, NUMBER oldY, NUMBER newX, NUMBER newY);
    
    int getNearbyCells(Point pt, NUMBER distance, CellPtr *pResultArray, int maxResults = 16, Cell *pExclude = NULL);
    
    int getNearbyCells(Cell * pNearCell, NUMBER distance, CellPtr *pResultArray, int maxResults = 16) {
        return getNearbyCells(pNearCell->mPos, distance, pResultArray, maxResults, pNearCell);
    }
    
    int getNearbyCells(int excludeGeneration, Point pt, NUMBER distance, CellPtr *pResultArray, int maxResults = 16);

    void setCells(Cell * pCell);

private:
    void testConsistency();
    
private:
    CellPtr mDivisions[WORLD_DIM][WORLD_DIM];
};

#endif
