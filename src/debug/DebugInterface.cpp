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

#include "DebugInterface.h"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <iterator>
#include <utility>

#include "../Clause.h"
#include "../Solver.h"
#include "../util/ErrorMessage.h"
#include "../util/Formatter.h"
#include "../util/Istream.h"
#include "../util/RuleNames.h"
#include "../util/Trace.h"
#include "../util/VariableNames.h"
#include "DebugUserInterface.h"

#define QUERY_DETERMINATION_TIMEOUT 3

bool
DebugInterface::isVariableContainedInLiterals(
    const Var variable,
    const vector< Literal >& literals )
{
    for ( const Literal& literal : literals )
        if ( literal.getVariable() == variable )
            return true;

    return false;
}

bool
DebugInterface::isFact(
    const Var variable )
{
    return find( facts.begin(), facts.end(), variable ) != facts.end();
}

vector< Literal >
DebugInterface::getCoreWithoutAssertions(
    const vector< Literal >& unsatCore )
{
    vector< Literal > coreWithoutAssertions;

    for ( const Literal& coreLiteral : unsatCore )
    {
        if ( !isVariableContainedInLiterals( coreLiteral.getVariable(), assertions )
          && !isVariableContainedInLiterals( coreLiteral.getVariable(), assertionDebugLiterals ))
        {
            coreWithoutAssertions.push_back( coreLiteral );
        }
    }

    return coreWithoutAssertions;
}

vector< Literal >
DebugInterface::getCoreAssertions(
    const vector< Literal >& unsatCore )
{
    vector< Literal > coreAssertions;

    for ( const Literal& coreLiteral : unsatCore )
    {
        if ( isVariableContainedInLiterals( coreLiteral.getVariable(), assertions ) )
        {
            // ensure correct sign
            for ( const Literal& assertion : assertions )
                if ( assertion.getVariable() == coreLiteral.getVariable() )
                    coreAssertions.push_back( assertion );
        }
        else if ( isVariableContainedInLiterals( coreLiteral.getVariable(), assertionDebugLiterals ) )
        {
            unsigned int sign = RuleNames::getGroundRule( coreLiteral ).find( "not " ) == string::npos ? NEGATIVE : POSITIVE;
            coreAssertions.push_back( Literal( RuleNames::getVariables( coreLiteral )[ 0 ], sign ) );
        }
    }

    return coreAssertions;
}

