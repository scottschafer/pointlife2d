//
//  Random.cpp
//  PointLifeCpp
//
//  Created by Scott SSE on 6/25/15.
//  Copyright (c) 2015 Scott SSE. All rights reserved.
//

#include "Random.h"
#include <cstdlib>
#include <time.h>

#define RAND_ARRAY_SIZE 65535

static NUMBER * arrayRandoms = NULL;
static int randIndex = 0;

/**
 * Init, create the array of random numbers that we'll cycle through for speed.
 */

void Random::init() {
    
    srand (time(NULL));
    
    if (arrayRandoms == NULL) {
        arrayRandoms = new NUMBER[RAND_ARRAY_SIZE];
        for (int i = 0; i < RAND_ARRAY_SIZE; i++) {
            double r = double(::rand());
            r /= double(RAND_MAX);
            
            if (r >= 1.0) {
                r = 0.999999999999;
            }
            arrayRandoms[i] = r;
        }
    }
}

/**
 * Get the next rand, in the range of >= 0 and < 1
 */
NUMBER Random::rand() {
    randIndex = (randIndex + 1) % RAND_ARRAY_SIZE;
    return arrayRandoms[randIndex];
}


NUMBER Random::randRange(NUMBER minV, NUMBER maxV) {
    NUMBER r = Random::rand();
    return minV + r * (maxV - minV);
}

int Random::randRange(int minV, int maxV) {
    NUMBER r = Random::rand();
    return (int) (minV + r * (maxV - minV));
}
