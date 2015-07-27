//
//  Genome.cpp
//  PointLifeCpp
//
//  Created by Scott SSE on 6/29/15.
//  Copyright (c) 2015 Scott SSE. All rights reserved.
//

#include "Genome.h"
#include "Random.h"
#include <cstring>

Genome :: Genome() {
}

Genome & Genome :: operator = (const Genome & src) {
    memcpy(genome, src.genome, sizeof(genome));
    return *this;
}

void Genome :: randomize() {
    for (int i = 0; i < GENOME_LENGTH; i++) {
        genome[i] = Random::randRange(0, 256);
    }
}

Genome Genome :: breed(const Genome & src) {
    Genome g;
    for (int i = 0; i < GENOME_LENGTH; i++) {
        int j = Random::randRange(0, 2);
        g.genome[i] = j ? genome[i] : src.genome[i];
    }
    return g;
}

void Genome :: mutate() {
    int i = Random::randRange(0, GENOME_LENGTH);
    genome[i] = Random::randRange(0, 256);
}
