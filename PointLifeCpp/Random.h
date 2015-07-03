//
//  Random.h
//  PointLifeCpp
//
//  Created by Scott SSE on 6/25/15.
//  Copyright (c) 2015 Scott SSE. All rights reserved.
//

#ifndef __PointLifeCpp__Random__
#define __PointLifeCpp__Random__

#include "constants.h"

class Random {
public:
    static void init();
    static NUM_TYPE rand();
    static NUM_TYPE randRange(NUM_TYPE minV, NUM_TYPE maxV);
    static int randRange(int minV, int maxV);
};

#endif /* defined(__PointLifeCpp__Random__) */
