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

#ifndef DEBUGINTERFACE_H
#define DEBUGINTERFACE_H

#include <vector>
using namespace std;

#include "Literal.h"
#include "Clause.h"
#include "QuickXPlain.h"
#include "DebugUserInterface.h"
#include "DebugUserInterfaceCLI.h"

class Solver;

/**
  * Used for the debugging of INCOHERENT programs using debug-literals (_debug) to identify the conflicting parts
  */
class DebugInterface
{    
	public:
        inline DebugInterface( Solver& s ) : solver( s ), coreMinimizer( s ), userInterface( new DebugUserInterfaceCLI() ) {}
        void addAssumption( Literal l ) { assumptions.push_back( l ); }
        void debug();

    private:
        DebugInterface( const DebugInterface& );
        void computeAssumptionsAnd( vector< Literal >& assumptionsAND );
        Var determineQueryVariable( const vector< Literal >& unsatCore );
        vector< Literal > clauseToVector( const Clause& unsatCore );
        unsigned int computeUnsatCore();

        Solver& solver;
        QuickXPlain coreMinimizer;
        DebugUserInterface* userInterface;
        vector< Literal > assumptions;
};

#endif
