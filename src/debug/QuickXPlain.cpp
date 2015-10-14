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

#include "QuickXPlain.h"

#include <algorithm>
#include <iostream>
#include <iterator>
#include <string>
#include <utility>

#include "../Solver.h"
#include "../util/Constants.h"
#include "../util/Formatter.h"
#include "../util/Trace.h"

vector< Literal >
QuickXPlain::minimizeUnsatCore(
    const Clause& unsatCore,
    unsigned int level )
{
    vector< Literal > coreLiterals;

    for ( unsigned int i = 0; i < unsatCore.size(); i++ )
        coreLiterals.push_back( unsatCore[ i ] );

    return minimizeUnsatCore( coreLiterals, level );
}

/**
 * Calculates the preferred conflict out of a number of conflicting literals using the QuickXplain algorithm
 * Used for debugging INCOHERENT programs
 *
 * @param  debugLiterals (input) vector of IDs from literals (_debug literals)
 * @return the preferred conflict (vector of literal IDs)
 */
vector< Literal >
QuickXPlain::minimizeUnsatCore(
    const vector< Literal >& unsatCore,
    unsigned int level )
{
    trace_msg( debug, level, "Minimizing the UNSAT core with QuickXPlain " );

    if ( unsatCore.empty() )
    {
        trace_msg( debug, level + 1, "UNSAT core is empty" );
        return unsatCore;
    }
    else
    {
        vector< Literal > minimalCore = minimizeUnsatCore( level, vector< Literal >(), vector< Literal >(), unsatCore );
        trace_msg(debug, level + 1, "Minimized core = " + Formatter::formatClause( minimalCore ));
        return minimalCore;
    }
}

/**
 * Calculation of the preferred conflict
 *
 * @param  level        current level in the algorithm
 * @param  toCheck      (input) current part of conflict to be checked (see QXP Paper - Background)
 * @param  addedToCheck (input) conflict added to toCheck in the last step (see QXP Paper - Delta)
 * @param  toSplit      (input) not considered conlicts (see QXP Paper - Constraints)
 * @return (part of a) preferred conflict
 */
vector< Literal >
QuickXPlain::minimizeUnsatCore(
        unsigned int level,
        const vector< Literal >& toCheck, const vector< Literal >& addedToCheck,
        const vector< Literal >& toSplit )
{
    trace_msg( debug, level + 1, "ToCheck = " << Formatter::formatClause( toCheck ) << "; added = " << Formatter::formatClause( addedToCheck ) << "; notChecked = " << Formatter::formatClause( toSplit ) );

    if ( !addedToCheck.empty() && solveAndClearWithAssumptions(toCheck) == INCOHERENT )
    {
        trace_msg( debug, level + 1, "Nothing to check and INCOHERENT for toCheck = " << Formatter::formatClause( toCheck ) << " -> prune" );
        return vector< Literal >();
    }
	else if ( toSplit.size() == 1 )
	{
        trace_msg( debug, level + 1, "Only " << Formatter::formatLiteral( toSplit.front() ) << " left -> add to minimal core");
		return toSplit;
	}
	else
	{
	    vector< Literal > firstHalf;
	    vector< Literal > secondHalf;
		partitionVector(toSplit, firstHalf, secondHalf);

        trace_msg( debug, level + 1, "Partition " << Formatter::formatClause( toSplit ) << " into " << Formatter::formatClause( firstHalf ) << " and " << Formatter::formatClause( secondHalf ) );

		vector< Literal > secondResult = minimizeUnsatCore( level + 1, appendToVector( toCheck, firstHalf ), firstHalf, secondHalf );
		vector< Literal > firstResult = minimizeUnsatCore( level + 1, appendToVector( toCheck, secondResult ), secondResult, firstHalf );

        trace_msg( debug, level + 1, "First result is " << Formatter::formatClause( firstResult ) );
		trace_msg( debug, level + 1, "Second result is " << Formatter::formatClause( secondResult ) );

		vector< Literal > result = appendToVector( secondResult, firstResult );
        trace_msg( debug, level + 1, "Adding " << Formatter::formatClause( result ) << " to the minimal core" );
		return result;
	}
}

/**
 * splits the given vector in two equal sized parts
 * used in quickXPlainIntern
 *
 * @param toSplit (input)  vector to split
 * @param v1      (output) first half of the vector
 * @param v2      (output) second half of the vector
 */
void
QuickXPlain::partitionVector(
        const vector< Literal >& toSplit,
		vector< Literal >& v1, vector< Literal >& v2)
{
    unsigned int splitAt = (toSplit.size() + 2 - 1) / 2;

    for ( unsigned int i = 0; i < splitAt; i++ )
        v1.push_back( toSplit[i] );

    for ( unsigned int i = splitAt; i < toSplit.size(); i++ )
        v2.push_back( toSplit[i] );
}

/**
 * combines the two given vectors into one (no duplicated elements)
 *
 * @param v1 (input) the first vector
 * @param v2 (input) the second vector
 */
vector< Literal >
QuickXPlain::appendToVector(
        const vector< Literal >& v1,
        const vector< Literal >& v2 )
{
    vector< Literal > added;

    for ( unsigned int i = 0; i < v1.size(); i++ )
        added.push_back( v1[i] );

    for ( unsigned int i = 0; i < v2.size(); i++ )
    {
        if ( find( v1.begin(), v1.end(), v2[i] ) == v1.end() )
            added.push_back( v2[i] );
    }

    return added;
}

/**
 * solve the program (with the given assumptions) using the solver instance from the constructor
 * unrolls and clears the conflict status afterwards
 */
unsigned int
 QuickXPlain::solveAndClearWithAssumptions(
        vector< Literal > assumptions )
{
    unsigned int result = solver.solve( assumptions );
    solver.unrollToZero();
    solver.clearConflictStatus();
    return result;
}
