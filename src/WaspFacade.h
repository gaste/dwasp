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

#ifndef WASPFACADE_H
#define WASPFACADE_H

#include <cassert>
#include <vector>
#include <unordered_map>
#include <fstream>
using namespace std;

#include "util/Constants.h"
#include "Solver.h"
#include "DebugInterface.h"
#include "input/Dimacs.h"
#include "weakconstraints/WeakInterface.h"
#include "weakconstraints/Bcd.h"
#include "weakconstraints/Mgd.h"
#include "weakconstraints/Oll.h"
#include "weakconstraints/Opt.h"
#include "weakconstraints/PMRes.h"
#include "weakconstraints/OllBB.h"

class WaspFacade
{
    public:
        inline WaspFacade();
        inline ~WaspFacade(){ delete debugInterface; }
        
        void readInput();
        void solve();
        inline void onFinish() { solver.onFinish(); }
        inline void onKill() { solver.onKill(); }
        
        inline void greetings(){ solver.greetings(); }
        
        void setDeletionPolicy( DELETION_POLICY, unsigned int deletionThreshold );
        void setDecisionPolicy( DECISION_POLICY, unsigned int heuristicLimit );
        void setOutputPolicy( OUTPUT_POLICY );
        void setRestartsPolicy( RESTARTS_POLICY, unsigned int threshold );

        inline void setMaxModels( unsigned int max ) { maxModels = max; }
        inline void setPrintProgram( bool printProgram ) { this->printProgram = printProgram; }
        inline void setPrintDimacs( bool printDimacs ) { this->printDimacs = printDimacs; }
        void setExchangeClauses( bool exchangeClauses ) { solver.setExchangeClauses( exchangeClauses ); }                
        
        inline void setWeakConstraintsAlgorithm( WEAK_CONSTRAINTS_ALG alg ) { weakConstraintsAlg = alg; }
        inline void setDisjCoresPreprocessing( bool value ) { disjCoresPreprocessing = value; }
        inline void setMinimizeUnsatCore( bool value ) { solver.setMinimizeUnsatCore( value ); }
        inline void setStratification( bool value ) { stratification = value; }
        
        inline unsigned int solveWithWeakConstraints();        

        inline void enableDebug( string debugFilename );

    private:
        Solver solver;
        DebugInterface* debugInterface;
        istream* inputStream;

        unsigned int numberOfModels;
        unsigned int maxModels;
        bool printProgram;
        bool printDimacs;

        WEAK_CONSTRAINTS_ALG weakConstraintsAlg;
        bool disjCoresPreprocessing;
        bool stratification;
};

WaspFacade::WaspFacade()
: debugInterface( NULL ),
  inputStream( &cin ),
  numberOfModels( 0 ),
  maxModels( 1 ),
  printProgram( false ),
  printDimacs( false ),
  weakConstraintsAlg( OPT ),
  disjCoresPreprocessing( false ),
  stratification( true )
{
}

void
WaspFacade::enableDebug(
    string debugFilename )
{
    if( debugFilename.length() != 0 )
    {
        inputStream = new ifstream( debugFilename );

        if ( !inputStream->good() )
            ErrorMessage::errorDuringParsing( "Could not open the debug input file '" + debugFilename + "'" );

        trace_msg( debug, 0, "Using file '" << debugFilename << "' as input for the logic program." );

        debugInterface = new DebugInterface( solver );
    }
}

unsigned int
WaspFacade::solveWithWeakConstraints()
{    
    WeakInterface* w = NULL;    
    switch( weakConstraintsAlg )
    {
        case BCD:
            w = new Bcd( solver );      
            break;

        case MGD:
            w = new Mgd( solver );
            break;

        case OPT:
            w = new Opt( solver );
            break;

        case BB:
            w = new Opt( solver, true );
            break;

        case PMRES:            
            w = new PMRes( solver );
            break;

        case OLLBB:
            w = new OllBB( solver );
            break;

        case OLLBBREST:
            w = new OllBB( solver, true );
            break;

        case MGDOLL:
            if( solver.isWeighted() )
                w = new Mgd( solver );
            else
                w = new Oll( solver );                
            break;

        case OLL:
        default:            
            w = new Oll( solver );
            break;
    }
    
    if( weakConstraintsAlg != OLLBB && weakConstraintsAlg != OLLBBREST )
        solver.simplifyOptimizationLiteralsAndUpdateLowerBound( w );
    w->setDisjCoresPreprocessing( disjCoresPreprocessing );
    w->setStratification( stratification );
    unsigned int res = w->run();    
    delete w;
    return res;
}

#endif
