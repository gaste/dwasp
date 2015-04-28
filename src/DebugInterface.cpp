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

#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>
#include <utility>

#include "Solver.h"
#include "util/ErrorMessage.h"
#include "util/Formatter.h"
#include "util/RuleNames.h"
#include "util/Trace.h"
#include "util/VariableNames.h"

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
        if ( !isVariableContainedInLiterals( coreLiteral.getVariable(), assertions ) )
        {
            coreWithoutAssertions.push_back( coreLiteral );
        }
    }

    return coreWithoutAssertions;
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
        ErrorMessage::errorGeneric( "Program not INCOHERENT" );
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

    assert( solver.getUnsatCore() != NULL );

    vector< Literal > minimalUnsatCore = coreMinimizer.minimizeUnsatCore( *solver.getUnsatCore() );
    
    do
    {
        switch (userInterface->promptCommand())
        {
        case SHOW_CORE:
            userInterface->printCore( getCoreWithoutAssertions( minimalUnsatCore ) );
            break;
        case SHOW_CORE_GROUND_RULES:
            userInterface->printCoreGroundRules( getCoreWithoutAssertions( minimalUnsatCore ) );
            break;
        case SHOW_CORE_NONGROUND_RULES:
            userInterface->printCoreUngroundRules( getCoreWithoutAssertions( minimalUnsatCore ) );
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
                    cout << "Found answer set";
                    solver.printAnswerSet();
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
            Literal assertion = userInterface->getAssertion();

            if ( isAssertion( assertion.getVariable() ) )
            {
                userInterface->informAssertionAlreadyPresent( VariableNames::getName( assertion.getVariable() ) );
            } else if ( isFact( assertion.getVariable() ) )
            {
                userInterface->informAssertionIsFact( VariableNames::getName( assertion.getVariable() ) );
            }
            else
            {
                assertions.push_back( assertion );

                userInterface->informSolving();

                if ( runSolver( consideredDebugLiterals, assertions ) == INCOHERENT )
                {
                    resetSolver();
                    assert( solver.getUnsatCore() != NULL );
                    minimalUnsatCore = coreMinimizer.minimizeUnsatCore( *solver.getUnsatCore() );
                }
                else
                {
                    cout << "Found answer set";
                    solver.printAnswerSet();
                    continueDebugging = false;
                }
            }
            break;
        }
        case ASK_QUERY:
        {
            userInterface->informComputingQueryVariable();
            Var queryVariable = determineQueryVariable( minimalUnsatCore );
            TruthValue queryVariableTruthValue = UNDEFINED;

            if ( queryVariable == 0 )
            {
                userInterface->informNoQueryPossible();
            }
            else
            {
                queryVariableTruthValue = userInterface->askTruthValue( queryVariable );
            }


            if ( queryVariableTruthValue != UNDEFINED )
            {
                resetSolver();
                Literal assertion( queryVariable, queryVariableTruthValue == TRUE ? POSITIVE : NEGATIVE );
                assertions.push_back( assertion );

                userInterface->informSolving();

                if ( runSolver( consideredDebugLiterals, assertions ) == INCOHERENT )
                {
                    resetSolver();
                    assert( solver.getUnsatCore() != NULL );
                    minimalUnsatCore = coreMinimizer.minimizeUnsatCore( *solver.getUnsatCore() );
                }
                else
                {
                    cout << "Found answer set";
                    solver.printAnswerSet();
                    continueDebugging = false;
                }
            }
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
                    cout << "Found answer set";
                    solver.printAnswerSet();
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

unsigned int
DebugInterface::runSolver(
    const vector< Literal >& debugAssumptions,
    const vector< Literal >& assertions)
{
    vector< Literal > assumptionsAND( debugAssumptions );
    vector< Literal > assumptionsOR;

    // add the assertions at the beginning
    assumptionsAND.insert( assumptionsAND.begin(), assertions.begin(), assertions.end() );

    solver.setComputeUnsatCores( true );
    //solver.setMinimizeUnsatCore( true );
    solver.setComputeMinimalUnsatCore( false );

    return solver.solve( assumptionsAND, assumptionsOR );
}

Var
DebugInterface::determineQueryVariable(
    const vector< Literal >& unsatCore )
{
    Var queryVariable = 0;
    map< Var, int > variableEntropy;
    map< Var, unsigned int > variableOccurences;

    trace_msg( debug, 1, "Determining query variable" );
    trace_msg( debug, 2, "Relaxing core variables and computing models" );

    unsigned int numModels = determineQueryVariable( unsatCore, variableEntropy, consideredDebugLiterals, 3, time( NULL ) );
    unsigned int lowestEntropy = numModels + 1;

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

    for ( auto const& pair : variableOccurences )
    {
        Var variable = pair.first;
        unsigned int entropy = abs( variableEntropy[ variable ] );

        if ( (entropy < lowestEntropy
                || (entropy == lowestEntropy
                    && variableOccurences[ variable ]
                     > variableOccurences[ queryVariable ]))
             && !isAssertion( variable )
             && !isFact( variable ) )
        {
            queryVariable = variable;
            lowestEntropy = entropy;
        }
    }

    return queryVariable;
}

unsigned int
DebugInterface::determineQueryVariable(
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
            numModels += determineQueryVariable( relaxedUnsatCore, variableEntropy, relaxedAssumptions, level + 1, startTime );
        }
    }

    return numModels;
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
