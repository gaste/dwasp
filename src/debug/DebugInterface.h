/*
 *
 *  Copyright 2015 Mario Alviano, Carmine Dodaro, Francesco Ricca, and Philip
 *  Gasteiger.
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

#include <time.h>
#include <map>
#include <string>
#include <vector>

#include "../Literal.h"
#include "../util/Constants.h"
#include "DebugUserInterfaceGUI.h"
#include "QuickXPlain.h"

class Istream;

using namespace std;

#define DEBUG_MAP_ENTRY 10
#define DEBUG_MAP_LINE_SEPARATOR 0

class Solver;

/**
  * Used for the debugging of INCOHERENT programs using debug-literals (_debug) to identify the conflicting parts
  */
class DebugInterface
{    
	public:
        inline DebugInterface( Solver& s ) : solver( s ), coreMinimizer( s ), userInterface( new DebugUserInterfaceGUI() ) {}
        void addDebugLiteral( Literal l ) { debugLiterals.push_back( l ); consideredDebugLiterals.push_back( l ); }
        void readDebugMapping( Istream& stream );
        void debug();

    private:
        DebugInterface( const DebugInterface& );
        Var determineQueryVariable( const vector< Literal >& unsatCore );
        Var determineQueryVariableUnfounded( const vector< Literal >& unsatCore );
        Var determineQueryVariableFounded( const vector< Literal >& unsatCore );
        unsigned int determineQueryVariableFounded( const vector< Literal >& unsatCore, map< Var, int >& variableEntropy, const vector< Literal >& relaxedLiterals, unsigned int level, const time_t& startTime );
        vector< Literal > getCoreWithoutAssertions( const vector< Literal >& unsatCore );
        vector< Literal > getCoreAssertions( const vector< Literal >& unsatCore );
        void determineAssertionDebugLiterals();
        void resetSolver();
        unsigned int runSolver( const vector< Literal >& debugAssumptions, const vector< Literal >& assertions );
        bool isUnfoundedCore( const vector< Literal > unsatCore );
        bool saveHistory( const string& filename );
        bool loadHistory( const string& filename );
        bool isFact( const Var variable );
        bool isVariableContainedInLiterals( const Var variable, const vector< Literal >& literals );
        inline bool isDebugVariable( const Var variable ) { return isVariableContainedInLiterals( variable, debugLiterals ); }
        inline bool isAssertion( const Var variable ) { return isVariableContainedInLiterals( variable, assertions ); }
        vector< vector< Literal > > computeDisjointCores();
        vector< Literal > fixCore( const vector< vector< Literal > >& cores );

        Solver& solver;
        QuickXPlain coreMinimizer;
        DebugUserInterface* userInterface;
        vector< Var > facts;
        vector< Literal > debugLiterals;
        vector< Literal > assertionDebugLiterals;
        vector< Literal > assumedAssertions;
        vector< Literal > consideredDebugLiterals;
        vector< Literal > assertions;
};

#endif
