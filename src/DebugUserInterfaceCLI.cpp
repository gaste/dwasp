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

#include "util/RuleNames.h"
#include "util/VariableNames.h"

UserCommand
DebugUserInterfaceCLI::promptCommand()
{
	string userInput = "";

	do
	{
		promptInput(userInput);

		if( userInput == "print" )
		{
			return SHOW_CORE;
		}
		else if( userInput == "history" )
		{
		    return SHOW_HISTORY;
		}
		else if( userInput == "ask" )
		{
			return ASK_QUERY;
		}
		else if( userInput == "exit" )
        {
            return EXIT;
        }
		else
		{
			cout << "Invalid command. Available commands:" << endl
				 << "    print: show the UNSAT core" << endl
				 << "    history: show the debug history" << endl
				 << "    ask: ask me a query" << endl
				 << "    exit: stop the debugging session";
		}
	} while(true);
}

void
DebugUserInterfaceCLI::printCore(
    const vector< Literal >& core )
{
    if ( !core.empty() )
    {
        for ( unsigned int i = 0; i < core.size(); i++ )
        {
            cout << RuleNames::getRule( VariableNames::getName( core[i].getVariable() ) ) << " "
                 << RuleNames::getSubstitution( VariableNames::getName( core[i].getVariable() ) ) << endl ;
        }
    }
}

void
DebugUserInterfaceCLI::printCoreGroundRules(
    const vector< Literal >& core )
{

}

void
DebugUserInterfaceCLI::printCoreUngroundRules(
    const vector< Literal >& core )
{

}

void
DebugUserInterfaceCLI::printHistory(
    const vector< Var >& queryHistory,
    const vector< TruthValue >& answerHistory )
{
    for ( unsigned int i = 0; i < queryHistory.size(); i ++ )
    {
        cout << i << ": " << VariableNames::getName( queryHistory[ i ] )
                << " = " << ((answerHistory[ i ] == TRUE) ? "true" : "false")
                << endl;
    }
}

TruthValue
DebugUserInterfaceCLI::askTruthValue(
    const Var variable )
{
	string userInput;

	do
	{
		cout << "Should '" << VariableNames::getName( variable )
			 << "' be in the model? (y/n)";

		promptInput(userInput);

		if ( userInput == "y" ) return TRUE;
		else if ( userInput == "n" ) return FALSE;
	} while(true);
}

string
DebugUserInterfaceCLI::askHistoryFilename()
{
    return "";
}

Literal
DebugUserInterfaceCLI::getAssertion()
{
    return Literal( 1 );
}


void
DebugUserInterfaceCLI::greetUser()
{

}

void
DebugUserInterfaceCLI::informSolving()
{

}

void
DebugUserInterfaceCLI::informComputingQueryVariable()
{

}

void
DebugUserInterfaceCLI::informSavedHistory(
    const string& filename )
{

}

void
DebugUserInterfaceCLI::informLoadedHistory(
    const string& filename )
{

}

void
DebugUserInterfaceCLI::informCouldNotSaveHistory(
    const string& filename )
{

}

void
DebugUserInterfaceCLI::informCouldNotLoadHistory(
    const string& filename )
{

}
