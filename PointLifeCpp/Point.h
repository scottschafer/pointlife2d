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
//#define POINT_REF const Point &

class Point {
public:
    NUMBER x, y;

    Point() {
        x = y = 0;
    }
    
    Point(NUMBER x, NUMBER y) : x(x), y(y) {
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
    
    NUMBER squaredDistance(POINT_REF p2) {
        NUMBER x = this->x - p2.x;
        NUMBER y = this->y - p2.y;
        return x * x + y * y;
    }
        
    NUMBER dot(POINT_REF p2) {
        return x * p2.x + y * p2.y;
    }
    
    inline NUMBER distance(POINT_REF p2) {
#if USE_FAST_DISTANCE
        return fastDistance(p2);
#else
        return accurateDistance(p2);
#endif
    }
    
    inline NUMBER distance() {
#if USE_FAST_DISTANCE
        return MAX(ABS(x),ABS(y));
#else
        return SQRT(x*x+y*y);
#endif
    }
    
    inline NUMBER fastLength()
    {
        return MAX(ABS(x),ABS(y));
    }

    inline NUMBER fastDistance(POINT_REF p2)
    {
        NUMBER xd = x - p2.x;
        NUMBER yd = y - p2.y;
        
        return MAX(ABS(xd),ABS(yd));
    }
    
    inline NUMBER accurateDistance(POINT_REF p2)
    {
        NUMBER xd = x - p2.x;
        NUMBER yd = y - p2.y;
        
        return SQRT(xd*xd+yd*yd);
    }

    void randomize() {
        x = Random::randRange(0, MAX_COORD);
        y = Random::randRange(0, MAX_COORD);
    }
    
    void randomize(NUMBER min, NUMBER max) {
        x = Random::randRange(min, max);
        y = Random::randRange(min, max);
    }
};

inline NUMBER accurateDistance(NUMBER x1, NUMBER y1, NUMBER x2, NUMBER y2) {
    x1 -= x2;
    y1 -= y2;
    
    return SQRT(x1*x1+y1*y1);
}

inline NUMBER accurateDistance(NUMBER x1, NUMBER y1) {
    return SQRT(x1*x1+y1*y1);
}

#endif
