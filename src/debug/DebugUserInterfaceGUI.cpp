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

#include "DebugUserInterfaceGUI.h"

#include "../util/VariableNames.h"


#define PART_DELIMITER ':'
#define MESSAGE_DELIMITER '\n'

#define REQUEST_GET_CORE "get:core"
#define RESPONSE_CORE "response:core"
#define REQUEST_GET_TRUTH_VALUE "get:truthvalue"

UserCommand
DebugUserInterfaceGUI::promptCommand()
{
    string message;

    getline( cin, message, MESSAGE_DELIMITER );

    if ( REQUEST_GET_CORE == message ) return UserCommand::SHOW_CORE;
    else return UserCommand::ASK_QUERY;
}

void
DebugUserInterfaceGUI::printCore(
    const vector< Literal >& core,
    const vector< Literal >& coreAssertions )
{
    cout << RESPONSE_CORE;

    for ( const Literal& coreElement : core )
    {
        cout << PART_DELIMITER << VariableNames::getName(coreElement.getVariable());
    }

    cout << MESSAGE_DELIMITER;
}

TruthValue
DebugUserInterfaceGUI::askTruthValue(
    const Var variable )
{
    cout << REQUEST_GET_TRUTH_VALUE << VariableNames::getName(variable) << MESSAGE_DELIMITER;

    string response;
    getline( cin, response, MESSAGE_DELIMITER );

    return UNDEFINED;
}
