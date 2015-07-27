//
//  constants.h
//  PointLifeCpp
//
//  Created by Scott SSE on 6/25/15.
//  Copyright (c) 2015 Scott SSE. All rights reserved.
//

#ifndef PointLifeCpp_constants_h
#define PointLifeCpp_constants_h

#define BATCH_MOVE_POINTS 0
#define USE_SIMULATION_THREAD 1

#define WORLD_DIM 500
#define MAX_COORD (WORLD_DIM-1)
#define NUM_CELLS 5000

#define NUM_GENOMES_TO_TEST 100
#define NUM_GENOMES_TO_KEEP (NUM_GENOMES_TO_TEST/5)
#define NUM_TURNS_PER_GENOME 700
#define NUM_MUTATIONS (NUM_GENOMES_TO_TEST)

#define USE_FLOAT 0
#define USE_FAST_DISTANCE 1

#if USE_FLOAT
    #define NUMBER float
    #define SQRT(x) (sqrtf(x))
#else
    #define NUMBER double
    #define SQRT(x) (sqrt(x))
#endif

#define NULL 0
#define MAX_CONNECTIONS 6

#define GENOME_LENGTH 1024

#define CELL_SIZE MIN(10,(5*NUMBER(WORLD_DIM)/NUMBER(NUM_CELLS)))
#define INITIAL_VELOCITY (CELL_SIZE/4.0)

inline NUMBER MAX(NUMBER x, NUMBER y) { return (x>y) ? x : y; }
inline NUMBER MIN(NUMBER x, NUMBER y) { return (x<y) ? x : y; }
inline NUMBER ABS(NUMBER x) { return (x>0) ? x : -x; }

#ifndef BYTE
typedef unsigned char BYTE;
#endif


#endif
