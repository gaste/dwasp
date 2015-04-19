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

#include "DebugInterface.h"
#include "DebugUserInterface.h"
#include "DebugUserInterfaceCLI.h"
#include "QuickXPlain.h"
#include "Solver.h"
#include "util/Formatter.h"
#include "util/RuleNames.h"
#include <string>

/**
 * gives a list of the AND assumptions added for debugging (added while parsing the input program)
 * assumptionsAND ... the vector for the assumptions
 */
void DebugInterface::computeAssumptionsAnd( vector< Literal >& assumptionsAND )
{
    for( unsigned int i = 0; i < assumptions.size(); i++ )
        assumptionsAND.push_back( assumptions[ i ] );
}

/**
 * gets all literals (IDs) from a clause
 * used to get the literals from the unsatisfiable core
 *
 * @param unsatCore (input) the unsatisfiable core (or a other clause)
 * @return a vector of literal IDs
 */
vector< Literal > DebugInterface::clauseToVector( const Clause& clause )
{
	vector< Literal > debugLiterals;

	for (unsigned int i = 0; i < clause.size(); i++)
	{
		debugLiterals.push_back(clause[i]);
	}

	return debugLiterals;
}

/**
 * the main debugging method
 * tries to solve the input program (with the input assumptions)
 * if it is INCOHERENT the unsatisfiable core is minimized by using the QuickXPlain algorithm (calculate preferred conflict)
 */
void DebugInterface::debug()
{
    Var queryVariable;
    TruthValue queryVariableTruthValue;
    bool continueDebugging = 1;
    
    trace_msg( debug, 1, "Start debugging with _debug assumptions" );

    if ( computeUnsatCore() != INCOHERENT ) {
        ErrorMessage::errorGeneric( "Program not INCOHERENT" );
        return;
    }

    assert( solver.getUnsatCore() != NULL );

    vector< Literal > unsatCore = clauseToVector(*solver.getUnsatCore());

    trace_msg( debug, 1, "INCOHERENT with UNSAT core " << Formatter::formatClause( unsatCore ) );
    vector< Literal > minimalUnsatCore = coreMinimizer.minimizeUnsatCore(unsatCore);
    trace_msg( debug, 1, "minimized UNSAT core: " << Formatter::formatClause( minimalUnsatCore ) );

    do
    {
        switch (userInterface->promptCommand())
        {
        case SHOW_CORE:
            userInterface->printCore( minimalUnsatCore );
            break;
        case ASK_QUERY:
            queryVariable = determineQueryVariable( minimalUnsatCore );
            queryVariableTruthValue = userInterface->askTruthValue( queryVariable );
            break;
        case EXIT:
            continueDebugging = 0;
            break;
        }
    } while(continueDebugging);

	delete userInterface;
}

void
DebugInterface::readDebugMapping( Istream& stream )
{
    bool doneParsing = 0;
    trace_msg( debug, 1, "Parsing debug mapping table" );

    do
    {
        unsigned int type;
        stream.read( type );

        if ( type == DEBUG_MAP_LINE_SEPARATOR )
        {
            doneParsing = 1;
        }
        else if ( type == DEBUG_MAP_ENTRY )
        {
            // format: DEBUG_MAP_ENTRY debugConstant #variables variables rule
            string debugConstant, rule, word;
            size_t numVars = 0;
            vector< string > variables;

            stream.read( debugConstant );
            stream.read( numVars );

            for ( size_t i = 0; i < numVars; i++ )
            {
                string var;
                stream.read( var );
                variables.push_back( var );
            }

            // read the ungrounded rule
            do
            {
                stream.read( word );
                rule += word;

                if ( word[ word.length() - 1 ] != '.' )
                    rule += " ";
            } while ( word[ word.length() - 1 ] != '.' );

            trace_msg( debug, 2, "Adding { " + debugConstant + " -> " + rule + " } to the rule map" );
            RuleNames::addRule( debugConstant, rule, variables );
        }
        else
        {
            ErrorMessage::errorDuringParsing( "Unsupported debug map type" );
        }
    } while ( !doneParsing );
}

unsigned int
DebugInterface::computeUnsatCore()
{
    vector< Literal > assumptionsAND;
    vector< Literal > assumptionsOR;

    computeAssumptionsAnd( assumptionsAND );

    solver.setComputeUnsatCores( true );
    solver.setComputeMinimalUnsatCore( true );

    unsigned int result = solver.solve( assumptionsAND, assumptionsOR );

    solver.unrollToZero();
    solver.clearConflictStatus();

    return result;
}

Var
DebugInterface::determineQueryVariable( const vector< Literal >& unsatCore )
{
    return 0;
}
