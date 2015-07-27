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
    static NUMBER rand();
    static NUMBER randRange(NUMBER minV, NUMBER maxV);
    static int randRange(int minV, int maxV);
};

#endif /* defined(__PointLifeCpp__Random__) */
