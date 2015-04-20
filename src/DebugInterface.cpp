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

vector< Literal > DebugInterface::clauseToVector( const Clause& clause )
{
	vector< Literal > debugLiterals;

	for (unsigned int i = 0; i < clause.size(); i++)
	{
		debugLiterals.push_back(clause[i]);
	}

	return debugLiterals;
}

void DebugInterface::debug()
{
    Var queryVariable;
    TruthValue queryVariableTruthValue;
    bool continueDebugging = true;
    
    trace_msg( debug, 1, "Start debugging with _debug assumptions" );

    if ( computeUnsatCore( assumptions ) != INCOHERENT )
    {
        ErrorMessage::errorGeneric( "Program not INCOHERENT" );
        return;
    }

    assert( solver.getUnsatCore() != NULL );    
    vector< Literal > minimalUnsatCore = coreMinimizer.minimizeUnsatCore( clauseToVector( *solver.getUnsatCore() ) );
    
    do
    {
        switch (userInterface->promptCommand())
        {
        case SHOW_CORE:
            userInterface->printCore( minimalUnsatCore );
            break;
        case SHOW_HISTORY:
            userInterface->printHistory( queryHistory, answerHistory );
            break;
        case ASK_QUERY:
        {
            queryVariable = determineQueryVariable( minimalUnsatCore );            
            queryVariableTruthValue = userInterface->askTruthValue( queryVariable );            
            Literal lit( queryVariable, queryVariableTruthValue == TRUE ? POSITIVE : NEGATIVE );
            queryHistory.push_back( queryVariable );
            answerHistory.push_back( queryVariableTruthValue );

            assert( solver.getCurrentDecisionLevel() == 0 );
            assert( !solver.isFalse( lit ) );
            solver.addClause( lit );

            if ( computeUnsatCore( assumptions ) == INCOHERENT )
            {
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
        case EXIT:
            continueDebugging = false;
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
DebugInterface::computeUnsatCore(
    const vector< Literal >& assumptions )
{
    vector< Literal > assumptionsAND( assumptions );
    vector< Literal > assumptionsOR;

    solver.setComputeUnsatCores( true );
    solver.setComputeMinimalUnsatCore( false );

    unsigned int result = solver.solve( assumptionsAND, assumptionsOR );

    solver.unrollToZero();
    solver.clearConflictStatus();

    return result;
}

Var
DebugInterface::determineQueryVariable( const vector< Literal >& unsatCore )
{
    Var queryVariable = unsatCore[ 0 ].getVariable();
    map< Var, unsigned int > variableInModel;
    float bestValue = 1;

    unsigned int numModels = determineQueryVariable( unsatCore, variableInModel, assumptions, 1 );

    for ( pair< Var, unsigned int > pair : variableInModel )
    {
        Var variable = pair.first;
        unsigned int numTrueInModels = pair.second;
        float val = (numTrueInModels == 0) ? 0.5 : (0.5 - numTrueInModels / (float)numModels);
        val = val < 0 ? -val : val;

        if ( val < bestValue && find( queryHistory.begin(), queryHistory.end(), variable ) == queryHistory.end() )
        {
            queryVariable = variable;
            bestValue = val;
        }
    }

    return queryVariable;
}

unsigned int
DebugInterface::determineQueryVariable(
    const vector< Literal >& unsatCore,
    map< Var, unsigned int >& countTrueInModels,
    const vector< Literal >& parentAssumptions,
    unsigned int level )
{
    unsigned int numModels = 0;

    for ( Literal relaxLiteral : unsatCore )
    {
        trace_msg( debug, level, "Relaxing " + Formatter::formatLiteral( relaxLiteral ) );
        vector< Literal > relaxedAssumptions;
        for ( size_t i = 0; i < parentAssumptions.size(); i ++ )
        {
            if ( parentAssumptions[ i ].getVariable() != relaxLiteral.getVariable() )
            {
                relaxedAssumptions.push_back( parentAssumptions[ i ] );
            }
        }

        if ( computeUnsatCore( relaxedAssumptions ) == COHERENT )
        {
            numModels ++;
            trace_msg( debug, level, "Model found after relaxing "  + Formatter::formatLiteral( relaxLiteral ) );
            size_t numAssignedVariables = solver.numberOfAssignedLiterals();

            for ( size_t i = 0; i < numAssignedVariables; i ++ )
            {
                Var variable = solver.getAssignedVariable( i );
                TruthValue value = solver.getTruthValue( variable );

                if ( countTrueInModels.count( variable ) == 0 )
                {
                    countTrueInModels[ variable ] = (value == TRUE ? 1 : 0);
                }
                else
                {
                    countTrueInModels[ variable ] = countTrueInModels[ variable ] + (value == TRUE ? 1 : 0);
                }
            }
        }
        else
        {
            vector< Literal > relaxedUnsatCore = coreMinimizer.minimizeUnsatCore( clauseToVector( *solver.getUnsatCore() ), level + 1 );
            numModels += determineQueryVariable( relaxedUnsatCore, countTrueInModels, relaxedAssumptions, level + 1 );
        }
    }

    return numModels;
}
