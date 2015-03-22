/*
 * quickXPlain.cpp
 *
 *  Created on: 22.03.2015
 *      Author: Benjamin Musitsch
 */

#include "QuickXPlain.h"
#include "Solver.h"
#include <string>

// Calculates the preferred conflict out of a number of conflicting literals using the QuickXplain algorithm
// Used for debugging INCOHERENT programs
// 		debugLiterals ... vector of IDs from literals (_debug literals)
//	returns the preferred conflict (vector of literal IDs)
vector< unsigned int > QuickXPlain::quickXPlain(vector< unsigned int > debugLiterals)
{
	trace_msg (debug, 0, "Start quickXPlain");
	vector< unsigned int > empty;

	if ( debugLiterals.empty() )
	{
		trace_msg (debug, 0, "No conflicting literals");
		return empty;
	}
	else
		return quickXPlainIntern(0, empty, empty, debugLiterals);
}

// Calculation of the preferred conflict
//		level        ... current level in the algorithm
//		toCheck      ... current part of conflict to be checked (see QXP Paper - Background)
//		addedToCheck ... conflict added to toCheck in the last step (see QXP Paper - Delta)
//		toSplit      ... not considered conlicts (see QXP Paper - Constraints)
// return (part of a) preferred conflict
vector< unsigned int > QuickXPlain::quickXPlainIntern(int level, vector< unsigned int > toCheck,
		vector< unsigned int > addedToCheck, vector< unsigned int > toSplit)
{
	// empty vector to return
	vector< unsigned int > empty;

	// vector for both half of the splittet vector toSplit
	vector< unsigned int > firstHalf;
	vector< unsigned int > secondHalf;

	// partial results
	vector< unsigned int > result1;
	vector< unsigned int > result2;

	// result
	vector< unsigned int > result;

	trace_msg( debug, level+1, "QXP lvl " << level << " with " << vectorToString(toCheck) << " to check (added " << vectorToString(addedToCheck) <<
			") and " << vectorToString(toSplit) << " not checked");

	// assembly assumption for the next check
	vector< Literal > assumptionsAND;
	vector< Literal > assumptionsOR;
	for (unsigned int i = 0; i < toCheck.size(); i++)
	{
		assumptionsAND.push_back( Literal( toCheck[i]) );
	}

	if ( addedToCheck.empty() && solveAndClearWithAssumptions(assumptionsAND, assumptionsOR) == INCOHERENT)
	{
		trace_msg( debug, level+1, "QXP lvl " << level << ": nothing to check and INCOHERENT -> prune");
		return empty;
	}
	else if ( toSplit.size() == 1 )
	{
		trace_msg( debug, level+1, "QXP lvl " << level << ": only " << toSplit.front() << " left -> add to preferred conflict");
		return toSplit;
	}
	else
	{
		// split vector and calculate partial results
		vectorSplit(toSplit, firstHalf, secondHalf);

		trace_msg( debug, level+1, "QXP lvl " << level << ": split " << vectorToString(toSplit) << " into " << vectorToString(firstHalf) <<
				" and " << vectorToString(secondHalf) );

		// first partial result
		result2 = quickXPlainIntern(level + 1, vectorAdd(toCheck, firstHalf), firstHalf, secondHalf);
		trace_msg (debug, level+1, "QXP lvl " << level << ": first result is " << vectorToString(result2));

		// second partial result
		result1 = quickXPlainIntern(level + 1, vectorAdd(toCheck, result2), result2, firstHalf);
		trace_msg (debug, level+1, "QXP lvl " << level << ": second result is " << vectorToString(result1));

		result = vectorAdd(result1, result2);
		trace_msg (debug, level+1, "QXP lvl " << level << ": add " << vectorToString(result) << " to preferred conflict");
		return result;
	}
}

// split the given vector in two equal sized parts
// used in quickXPlainIntern
//		toSplit ... vector to split
//		v1      ... first half of the vector
//		v2      ... second half of the vector
void QuickXPlain::vectorSplit(vector< unsigned int > toSplit,
		vector< unsigned int >& v1, vector< unsigned int >& v2)
{
	unsigned int splitAt = (toSplit.size() + 2 - 1) / 2;

	for (unsigned int i = 0; i < splitAt; i++)
		v1.push_back(toSplit[i]);

	for (unsigned int i = splitAt; i < toSplit.size(); i++)
		v2.push_back(toSplit[i]);
}

// combines the two given vectors into one (no duplicated elements)
//		v1 ... the first vector
//		v2 ... the second vector
vector< unsigned int > QuickXPlain::vectorAdd(vector< unsigned int > v1, vector< unsigned int > v2)
{
	std::vector<unsigned int>::iterator it;
	for (unsigned int i = 0; i < v2.size(); i++)
	{
		it = find(v1.begin(), v1.end(), v2[i]);

		if (it == v1.end())
			v1.push_back(v2[i]);
	}

	return v1;
}

// constructs a string of the given vector (form: [ element_1 | ... | element_n ])
//		v ... the vector
//  returns the vector as string
string QuickXPlain::vectorToString(vector < unsigned int > v)
{
	string s = "[";
	if (!v.empty())
	{
		s += " " + std::to_string((unsigned int)v.at(0));

		for (unsigned int i = 1; i < v.size(); i++)
				s += " | " + std::to_string((unsigned int)v.at(i));
	}
	s += " ]";
	return s;
}

// solve the program (with the given assumptions) using the solver instance from the constructor
// unrolls and clears the conflict status afterwards
unsigned int QuickXPlain::solveAndClearWithAssumptions(vector< Literal >& assumptionsAND, vector< Literal >& assumptionsOR)
{
	unsigned int result = solver.solve(assumptionsAND, assumptionsOR);
	solver.unrollToZero();
	solver.clearConflictStatus();
	return result;
}



