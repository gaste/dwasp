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
#include "QuickXPlain.h"
#include "Solver.h"
#include <string>

// gives a list of the AND assumptions added for debugging (added while parsing the input program)
//		assumptionsAND ... the vector for the assumptions
void DebugInterface::computeAssumptionsAnd( vector< Literal >& assumptionsAND )
{
    for( unsigned int i = 0; i < assumptions.size(); i++ )
        assumptionsAND.push_back( assumptions[ i ] );
}

// creates a string containing all literals stated in the given vector
// form: [ literal (id) | ... | literal (id)]
//		literalIds ... IDs of the literals
//		withID	   ... considers also the id of the literal when the creating the string if true
//  the string with the literals
string DebugInterface::literalsToString(vector< unsigned int > literalIds, bool withId)
{
	string s = "[ ";

	if ( !literalIds.empty() )
	{
		s += VariableNames::getName(literalIds[0]);
		if (withId)
			s += " (ID " + std::to_string( literalIds[0] ) + ")";

		s += " ";

		for (unsigned int i = 1; i < literalIds.size(); i++)
		{
			s += "| " + VariableNames::getName(literalIds[i]);

			if (withId)
				s += " (ID " + std::to_string( literalIds[i] ) + ")";

			s += " ";
		}
	}

	return s += "]";
}

// gets all literals (IDs) from a clause
// used to get the literals from the unsatisfiable core
//		unsatCore ... the unsatisfiable core (or a other clause)
//  return a vector of literal IDs
vector< unsigned int > DebugInterface::getDebugLiterals( const Clause& unsatCore )
{
	vector< unsigned int > debugLiterals;

	for (unsigned int i = 0; i < unsatCore.size(); i++)
	{
		debugLiterals.push_back(unsatCore.getAt(i).getId());
	}

	return debugLiterals;
}

// the main debugging method
// tries to solve the input program (with the input assumptions)
// if it is INCOHERENT the unsatisfiable core is minimized by using the QuickXPlain algorithm (calculate preferred conflict)
void DebugInterface::debug()
{
	// result from the solver
	unsigned int result;

	QuickXPlain qxp (solver);

	// assumptions for the solver
    vector< Literal > assumptionsAND;
    vector< Literal > assumptionsOR;

    // temporary for the QXP
    vector< unsigned int > debugLiterals;
    vector< unsigned int > preferredConflict;
    
    trace_msg( debug, 0, "Start debugging with _debug assumptions" );

    computeAssumptionsAnd( assumptionsAND );
    solver.setComputeUnsatCores( true );
    solver.setComputeMinimalUnsatCore( true );

    result = solver.solve(assumptionsAND, assumptionsOR);
	solver.unrollToZero();
	solver.clearConflictStatus();

	if (result == INCOHERENT)
	{
		assert( solver.getUnsatCore() != NULL );
		const Clause& unsatCore = *( solver.getUnsatCore() );

		debugLiterals = getDebugLiterals(unsatCore);

		trace_msg( debug, 0, "INCOHERENT with core " << literalsToString( debugLiterals, true ) );

		preferredConflict = qxp.quickXPlain(debugLiterals);

		trace_msg( debug, 0, "preferred conflict found: " << literalsToString( preferredConflict, true ) );
	}
	else
	{
		trace_msg( debug, 0, "not INCOHERENT" );
	}
}