void
DebugInterface::debug()
{
    bool continueDebugging = true;
    
    trace_msg( debug, 1, "Start debugging with _debug assumptions" );

    userInterface->greetUser();
    userInterface->informSolving();

    if ( runSolver( consideredDebugLiterals, assertions ) != INCOHERENT )
    {
        userInterface->informProgramCoherent();
        return;
    }


    trace_msg( debug, 1, "Determining facts" );

    resetSolver();

    for( Var variable = 1; variable <= solver.numberOfVariables(); variable++ )
    {
        if ( !solver.isUndefined( variable ) )
        {
            trace_msg( debug, 2, "Fact: " << VariableNames::getName( variable ) << " = " << ( solver.isTrue( variable ) ? "true" : "false" ));
            facts.push_back( variable );
        }
    }

    determineAssertionDebugLiterals();

    assert( solver.getUnsatCore() != NULL );

    vector< Literal > minimalUnsatCore = coreMinimizer.minimizeUnsatCore( *solver.getUnsatCore() );
    
    do
    {
        switch (userInterface->promptCommand())
        {
        case SHOW_CORE:
            userInterface->printCore( getCoreWithoutAssertions( minimalUnsatCore ), getCoreAssertions( minimalUnsatCore ) );
            break;
        case SHOW_CORE_GROUND_RULES:
            userInterface->printCoreGroundRules( getCoreWithoutAssertions( minimalUnsatCore ), getCoreAssertions( minimalUnsatCore ) );
            break;
        case SHOW_CORE_NONGROUND_RULES:
            userInterface->printCoreUngroundRules( getCoreWithoutAssertions( minimalUnsatCore ), getCoreAssertions( minimalUnsatCore ) );
            break;
        case SHOW_HISTORY:
            userInterface->printHistory( assertions );
            break;
        case ANALYZE_DISJOINT_CORES:
        {
            userInterface->informSolving();
            vector< vector< Literal > > cores = computeDisjointCores();
            minimalUnsatCore = fixCore( cores );
            userInterface->informAnalyzedDisjointCores( cores.size() );
            break;
        }
        case SAVE_HISTORY:
        {
            string filename = userInterface->askHistoryFilename();

            if ( saveHistory( filename ) )
                userInterface->informSavedHistory( filename );
            else
                userInterface->informCouldNotSaveHistory( filename );
            break;
        }
        case LOAD_HISTORY:
        {
            string filename = userInterface->askHistoryFilename();

            if ( loadHistory( filename ) )
            {
                userInterface->informLoadedHistory( filename );
                userInterface->informSolving();
                if ( runSolver( consideredDebugLiterals, assertions ) == INCOHERENT )
                {
                    resetSolver();
                    assert( solver.getUnsatCore() != NULL );
                    minimalUnsatCore = coreMinimizer.minimizeUnsatCore( *solver.getUnsatCore() );
                }
                else
                {
                    userInterface->informProgramCoherent();
                    continueDebugging = false;
                }
            }
            else
            {
                userInterface->informCouldNotLoadHistory( filename );
            }

            break;
        }
        case ASSERT_VARIABLE:
        {
            for ( const Literal& assertion : userInterface->getAssertions() )
            {
                if ( isAssertion( assertion.getVariable() ) )
                {
                    userInterface->informAssertionAlreadyPresent( VariableNames::getName( assertion.getVariable() ) );
                }
                else if ( isFact( assertion.getVariable() ) )
                {
                    userInterface->informAssertionIsFact( VariableNames::getName( assertion.getVariable() ) );
                }
                else
                {
                    assertions.push_back( assertion );
                }
            }

            userInterface->informSolving();

            if ( runSolver( consideredDebugLiterals, assertions ) == INCOHERENT )
            {
                resetSolver();
                assert( solver.getUnsatCore() != NULL );
                minimalUnsatCore = coreMinimizer.minimizeUnsatCore( *solver.getUnsatCore() );
            }
            else
            {
                userInterface->informProgramCoherent();
                continueDebugging = false;
            }

//            Literal assertion = userInterface->getAssertion();
//
//            if ( isAssertion( assertion.getVariable() ) )
//            {
//                userInterface->informAssertionAlreadyPresent( VariableNames::getName( assertion.getVariable() ) );
//            } else if ( isFact( assertion.getVariable() ) )
//            {
//                userInterface->informAssertionIsFact( VariableNames::getName( assertion.getVariable() ) );
//            }
//            else
//            {
//                assertions.push_back( assertion );
//
//                userInterface->informSolving();
//
//                if ( runSolver( consideredDebugLiterals, assertions ) == INCOHERENT )
//                {
//                    resetSolver();
//                    assert( solver.getUnsatCore() != NULL );
//                    minimalUnsatCore = coreMinimizer.minimizeUnsatCore( *solver.getUnsatCore() );
//                }
//                else
//                {
//                    userInterface->informProgramCoherent();
//                    continueDebugging = false;
//                }
//            }
            break;
        }
        case ASK_QUERY:
        {
            userInterface->informComputingQueryVariable();
            vector< Var > queryVariables = determineQueryVariable( minimalUnsatCore );

            userInterface->queryResponse( queryVariables );

//            TruthValue queryVariableTruthValue = UNDEFINED;
//
//            if ( queryVariables.empty() )
//            {
//                userInterface->informNoQueryPossible();
//            }
//            else
//            {
//                queryVariableTruthValue = userInterface->askTruthValue( queryVariables[ 0 ] );
//            }
//
//
//            if ( queryVariableTruthValue != UNDEFINED )
//            {
//                resetSolver();
//                Literal assertion( queryVariables[ 0 ], queryVariableTruthValue == TRUE ? POSITIVE : NEGATIVE );
//                assertions.push_back( assertion );
//
//                userInterface->informSolving();
//
//                if ( runSolver( consideredDebugLiterals, assertions ) == INCOHERENT )
//                {
//                    resetSolver();
//                    assert( solver.getUnsatCore() != NULL );
//                    minimalUnsatCore = coreMinimizer.minimizeUnsatCore( *solver.getUnsatCore() );
//                }
//                else
//                {
//                    userInterface->informProgramCoherent();
//                    continueDebugging = false;
//                }
//            }
            break;
        }
        case UNDO_ASSERTION:
        {
            unsigned int undo = userInterface->chooseAssertionToUndo( assertions );

            if ( undo < assertions.size() )
            {
                assertions.erase( assertions.begin() + undo );

                if ( runSolver( consideredDebugLiterals, assertions ) == INCOHERENT )
                {
                    resetSolver();
                    assert( solver.getUnsatCore() != NULL );
                    minimalUnsatCore = coreMinimizer.minimizeUnsatCore( *solver.getUnsatCore() );
                }
                else
                {
                    userInterface->informProgramCoherent();
                    continueDebugging = false;
                }
            }
            break;
        }
        case EXIT:
            continueDebugging = false;
            break;
        }
    } while(continueDebugging);

	delete userInterface;
}

