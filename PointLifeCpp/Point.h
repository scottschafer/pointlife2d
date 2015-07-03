//
//  Point.h
//  PointLifeCpp
//
//  Created by Scott SSE on 6/25/15.
//  Copyright (c) 2015 Scott SSE. All rights reserved.
//

#ifndef PointLifeCpp_Point_h
#define PointLifeCpp_Point_h

#include "constants.h"
#include "Random.h"
#include <math.h>
#include <algorithm>

/**
 * I'm unclear as to whether it's better (speed-wise) to pass by value or reference. But I'll trust this:
 * http://cpp-next.com/archive/2009/08/want-speed-pass-by-value/
 */

#define POINT_REF Point
#define USE_FAST_DISTANCE 1
//#define POINT_REF const Point &

class Point {
public:
    NUM_TYPE x, y;

    Point() {
    }
    
    Point(NUM_TYPE x, NUM_TYPE y) : x(x), y(y) {
    }
    
    void reset() {
        x = y = 0;
    }
    
    Point operator + (POINT_REF p2) {
        return Point(x + p2.x, y + p2.y);
    }
    
    Point & operator += (POINT_REF p2) {
        x += p2.x;
        y += p2.y;
        return *this;
    }
    
    inline NUM_TYPE distance(POINT_REF p2) {
#if USE_FAST_DISTANCE
        return fastDistance(p2);
#else
        return accurateDistance(p2);
#endif
    }
    
    inline NUM_TYPE distance() {
#if USE_FAST_DISTANCE
        return MAX(ABS(x),ABS(y));
#else
        return SQRT(x*x+y*y);
#endif
    }
    
    inline NUM_TYPE fastDistance(POINT_REF p2)
    {
        NUM_TYPE xd = x - p2.x;
        NUM_TYPE yd = y - p2.y;
        
        return MAX(ABS(xd),ABS(yd));
    }
    
    inline NUM_TYPE accurateDistance(POINT_REF p2)
    {
        NUM_TYPE xd = x - p2.x;
        NUM_TYPE yd = y - p2.y;
        
        return SQRT(xd*xd+yd*yd);
    }

    void randomize() {
        x = Random::randRange(0, MAX_COORD);
        y = Random::randRange(0, MAX_COORD);
    }
    
    void randomize(NUM_TYPE min, NUM_TYPE max) {
        x = Random::randRange(min, max);
        y = Random::randRange(min, max);
    }
};

inline NUM_TYPE accurateDistance(NUM_TYPE x1, NUM_TYPE y1, NUM_TYPE x2, NUM_TYPE y2) {
    x1 -= x2;
    y1 -= y2;
    
    return SQRT(x1*x1+y1*y1);
}

inline NUM_TYPE accurateDistance(NUM_TYPE x1, NUM_TYPE y1) {
    return SQRT(x1*x1+y1*y1);
}

#endif
