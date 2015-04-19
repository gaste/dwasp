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
#include "util/Formatter.h"
#include <iostream>
#include <string>

using namespace std;

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
				 << "    ask: ask me a query" << endl
				 << "    exit: stop the debugging session";
		}
	} while(true);
}

void
DebugUserInterfaceCLI::printCore( vector< Literal >& literals )
{
    if ( !literals.empty() )
    {
        for ( unsigned int i = 0; i < literals.size(); i++ )
        {
            cout << RuleNames::getRule( VariableNames::getName( literals[i].getVariable() ) ) << endl
                 << "    substitution = "
                 << RuleNames::getSubstitution( VariableNames::getName( literals[i].getVariable() ) ) << endl ;
        }
    }
}

TruthValue
DebugUserInterfaceCLI::askTruthValue( Var variable )
{
	string userInput;

	do
	{
		cout << "Should the variable '" << VariableNames::getName( variable )
			 << "' be true (t) or false (f) in the model?";

		promptInput(userInput);

		if ( userInput == "t" ) return TRUE;
		else if ( userInput == "f" ) return FALSE;
	} while(true);
}