void
DebugInterface::readDebugMapping(
    Istream& stream )
{
    bool doneParsing = false;
    trace_msg( debug, 1, "Parsing debug mapping table" );

    do
    {
        unsigned int type;
        stream.read( type );

        if ( type == DEBUG_MAP_LINE_SEPARATOR )
        {
            doneParsing = true;
        }
        else if ( type == DEBUG_MAP_ENTRY )
        {
            // format: DEBUG_MAP_ENTRY debugConstant #variables variables rule
            string debugConstant;
            unsigned int numVars = 0;
            vector< string > variables;

            stream.read( debugConstant );
            stream.read( numVars );

            for ( unsigned int i = 0; i < numVars; i++ )
            {
                string var;
                stream.read( var );
                variables.push_back( var );
            }

            string rule, word;
            do
            {
                stream.read( word );
                rule += word;

                if ( word[ word.length() - 1 ] != '.' )
                    rule += " ";
            } while ( word[ word.length() - 1 ] != '.' ); // rule is delimited by an '.'

            trace_msg( debug, 2, "Adding { " << debugConstant << " -> " << rule << " } to the rule map" );
            RuleNames::addRule( debugConstant, rule, variables );
        }
        else
        {
            ErrorMessage::errorDuringParsing( "Unsupported debug map type" );
        }
    } while ( !doneParsing );
}

vector< vector< Literal > >
DebugInterface::computeDisjointCores()
{
    vector< vector< Literal > > cores;
    vector< Literal > reducedAssumptions( consideredDebugLiterals );

    trace_msg( debug, 1, "Computing disjoint cores" );

    unsigned int solverResult = runSolver( reducedAssumptions, assertions );
    resetSolver();

    while ( solverResult == INCOHERENT )
    {
        vector< Literal > core = coreMinimizer.minimizeUnsatCore( *solver.getUnsatCore(), 3 );
        cores.push_back( core );

        trace_msg( debug, 2, "Found core: " << Formatter::formatClause( core ) );

        // disjoint cores must not contain any literals from the current core
        // -> remove the core literals from the assumptions
        for ( const Literal& coreLiteral : core )
        {
            reducedAssumptions.erase( std::remove( reducedAssumptions.begin(), reducedAssumptions.end(), Literal( coreLiteral.getVariable(), POSITIVE) ), reducedAssumptions.end() );
        }

        solverResult = runSolver( reducedAssumptions, assertions );
        resetSolver();
    }

    return cores;
}

vector< Literal >
DebugInterface::fixCore(
    const vector< vector< Literal > >& cores )
{
    // fix the first core
    for ( auto iterator = cores.begin() + 1; iterator != cores.end(); iterator ++ )
    {
        for ( const Literal& literal : *iterator )
        {
            consideredDebugLiterals.erase( remove( consideredDebugLiterals.begin(), consideredDebugLiterals.end(), Literal( literal.getVariable() ) ), consideredDebugLiterals.end() );
        }
    }

    return cores[ 0 ];
}

void
DebugInterface::resetSolver()
{
    solver.unrollToZero();
    solver.clearConflictStatus();
}

