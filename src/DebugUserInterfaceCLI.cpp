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

#include "DebugUserInterfaceCLI.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <iterator>
#include <utility>

#include "util/Formatter.h"
#include "util/OutputPager.h"
#include "util/RuleNames.h"
#include "util/VariableNames.h"

map< string, cmd > DebugUserInterfaceCLI::commandMap =
{
    { "show core", { SHOW_CORE, "Show the literals inside the UNSAT core." } },
    { "show core ground", { SHOW_CORE_GROUND_RULES, "Show the ground rules inside the UNSAT core." } },
    { "show core unground", { SHOW_CORE_NONGROUND_RULES, "Show the unground rules inside the UNSAT core." } },
    { "show history", { SHOW_HISTORY, "Show the history of assertions." } },
    { "ask", { ASK_QUERY, "Ask me a question about the program." } },
    { "save history", { SAVE_HISTORY, "Save the assertion history in a file." } },
    { "load history", { LOAD_HISTORY, "Load the assertion history from a file." } },
    { "assert", { ASSERT_VARIABLE, "Assert the truth value of a variable." } },
    { "undo assert", { UNDO_ASSERTION, "Choose and undo an assertion." } },
    { "fix core", { ANALYZE_DISJOINT_CORES, "Compute all disjoint cores and fix one of them." } },
    { "exit", { EXIT, "Stop the debugging session." } }
};

UserCommand
DebugUserInterfaceCLI::promptCommand()
{
	string userInput = "";

	do
	{
		promptInput(userInput);

		if ( userInput == "help" )
		{
		    printHelp();
		}
		else if ( commandMap.count( userInput ) )
		{
		    return commandMap[ userInput ].command;
		}
		else
		{
		    cout << "Undefined command: \"" + userInput + "\".  Try \"help\"." << endl;
		}
	} while(true);
}

void
DebugUserInterfaceCLI::printHelp()
{
    string helpText = "Available commands:\n\n";

    for ( const auto& pair : commandMap )
    {
        helpText += pair.first + " -- " + pair.second.helpText + "\n";
    }

    OutputPager::paginate( helpText );
}

void
DebugUserInterfaceCLI::printCore(
    const vector< Literal >& core )
{
    OutputPager::paginate( Formatter::formatClause( core ) + "\n" );
}

void
DebugUserInterfaceCLI::printCoreGroundRules(
    const vector< Literal >& core )
{
    string groundCoreRules = "";
    if ( !core.empty() )
    {
        for ( const Literal& coreLiteral : core )
        {
            groundCoreRules += RuleNames::getGroundRule( coreLiteral ) + "\n";
        }
    }

    OutputPager::paginate( groundCoreRules );
}

void
DebugUserInterfaceCLI::printCoreUngroundRules(
    const vector< Literal >& core )
{
    string coreUngroundRules = "";
    map< string, vector< string > > ruleSubstitutionMap;

    for( const Literal& coreLiteral : core )
    {
        string rule = RuleNames::getRule( coreLiteral.getVariable() );
        string substitution = RuleNames::getSubstitution( coreLiteral.getVariable() );

        if ( substitution.length() > 0 )
            ruleSubstitutionMap[ rule ].push_back( substitution );
        else
            ruleSubstitutionMap[ rule ];
    }

    for ( const auto& mapEntry : ruleSubstitutionMap )
    {
        coreUngroundRules += mapEntry.first + "\n";

        for ( const string& substitution : mapEntry.second )
        {
            coreUngroundRules += "    " + substitution + "\n";
        }
    }

    OutputPager::paginate( coreUngroundRules );
}

void
DebugUserInterfaceCLI::printHistory(
    const vector< Var >& queryHistory,
    const vector< TruthValue >& answerHistory )
{
    string history = "";

    for ( unsigned int i = 0; i < queryHistory.size(); i ++ )
    {
        history += to_string( i ) + ": " + VariableNames::getName( queryHistory[ i ] )
                + " = " + ((answerHistory[ i ] == TRUE) ? "true" : "false")
                + "\n";
    }

    OutputPager::paginate( history );
}

