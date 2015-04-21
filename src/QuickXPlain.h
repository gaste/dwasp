/*
 *
 *  Copyright 2013 Mario Alviano, Carmine Dodaro, and Francesco Ricca.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 */

#ifndef QUICKXPLAIN_H
#define QUICKXPLAIN_H

#include <vector>

using namespace std;

#include "Literal.h"

class Solver;

/**
 * QuickXPlain (QXP) algorithm to determine preferred conflicts
 * if INCOHERENT. For details on the algorithm see
 * Junker 2004; QuickXPlain - Preferred Explanations and Relaxations over constraint-based problems
 */
class QuickXPlain
{
    public:
        inline QuickXPlain( Solver& s ) : solver( s ) { }
        vector< Literal > minimizeUnsatCore( const vector< Literal >& unsatCore, unsigned int level = 1 );

    private:
        vector< Literal > minimizeUnsatCore( unsigned int level, const vector< Literal >& toCheck, const vector< Literal >& addedToCheck, const vector< Literal >& toSplit );

        // utility methods for vectors used in QXP
        void partitionVector( const vector< Literal >& toSplit, vector< Literal >& v1, vector< Literal >& v2 );
        vector< Literal > appendToVector( const vector< Literal >& v, const vector< Literal >& toAppend );

        // solve with given assumptions and clear status afterwards
        unsigned int solveAndClearWithAssumptions(vector< Literal > assumptionsAND);

        Solver& solver;
};

#endif