void
DebugInterface::determineAssertionDebugLiterals()
{
    trace_msg( debug, 1, "Determining _debug atoms that are assertions" );

    for ( const Literal& debugLiteral : debugLiterals )
    {
        // a rule r is assumed to be an assertion if it is a ground constraint
        // with |B(r)| = 1
        if ( VariableNames::getName( debugLiteral.getVariable() ).find( '(' ) == string::npos )
        {
            string rule = RuleNames::getRule( debugLiteral );
            rule.erase( remove_if( rule.begin(), rule.end(), ::isspace ), rule.end() );

            if ( rule.find( ":-" ) == 0 && rule.find( ',' ) == string::npos )
            {
                trace_msg( debug, 2, "Debug atom '" << debugLiteral << "' is assumed to be an assertion. Rule: '" << RuleNames::getRule( debugLiteral ) << "'" );
                assertionDebugLiterals.push_back( debugLiteral );
                assumedAssertions.push_back( RuleNames::getLiterals( debugLiteral )[ 0 ].getOppositeLiteral() );
            }
        }
    }
}

unsigned int
DebugInterface::runSolver(
    const vector< Literal >& debugAssumptions,
    const vector< Literal >& assertions)
{
    vector< Literal > assumptions( debugAssumptions );

    // add the assertions at the beginning
    assumptions.insert( assumptions.begin(), assertions.begin(), assertions.end() );

    solver.setComputeUnsatCores( true );
    //solver.setMinimizeUnsatCore( true );

    return solver.solve( assumptions );
}

vector< Var >
DebugInterface::determineQueryVariable(
    const vector< Literal >& unsatCore )
{
    if ( isUnfoundedCore( unsatCore ) )
        //return determineQueryVariableUnfounded( unsatCore );
        return vector< Var >();
    else
        return determineQueryVariableFounded( unsatCore );
}

Var
DebugInterface::determineQueryVariableUnfounded(
    const vector< Literal >& unsatCore )
{
    cout << "The core is an unfounded set" << endl;

    vector< Literal > unfoundedAssertions = getCoreAssertions( unsatCore );
    vector< Literal > visitedAssertions ( unfoundedAssertions );

    trace_msg( debug, 1, "Determining query variables - unfounded assertions = " << Formatter::formatClause( unfoundedAssertions ) );

    while ( !unfoundedAssertions.empty() )
    {
        Literal unfoundedAssertion = unfoundedAssertions.front();
        unfoundedAssertions.erase( unfoundedAssertions.begin() );

        trace_msg( debug, 2, "Unfounded assertion '" << Formatter::formatLiteral( unfoundedAssertion ) << "'" );
        trace_msg( debug, 3, "Computing supporting rules" );

        vector< pair< string, vector< Literal > > > supportingRules = RuleNames::getSupportingRules( unfoundedAssertion );

        trace_msg( debug, 3, "Found " << supportingRules.size() << " supporting rule(s)" );

        // iterate over each supporting rule and add the unfulfilled body literals to the queue
        for ( pair< string, vector< Literal > > pair : supportingRules )
        {
            string supportingRule = pair.first;
            vector< Literal > supportingRuleLiterals = pair.second;
            vector< Literal > unsatisfiedLiterals;

            trace_msg( debug, 4, "Rule '" << supportingRule << "' with literals " << Formatter::formatClause( supportingRuleLiterals ) );

            for ( const Literal& literal : supportingRuleLiterals )
            {
                trace_msg( debug, 5, "Literal '" << Formatter::formatLiteral( literal ) << "': isAssertion = " << isAssertion( literal.getVariable() ) << "; isAssumedAssertion = " << isVariableContainedInLiterals( literal.getVariable(), assumedAssertions ) << "; already visited = " << !isVariableContainedInLiterals( literal.getVariable(), visitedAssertions ));
                if ( !isAssertion( literal.getVariable() )
                  && !isVariableContainedInLiterals( literal.getVariable(), assumedAssertions )
                  && !isVariableContainedInLiterals( literal.getVariable(), visitedAssertions ))
                {
                    unsatisfiedLiterals.push_back( literal );
                }
            }

            trace_msg( debug, 5, "Unsatisfied literals: " << Formatter::formatClause( unsatisfiedLiterals ) );

            cout << "Possibly supporting rule for atom '" << Formatter::formatLiteral( unfoundedAssertion ) << "':" << endl << "  " << supportingRule << endl;

            if ( !unsatisfiedLiterals.empty() )
            {
                for ( const Literal& unsatL : unsatisfiedLiterals )
                {
                    TruthValue val = userInterface->askTruthValue( unsatL.getVariable() );
                    TruthValue satisfyingVal = unsatL.isPositive() ? TRUE : FALSE;
                    if ( val == satisfyingVal && !isVariableContainedInLiterals( unsatL.getVariable(), visitedAssertions ) )
                    {
                        trace_msg( debug, 5, "Adding '" << Formatter::formatLiteral( unsatL ) << "' to the queue." );
                        unfoundedAssertions.push_back( unsatL );
                        visitedAssertions.push_back( unsatL );
                    }
                }

            }

        }
    }

    return 0;

//    Var queryVariable = 0;
//    map< Var, unsigned int > variableOccurences;
//
//    trace_msg( debug, 1, "Determining query variables - unfounded case" );
//
//    for ( const Literal& coreLiteral : unsatCore )
//    {
//        Var unfoundedVariable = coreLiteral.getVariable();
//
//        if ( isVariableContainedInLiterals( unfoundedVariable, assertionDebugLiterals ) )
//        {
//            unfoundedVariable = RuleNames::getVariables( coreLiteral )[ 0 ];
//        }
//
//        trace_msg( debug, 2, "Get rule variables for variable '" << VariableNames::getName( unfoundedVariable ) << "'" );
//        for ( const Var bodyVariable : RuleNames::getVariablesOfSupportingRules( unfoundedVariable ) )
//        {
//            trace_msg( debug, 3, "Variable: " << VariableNames::getName( bodyVariable ) );
//            variableOccurences[ bodyVariable ] ++;
//        }
//    }
//
//    // return most occuring variable
//    unsigned int maxOccurence = 0;
//
//    trace_msg( debug, 2, "Iterating over all query variable candidates" );
//    for ( const auto& pair : variableOccurences )
//    {
//        Var currentVariable = pair.first;
//        unsigned int numOccurances = pair.second;
//
//        trace_msg( debug, 3, "Variable '" << VariableNames::getName( currentVariable ) << "' occurs " << numOccurances << " times");
//
//        if ( numOccurances > maxOccurence
//             && !isAssertion( currentVariable )
//             && !isFact( currentVariable ))
//        {
//            maxOccurence = numOccurances;
//            queryVariable = currentVariable;
//        }
//    }
//
//    return queryVariable;
}