TruthValue
DebugUserInterfaceCLI::askTruthValue(
    const Var variable )
{
	string userInput;

	do
	{
		cout << "Should '" << VariableNames::getName( variable )
			 << "' be in the model? (y/n/u): ";

		getline( cin, userInput );

		transform( userInput.begin(), userInput.end(), userInput.begin(), ::tolower );

		if ( userInput == "y" ) return TRUE;
		else if ( userInput == "n" ) return FALSE;
		else if ( userInput == "u" ) return UNDEFINED;
	} while(true);
}

string
DebugUserInterfaceCLI::askHistoryFilename()
{
    string filename;
    cout << "Filename: ";
    getline( cin, filename );
    return filename;
}

Literal
DebugUserInterfaceCLI::getAssertion()
{
    bool inputCorrect = false;
    string input;
    Var assertionVariable;

    do
    {
        cout << "Variable: ";
        getline( cin, input );
        inputCorrect = VariableNames::getVariable( input, assertionVariable );

        if ( !inputCorrect )
        {
            cout << "No variable named \"" << input << "\" exists" << endl;
        }
    } while ( !inputCorrect );

    inputCorrect = false;

    do
    {
        cout << "Truth value (t/f): ";
        getline( cin, input );
        transform( input.begin(), input.end(), input.begin(), ::tolower );

        inputCorrect = input == "t" || input == "f";
    } while ( !inputCorrect );

    return Literal( assertionVariable, input == "t" ? POSITIVE : NEGATIVE );
}

unsigned int
DebugUserInterfaceCLI::chooseAssertionToUndo(
    const vector< Var >& queryHistory,
    const vector< TruthValue >& answerHistory )
{
    if ( queryHistory.empty() )
    {
        cout << "No assertions available." << endl;
        return 0;
    }

    string userInput;
    unsigned int assertion = queryHistory.size();
    cout << "Choose an assertion to undo:" << endl;
    printHistory( queryHistory, answerHistory );

    do
    {
        cout << "Assertion (0-" << (queryHistory.size() - 1) << "): ";
        getline( cin, userInput );

        // check if the input is a number
        auto iterator = userInput.begin();
        while ( iterator != userInput.end() && isdigit( *iterator ) ) ++iterator;

        if ( !userInput.empty() && iterator == userInput.end() )
            assertion = atoi( userInput.c_str() );
    } while ( assertion >= queryHistory.size() );

    return assertion;
}

void
DebugUserInterfaceCLI::informAnalyzedDisjointCores(
    const unsigned int numCores )
{
    if ( numCores == 1 )
    {
        cout << "There is only one core." << endl;
    }
    else
    {
        cout << "Found " << numCores << " cores and fixed one of them." << endl;
    }
}


void
DebugUserInterfaceCLI::greetUser()
{
    cout << "WASP debbuging mode" << endl;
}

void
DebugUserInterfaceCLI::informSolving()
{
    cout << "Computing the unsatisfiable core" << endl;
}

void
DebugUserInterfaceCLI::informComputingQueryVariable()
{
    cout << "Computing the query" << endl;
}

void
DebugUserInterfaceCLI::informSavedHistory(
    const string& filename )
{
    cout << "Saved history to '" << filename << "'" << endl;
}

void
DebugUserInterfaceCLI::informLoadedHistory(
    const string& filename )
{
    cout << "Loaded history from '" << filename << "'" << endl;
}

void
DebugUserInterfaceCLI::informCouldNotSaveHistory(
    const string& filename )
{
    cout << "Unable to save the history to the file '" << filename << "'" << endl;
}

void
DebugUserInterfaceCLI::informCouldNotLoadHistory(
    const string& filename )
{
    cout << "Unable to load the history from the file '" << filename << "'" << endl;
}
