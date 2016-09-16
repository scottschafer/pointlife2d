//
//  constants.h
//  PointLifeCpp
//
//  Created by Scott SSE on 6/25/15.
//  Copyright (c) 2015 Scott SSE. All rights reserved.
//

#include "CellTypes.h"

#ifndef PointLifeCpp_constants_h
#define PointLifeCpp_constants_h

// how many turns does a contraction take?
#define CONTRACT_TURN_COUNT 6

// default action frequency
#define DEFAULT_ACTION_FREQUENCY 30


#define NUM_CONNECTION_PHYSICS_ITERATIONS 5
#define INACTIVE_AFTER_BITE_COUNT 20
#define VELOCITY_DAMPING 1

#define ALLOW_DEATH 0

#define TURN_COST 0.0001
#define BITE_GAIN 6
#define BITE_COST 6
#define MOVE_ENERGY .002
#define FLAGELLUM_TURNS 20

#define MAX_VELOCITY (CELL_SIZE*.4)

#define BATCH_MOVE_POINTS 0
#define USE_SIMULATION_THREAD 1

#define WORLD_DIM 500
#define MAX_COORD (WORLD_DIM-1)
#define NUM_CELLS 10000

#define BASE_ENERGY_PER_CELL 3

#define NUM_GENOMES_TO_TEST 100
#define NUM_GENOMES_TO_KEEP (NUM_GENOMES_TO_TEST/5)
#define NUM_TURNS_PER_GENOME 1500
#define NUM_MUTATIONS (NUM_GENOMES_TO_TEST)

#define USE_FLOAT 0
#define USE_FAST_DISTANCE 0

#if USE_FLOAT
    #define NUMBER float
    #define SQRT(x) (sqrtf(x))
#else
    #define NUMBER double
    #define SQRT(x) (sqrt(x))
#endif

#define NULL 0
#define MAX_CONNECTIONS 6

#define GENOME_LENGTH 512

#define CELL_SIZE MIN(10,(15*NUMBER(WORLD_DIM)/NUMBER(NUM_CELLS)))
#define INITIAL_VELOCITY (CELL_SIZE/4.0)

inline NUMBER MAX(NUMBER x, NUMBER y) { return (x>y) ? x : y; }
inline NUMBER MIN(NUMBER x, NUMBER y) { return (x<y) ? x : y; }
inline NUMBER ABS(NUMBER x) { return (x>0) ? x : -x; }

#ifndef BYTE
typedef unsigned char BYTE;
#endif


#endif