class VariableComparator {
private:
    map< Var, int > variableEntropy;
    map< Var, unsigned int > variableOccurences;

public:
    VariableComparator(
        map< Var, int > variableEntropy,
        map< Var, unsigned int > variableOccurences )
    : variableEntropy( variableEntropy ), variableOccurences( variableOccurences ) {}

    bool operator () (
        Var v1,
        Var v2 )
    {
        if ( variableEntropy[ v1 ] == variableEntropy[ v2 ])
            return variableOccurences[ v1 ] > variableOccurences[ v2 ];
        else
            return variableEntropy[ v1 ] < variableEntropy[ v2 ];
    }
};

vector< Var >
DebugInterface::determineQueryVariableFounded(
    const vector< Literal >& unsatCore )
{
    map< Var, int > variableEntropy;
    map< Var, unsigned int > variableOccurences;

    trace_msg( debug, 1, "Determining query variable" );
    trace_msg( debug, 2, "Relaxing core variables and computing models" );

    unsigned int numModels = determineQueryVariableFounded( unsatCore, variableEntropy, consideredDebugLiterals, 3, time( NULL ) );

    trace_msg( debug, 2, "Found " << numModels << " models" );

    resetSolver();

    for ( const Literal& coreLiteral : unsatCore )
    {
        if ( isDebugVariable( coreLiteral.getVariable() ) )
        {
            for ( const Var coreVariable : RuleNames::getVariables( coreLiteral ) )
                variableOccurences[ coreVariable ] ++;
        }
    }

#ifdef TRACE_ON
    trace_msg( debug, 2, "Computed variable entropies" );
    for ( const auto& pair : variableOccurences )
    {
        trace_msg( debug, 3, "Variable " << VariableNames::getName( pair.first ) << ": " << pair.second << "x in the core, entropy = " << variableEntropy[ pair.first ] );
    }
#endif

    vector< Var > queryVariables;

    for ( auto const& pair : variableOccurences )
    {
        Var variable = pair.first;

        if ( !isAssertion( variable ) && !isFact( variable ) )
        {
            queryVariables.push_back( variable );
        }
    }

    sort( queryVariables.begin(), queryVariables.end(), VariableComparator( variableEntropy, variableOccurences ) );

    return queryVariables;
}


