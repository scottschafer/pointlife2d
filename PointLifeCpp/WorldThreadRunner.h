//
//  WorldThreadRunner.h
//  PointLifeCpp
//
//  Created by Scott SSE on 6/25/15.
//  Copyright (c) 2015 Scott SSE. All rights reserved.
//

#ifndef __PointLifeCpp__WorldThreadRunner__
#define __PointLifeCpp__WorldThreadRunner__

#include <stdio.h>

#include "World.h"

class WorldThreadMutex {
public:
    WorldThreadMutex();
    ~WorldThreadMutex();
};

class WorldThreadRunner {
public:
    static void start();
    static void pause();
    static void resume();
    static void exit();

    static World & getWorld() { return mWorld; }
    
    static void * threadFunc(void*);
    
private:
    static World mWorld;
};
#endif /* defined(__PointLifeCpp__WorldThreadRunner__) */
