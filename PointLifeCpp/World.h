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
    
    void beginFitnessTest();
    void endFitnessTest();
    
    double testGenome(Genome & genome);
    
    Cell * addCell(NUMBER x, NUMBER y, NUMBER minDistTest = 0);
    void removeCell(Cell *pCell);
    void killEntity(Cell *pCell);
    
    Genome * getTopGenome();
    void generateNewEntity(Genome);
    void generateNewEntity(const char ** asciiArt);
    
    int getGeneration();
    int getTestIndex();
    
    int getTurn() { return mWorldTurn; }
    int getNumEntities() { return mNumEntities; }
    WorldSpace & getWorldSpace() { return mWorldSpace; }
    
private:
    int getEntityLength(Cell *pCell);
    
    void applyPointPhysics(NUMBER fraction = 1.0);
    void applyConnectionPhysics(NUMBER fraction = 1.0);
    void detectCollisions(NUMBER fraction = 1.0);
    void applyFluidMechanics();
    
    void processCellActivity();    
    void applyNewPositions();

    void sendSignal(Cell * pCell, int signal, int level = 0);
    WorldSpace mWorldSpace;
    
    void spawn();

private:
    double calcFitness();
    void prepareGenomeForTest();
    void prepareGenerationForTest();
    
public:
    Cell * mCells;
    Genome * mGenomes;
    int mNumCells;
    int mNumEntities;

    
    bool mInFitnessTest;
    int mFitnessGeneration;
    int mFitnessTestIndex;
    int mFitnessTestTurn;
    
    int mWorldTurn;
};

#endif