unsigned int
DebugInterface::determineQueryVariableFounded(
    const vector< Literal >& unsatCore,
    map< Var, int >& variableEntropy,
    const vector< Literal >& parentAssumptions,
    unsigned int level,
    const time_t& startTime)
{
    unsigned int numModels = 0;

    for ( const Literal& relaxLiteral : unsatCore )
    {
        if ( difftime( time( NULL ), startTime ) > QUERY_DETERMINATION_TIMEOUT )
        {
            trace_msg( debug, 2, "Query variable determination aborted due to timeout" );
            return numModels;
        }

        trace_msg( debug, level, "Relaxing " << Formatter::formatLiteral( relaxLiteral ) );
        vector< Literal > relaxedAssumptions;
        for ( unsigned int i = 0; i < parentAssumptions.size(); i ++ )
        {
            if ( parentAssumptions[ i ].getVariable() != relaxLiteral.getVariable() )
            {
                relaxedAssumptions.push_back( parentAssumptions[ i ] );
            }
        }

        if ( relaxedAssumptions.size() == parentAssumptions.size() )
        {
            trace_msg( debug, level, "Could not relax " << relaxLiteral << " because it was not inside the parent assumptions" );
        }
        else if ( runSolver( relaxedAssumptions, assertions ) == COHERENT )
        {
            numModels ++;
            trace_msg( debug, level, "Model found after relaxing " << Formatter::formatLiteral( relaxLiteral ) );

            for( Var variable = 1; variable <= solver.numberOfVariables(); variable++ )
            {
                variableEntropy[ variable ] = variableEntropy[ variable ] + (solver.isTrue( variable ) ? 1 : -1);
            }
            resetSolver();
        }
        else
        {
            resetSolver();
            vector< Literal > relaxedUnsatCore = coreMinimizer.minimizeUnsatCore( *solver.getUnsatCore(), level + 1 );
            numModels += determineQueryVariableFounded( relaxedUnsatCore, variableEntropy, relaxedAssumptions, level + 1, startTime );
        }
    }

    return numModels;
}

bool
DebugInterface::isUnfoundedCore(
    const vector< Literal > unsatCore )
{
    // core is unfounded, if each core variable is an assertion
    for ( const Literal& coreLiteral : unsatCore )
    {
        if ( !isVariableContainedInLiterals( coreLiteral.getVariable(), assertions )
          && !isVariableContainedInLiterals( coreLiteral.getVariable(), assertionDebugLiterals ) )
            return false;
    }

    return true;
}

bool
DebugInterface::loadHistory(
    const string& filename )
{
	string ruleHistory = "", answer;
	Var query;
	vector< Var > queryHistoryLoaded;
	vector< TruthValue > answerHistoryLoaded;

	ifstream historyFile ( filename );

	if ( !historyFile.is_open() )
		return false;

	while ( getline ( historyFile, ruleHistory ) )
	{
		int pos = ruleHistory.find(" ");

		if ( !VariableNames::getVariable( ruleHistory.substr( 0, pos ), query ) )
			return false;

		answer = ruleHistory.substr( pos+1, ruleHistory.length() );

		if ( answer != "true" && answer != "false" )
			return false;

		queryHistoryLoaded.push_back( query );
		answerHistoryLoaded.push_back( ( answer == "true" ) ? TRUE : FALSE );
	}
	historyFile.close();

	for ( unsigned int i = 0; i < queryHistoryLoaded.size(); i++ )
	{
	    assertions.push_back( Literal( queryHistoryLoaded[ i ], answerHistoryLoaded[ i ] == TRUE ? POSITIVE : NEGATIVE ) );
	}

    return true;
}

bool
DebugInterface::saveHistory(
    const string& filename )
{
	string ruleHistory = "";

	for ( const Literal& assertion : assertions )
	{
		ruleHistory += VariableNames::getName( assertion.getVariable() ) + " " +
					   ( assertion.isPositive() ? "true" : "false" ) + "\n";
	}

	ofstream historyFile ( filename );

	if ( !historyFile.is_open() )
		return false;

	historyFile << ruleHistory;

	historyFile.close();

    return true;
}
