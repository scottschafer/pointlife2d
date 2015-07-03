//
//  World.h
//  PointLifeCpp
//
//  Created by Scott SSE on 6/25/15.
//  Copyright (c) 2015 Scott SSE. All rights reserved.
//

#ifndef PointLifeCpp_World_h
#define PointLifeCpp_World_h

#include "constants.h"
#include "WorldSpace.h"

class World {
public:
    World();
    
    void reset();
    void randomize();
    
    void turnCrank();
    
private:
    void applyPointPhysics();
    void applyConnectionPhysics();
    void detectCollisions();
    void processCellActivity();    
    void applyNewPositions();

    WorldSpace mWorldSpace;

public:
    Cell * mCells;
    Genome * mGenomes;
};

#endif
