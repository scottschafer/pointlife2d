//
//  CreatureConstructor.h
//  PointLifeCpp
//
//  Created by Scott SSE on 7/6/15.
//  Copyright (c) 2015 Scott SSE. All rights reserved.
//

#ifndef __PointLifeCpp__CreatureConstructor__
#define __PointLifeCpp__CreatureConstructor__

#include <stdio.h>
#include <vector>
#include "world.h"
#include "Genome.h"
#include "Cell.h"

class CellPhenotype {
public:
    // color
    // up to six connections, each with:
    //  elasticity
    //  color
    
};

class CreatureConstructor {
public:
    CreatureConstructor(World &, Genome &);
    
    Cell * createCell(int genomeIndex, NUMBER angle, NUMBER x, NUMBER y, int level = 0);
    Cell * addCell(NUMBER x, NUMBER y);
    Cell * addCell(BYTE cellType, NUMBER x, NUMBER y);
    void addCell(int cellType, NUMBER x, NUMBER y);
    
    
    void processGenome(int genomeIndex, int iCellA = 0, int iCellB = 0, int level = 0, int maxLevel = 0,
                       NUMBER elasticity = 0);
    
    bool go(NUMBER x = -1, NUMBER y = -1);
    void deleteAddedCells();
    
    void go(Cell * pFromCell);
    
    void go(Cell * pFromCell, NUMBER angle);
    
    void connectCell(Cell *pFrom, Cell *pTo, int portIndex);
    
private:
    bool finalize();
    
private:
    World & mWorld;
    Genome & mGenome;
    NUMBER mOffsetX, mOffsetY;
    int mNumCells;
    int mIndex;
    int mTurns;
    int mEntityIndex;
    NUMBER mAngle;
    std::vector<Cell*> mAddedCells;
};

#endif /* defined(__PointLifeCpp__CreatureConstructor__) */
