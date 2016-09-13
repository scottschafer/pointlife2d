//
//  WorldThreadRunner.cpp
//  PointLifeCpp
//
//  Created by Scott SSE on 6/25/15.
//  Copyright (c) 2015 Scott SSE. All rights reserved.
//

#include "WorldThreadRunner.h"
#include "World.h"
#include "Globals.h"
#include <pthread.h>
#include <unistd.h>

World WorldThreadRunner :: mWorld;

static bool isPaused = false;
static bool alive = false;

#if USE_SIMULATION_THREAD
static pthread_t worldThread;
#endif

void WorldThreadRunner :: start() {

    alive = true;
#if USE_SIMULATION_THREAD
    if(pthread_create(&worldThread, NULL, &WorldThreadRunner::threadFunc, NULL)) {
        
        fprintf(stderr, "Error creating thread\n");
        exit();
    }
#endif
}


void WorldThreadRunner :: pause() {
    isPaused = true;
}

void WorldThreadRunner :: resume() {
    isPaused = false;
}

void WorldThreadRunner :: exit() {
    alive = false;
}

inline long curMicroseconds() {
    return clock() / (CLOCKS_PER_SEC/1000000);
}

void * WorldThreadRunner :: threadFunc(void*) {

    
    while (alive) {
        long minMicroseconds = Globals::minMsPerTurn;
        minMicroseconds *= minMicroseconds;
        minMicroseconds *= 1000;
        
        long startTime = curMicroseconds();
        {
            WorldThreadMutex wtm;
            mWorld.turnCrank();
        }
        long elapsedTime = curMicroseconds() - startTime;

        if (minMicroseconds > elapsedTime) {
            usleep(minMicroseconds - elapsedTime);
        }
    }
    return NULL;
}

#if USE_SIMULATION_THREAD
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
#endif

WorldThreadMutex::WorldThreadMutex() {
#if USE_SIMULATION_THREAD
    pthread_mutex_lock( &mutex );
#endif
}

WorldThreadMutex::~WorldThreadMutex() {
#if USE_SIMULATION_THREAD
    pthread_mutex_unlock( &mutex );
#endif
}
