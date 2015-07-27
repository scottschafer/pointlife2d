//
//  Genome.h
//  PointLifeCpp
//
//  Created by Scott SSE on 6/29/15.
//  Copyright (c) 2015 Scott SSE. All rights reserved.
//

#ifndef __PointLifeCpp__Genome__
#define __PointLifeCpp__Genome__

#include "constants.h"

class Genome {
    
public:
    Genome();
    
    Genome & operator = (const Genome & src);
    void randomize();
    Genome breed(const Genome &);
    void mutate();
    
    BYTE genome[GENOME_LENGTH];
};

#endif /* defined(__PointLifeCpp__Genome__) */
