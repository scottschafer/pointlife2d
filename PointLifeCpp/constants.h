//
//  constants.h
//  PointLifeCpp
//
//  Created by Scott SSE on 6/25/15.
//  Copyright (c) 2015 Scott SSE. All rights reserved.
//

#ifndef PointLifeCpp_constants_h
#define PointLifeCpp_constants_h

#define WORLD_DIM 100
#define MAX_COORD (WORLD_DIM-1)
#define NUM_CELLS 5000

#define USE_FLOAT 0

#if USE_FLOAT
    #define NUM_TYPE float
    #define SQRT(x) (sqrtf(x))
#else
    #define NUM_TYPE double
    #define SQRT(x) (sqrt(x))
#endif

#define NULL 0
#define MAX_CONNECTIONS 3

#define GENOME_LENGTH 64

#define CELL_SIZE MIN(10,(5*NUM_TYPE(WORLD_DIM)/NUM_TYPE(NUM_CELLS)))
#define INITIAL_VELOCITY (CELL_SIZE/4.0)
#define MAX_VELOCITY (CELL_SIZE*.8)

#define USE_FAST_DISTANCE 1

inline NUM_TYPE MAX(NUM_TYPE x, NUM_TYPE y) { return (x>y) ? x : y; }
inline NUM_TYPE MIN(NUM_TYPE x, NUM_TYPE y) { return (x<y) ? x : y; }
inline NUM_TYPE ABS(NUM_TYPE x) { return (x>0) ? x : -x; }

#ifndef BYTE
typedef unsigned char BYTE;
#endif


#endif
