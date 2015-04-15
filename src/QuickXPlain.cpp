/*
 * quickXPlain.cpp
 *
 *  Created on: 22.03.2015
 *      Author: Benjamin Musitsch
 */

#include "QuickXPlain.h"
#include "Solver.h"
#include "util/Formatter.h"
#include <string>

/**
 * Calculates the preferred conflict out of a number of conflicting literals using the QuickXplain algorithm
 * Used for debugging INCOHERENT programs
 *
 * @param  debugLiterals (input) vector of IDs from literals (_debug literals)
 * @return the preferred conflict (vector of literal IDs)
 */
vector< Literal >
QuickXPlain::minimizeUnsatCore( const vector< Literal >& unsatCore )
{
    trace_msg( debug, 1, "Start minimizing UNSAT core with QuickXPlain ..." );

    if ( unsatCore.empty() )
    {
        trace_msg( debug, 1, "UNSAT core is empty" );
        return unsatCore;
    }
    else
        return minimizeUnsatCore( 1, vector< Literal >(), vector< Literal >(), unsatCore );
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
    trace_msg( debug, level + 1, "QXP level " << level << ": toCheck = " << Formatter::formatClause( toCheck ) << "; added = " << Formatter::formatClause( addedToCheck ) << "; notChecked = " << Formatter::formatClause( toSplit ) );

    if ( !addedToCheck.empty() && solveAndClearWithAssumptions(toCheck) == INCOHERENT )
    {
        trace_msg( debug, level + 1, "QXP level " << level << ": nothing to check and INCOHERENT for toCheck = " << Formatter::formatClause( toCheck ) << " -> prune" );
        return vector< Literal >();
    }
	else if ( toSplit.size() == 1 )
	{
        trace_msg( debug, level + 1, "QXP level " << level << ": only " << Formatter::formatLiteral( toSplit.front() ) << " left -> add to minimal core");
		return toSplit;
	}
	else
	{
	    vector< Literal > firstHalf;
	    vector< Literal > secondHalf;
		partitionVector(toSplit, firstHalf, secondHalf);

        trace_msg( debug, level + 1, "QXP level " << level << ": partition " << Formatter::formatClause( toSplit ) << " into " << Formatter::formatClause( firstHalf ) << " and " << Formatter::formatClause(secondHalf) );

		// first partial result
		vector< Literal > firstResult = minimizeUnsatCore(level + 1, appendToVector(toCheck, firstHalf), firstHalf, secondHalf);

		trace_msg( debug, level + 1, "QXP level " << level << ": first result is " << Formatter::formatClause( firstResult ) );

		// second partial result
		vector< Literal > secondResult = minimizeUnsatCore(level + 1, appendToVector(toCheck, firstResult), firstResult, firstHalf);
		trace_msg( debug, level + 1, "QXP level " << level << ": second result is " << Formatter::formatClause( secondResult ) );

		vector< Literal > result = appendToVector( firstResult, secondResult );
        trace_msg( debug, level + 1, "QXP level " << level << ": add " << Formatter::formatClause( result ) << " to minimal core" );
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
        vector< Literal > assumptionsAND )
{
    vector< Literal > assumptionsOR;
    unsigned int result = solver.solve( assumptionsAND, assumptionsOR );
    solver.unrollToZero();
    solver.clearConflictStatus();
    return result;
}
