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

vector< Literal >
DebugInterface::clauseToVector(
    const Clause& clause )
{
	vector< Literal > literals;

	for (unsigned int i = 0; i < clause.size(); i++)
		literals.push_back(clause[i]);

	return literals;
}

bool
DebugInterface::isDebugVariable(
    const Var variable )
{
    for ( Literal debugLiteral : debugLiterals )
        if ( debugLiteral.getVariable() == variable )
            return true;

    return false;
}

bool
DebugInterface::isFact(
    const Var variable )
{
    return find( facts.begin(), facts.end(), variable ) != facts.end();
}

void
DebugInterface::debug()
{
    bool continueDebugging = true;
    
    trace_msg( debug, 1, "Start debugging with _debug assumptions" );

    userInterface->greetUser();
    userInterface->informSolving();

    if ( computeUnsatCore( consideredDebugLiterals, assertions ) != INCOHERENT )
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

    vector< Literal > minimalUnsatCore = coreMinimizer.minimizeUnsatCore( clauseToVector( *solver.getUnsatCore() ) );
    
    do
    {
        switch (userInterface->promptCommand())
        {
        case SHOW_CORE:
        {
            vector< Literal > rulesOnlyCore;
            for ( const Literal& assertion : minimalUnsatCore )
            {
                if ( find( assertions.begin(), assertions.end(), Literal( assertion.getVariable(), POSITIVE ) ) == assertions.end()
                        && find( assertions.begin(), assertions.end(), Literal( assertion.getVariable(), NEGATIVE ) ) == assertions.end() )
                {
                    rulesOnlyCore.push_back( assertion );
                }
            }
            userInterface->printCore( rulesOnlyCore );
            break;
        }
        case SHOW_CORE_GROUND_RULES:
        {
            vector< Literal > rulesOnlyCore;
            for ( const Literal& assertion : minimalUnsatCore )
            {
                if ( find( assertions.begin(), assertions.end(), Literal( assertion.getVariable(), POSITIVE ) ) == assertions.end()
                        && find( assertions.begin(), assertions.end(), Literal( assertion.getVariable(), NEGATIVE ) ) == assertions.end() )
                {
                    rulesOnlyCore.push_back( assertion );
                }
            }
            userInterface->printCoreGroundRules( rulesOnlyCore );
            break;
        }
        case SHOW_CORE_NONGROUND_RULES:
        {
            vector< Literal > rulesOnlyCore;
            for ( const Literal& assertion : minimalUnsatCore )
            {
                if ( find( assertions.begin(), assertions.end(), Literal( assertion.getVariable(), POSITIVE ) ) == assertions.end()
                        && find( assertions.begin(), assertions.end(), Literal( assertion.getVariable(), NEGATIVE ) ) == assertions.end() )
                {
                    rulesOnlyCore.push_back( assertion );
                }
            }
            userInterface->printCoreUngroundRules( rulesOnlyCore );
            break;
        }
        case SHOW_HISTORY:
            userInterface->printHistory( queryHistory, answerHistory );
            break;
        case ANALYZE_DISJOINT_CORES:
        {
            userInterface->informSolving();
            vector< vector< Literal > > cores = computeDisjointCores();
            fixCore( cores );
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
            unsigned int historyPoint = queryHistory.size();
            string filename = userInterface->askHistoryFilename();

            if ( loadHistory( filename ) )
            {
                // replay the history
                for ( unsigned int i = historyPoint; i < queryHistory.size(); i ++ )
                {
                    assertions.push_back( Literal( queryHistory[ i ], answerHistory[ i ] == TRUE ? POSITIVE : NEGATIVE ) );
                }

                userInterface->informLoadedHistory( filename );
                userInterface->informSolving();
                if ( computeUnsatCore( consideredDebugLiterals, assertions ) == INCOHERENT )
                {
                    resetSolver();
                    assert( solver.getUnsatCore() != NULL );
                    minimalUnsatCore = coreMinimizer.minimizeUnsatCore( clauseToVector( *solver.getUnsatCore() ) );
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
            assertions.push_back( assertion );
            queryHistory.push_back( assertion.getVariable() );
            answerHistory.push_back( assertion.getSign() == POSITIVE ? POSITIVE : NEGATIVE );

            userInterface->informSolving();

            if ( computeUnsatCore( consideredDebugLiterals, assertions ) == INCOHERENT )
            {
                resetSolver();
                assert( solver.getUnsatCore() != NULL );
                minimalUnsatCore = coreMinimizer.minimizeUnsatCore( clauseToVector( *solver.getUnsatCore() ) );
            }
            else
            {
                cout << "Found answer set";
                solver.printAnswerSet();
                continueDebugging = false;
            }
            break;
        }
        case UNDO_ASSERTION:
        {
            unsigned int undo = userInterface->chooseAssertionToUndo( queryHistory, answerHistory );

            if ( queryHistory.size() > 0 )
            {
                queryHistory.erase( queryHistory.begin() + undo );
                answerHistory.erase( answerHistory.begin() + undo );
                assertions.erase( assertions.begin() + undo );
            }
            break;
        }
        case ASK_QUERY:
        {
            userInterface->informComputingQueryVariable();
            Var queryVariable = determineQueryVariable( minimalUnsatCore );
            TruthValue queryVariableTruthValue = userInterface->askTruthValue( queryVariable );

            if ( queryVariableTruthValue != UNDEFINED )
            {
                resetSolver();
                Literal assertion( queryVariable, queryVariableTruthValue == TRUE ? POSITIVE : NEGATIVE );

                queryHistory.push_back( queryVariable );
                answerHistory.push_back( queryVariableTruthValue );
                assertions.push_back( assertion );

                userInterface->informSolving();

                if ( computeUnsatCore( consideredDebugLiterals, assertions ) == INCOHERENT )
                {
                    resetSolver();
                    assert( solver.getUnsatCore() != NULL );
                    minimalUnsatCore = coreMinimizer.minimizeUnsatCore( clauseToVector( *solver.getUnsatCore() ) );
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

            // read the ungrounded rule
            do
            {
                stream.read( word );
                rule += word;

                if ( word[ word.length() - 1 ] != '.' )
                    rule += " ";
            } while ( word[ word.length() - 1 ] != '.' );

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
    vector< Literal > reducedAssumptions( consideredDebugLiterals );
    vector< vector< Literal > > cores;

    trace_msg( debug, 1, "Computing disjoint cores" );

    unsigned int solverResult = computeUnsatCore( reducedAssumptions, assertions );
    resetSolver();

    while ( solverResult == INCOHERENT )
    {
        vector< Literal > core = coreMinimizer.minimizeUnsatCore( clauseToVector( *solver.getUnsatCore() ), 3 );
        cores.push_back( core );

        trace_msg( debug, 2, "Found core: " << Formatter::formatClause( core ) );

        for ( const Literal& coreLiteral : core )
        {
            reducedAssumptions.erase( std::remove( reducedAssumptions.begin(), reducedAssumptions.end(), Literal( coreLiteral.getVariable(), POSITIVE) ), reducedAssumptions.end() );
        }

        solverResult = computeUnsatCore( reducedAssumptions, assertions );
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
DebugInterface::computeUnsatCore(
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
    Var queryVariable = unsatCore[ 0 ].getVariable();
    map< Var, int > variableEntropy;

    unsigned int numModels = determineQueryVariable( unsatCore, variableEntropy, consideredDebugLiterals, 1 );
    unsigned int lowestEntropy = numModels + 1;

    resetSolver();

    for ( auto const& pair : variableEntropy )
    {
        Var variable = pair.first;
        unsigned int entropy = abs( pair.second );

        if ( entropy < lowestEntropy
             && find( queryHistory.begin(), queryHistory.end(), variable ) == queryHistory.end()
             && !isDebugVariable( variable )
             && !isFact( variable )
             && solver.isUndefined( variable ) )
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
    unsigned int level )
{
    unsigned int numModels = 0;

    for ( Literal relaxLiteral : unsatCore )
    {
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
        else if ( computeUnsatCore( relaxedAssumptions, assertions ) == COHERENT )
        {
            numModels ++;
            trace_msg( debug, level, "Model found after relaxing " << Formatter::formatLiteral( relaxLiteral ) );

            for( Var variable = 1; variable <= solver.numberOfVariables(); variable++ )
            {
                if ( variableEntropy.count( variable ) == 0 )
                {
                    variableEntropy[ variable ] = solver.isTrue( variable ) ? 1 : -1;
                }
                else
                {
                    variableEntropy[ variable ] += solver.isTrue( variable ) ? 1 : -1;
                }
            }
            resetSolver();
        }
        else
        {
            resetSolver();
            vector< Literal > relaxedUnsatCore = coreMinimizer.minimizeUnsatCore( clauseToVector( *solver.getUnsatCore() ), level + 1 );
            numModels += determineQueryVariable( relaxedUnsatCore, variableEntropy, relaxedAssumptions, level + 1 );
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
		queryHistory.push_back( queryHistoryLoaded[i] );
		answerHistory.push_back( answerHistoryLoaded[i] );
	}

    return true;
}

bool
DebugInterface::saveHistory(
    const string& filename )
{
	string ruleHistory = "";

	for ( unsigned int i = 0; i < queryHistory.size(); i ++ )
	{
		ruleHistory += VariableNames::getName( queryHistory[ i ] ) + " " +
					   ( ( answerHistory[ i ] == TRUE ) ? "true" : "false" ) + "\n";
	}

	ofstream historyFile ( filename );

	if ( !historyFile.is_open() )
		return false;

	historyFile << ruleHistory;

	historyFile.close();

    return true;
}
