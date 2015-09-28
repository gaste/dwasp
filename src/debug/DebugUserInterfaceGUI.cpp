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

#define PART_DELIMITER        ':'
#define MESSAGE_DELIMITER     '\n'

#define REQUEST_GET_CORE      "get:core"
#define REQUEST_GET_QUERY     "get:query"
#define REQUEST_ASSERT        "assert"
#define REQUEST_ASSERT_LENGTH 6

#define RESPONSE_CORE         "response:core"
#define RESPONSE_QUERY        "response:query"

#define INFO_COHERENT         "info:coherent"
#define INFO_COMPUTING_QUERY  "info:compute:query"
#define INFO_COMPUTING_CORE   "info:compute:core"

#define ASSERT_TRUE           'y'
#define ASSERT_FALSE          'n'
#define ASSERT_UNDEFINED      'u'

UserCommand
DebugUserInterfaceGUI::promptCommand()
{
    string message;

    if ( cin.eof() ) return UserCommand::EXIT;

    getline( cin, message, MESSAGE_DELIMITER );

    lastMessage = message;

    if ( REQUEST_GET_CORE == message ) return UserCommand::SHOW_CORE;
    if ( REQUEST_GET_QUERY == message ) return UserCommand::ASK_QUERY;
    if ( REQUEST_ASSERT == message.substr( 0, REQUEST_ASSERT_LENGTH ) ) return UserCommand::ASSERT_VARIABLE;

    return UserCommand::EXIT;
}

void
DebugUserInterfaceGUI::printCore(
    const vector< Literal >& core,
    const vector< Literal >& coreAssertions )
{
    cout << RESPONSE_CORE;

    if ( core.empty() )
    {
        cout << PART_DELIMITER;
    }

    for ( const Literal& coreElement : core )
    {
        cout << PART_DELIMITER << VariableNames::getName(coreElement.getVariable());
    }

    cout << MESSAGE_DELIMITER;
}

void
DebugUserInterfaceGUI::queryResponse(
    const vector< Var >& variables )
{
    cout << RESPONSE_QUERY;

    if ( variables.empty() )
    {
        cout << PART_DELIMITER;
    }

    for ( const Var v : variables )
    {
        cout << PART_DELIMITER << VariableNames::getName( v );
    }

    cout << MESSAGE_DELIMITER;
}

std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems) {
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        elems.push_back(item);
    }
    return elems;
}

std::vector<std::string> split(const std::string &s, char delim) {
    std::vector<std::string> elems;
    split(s, delim, elems);
    return elems;
}

vector< Literal >
DebugUserInterfaceGUI::getAssertions()
{
    vector< Literal > userAssertions;

    if ( REQUEST_ASSERT == lastMessage.substr( 0, REQUEST_ASSERT_LENGTH ) )
    {
        vector< string > elems = split( lastMessage, PART_DELIMITER );

        // remove the first message part which is the identifier
        elems.erase( elems.begin() );

        for ( const string& e : elems )
        {
            if ( e[ e.length() - 1 ] != ASSERT_UNDEFINED )
            {
                Var var;

                if ( VariableNames::getVariable( e.substr( 0, e.length() - 2 ), var ) )
                {
                    unsigned int sign;

                    if ( e[ e.length() - 1 ] == ASSERT_TRUE ) sign = POSITIVE;
                    if ( e[ e.length() - 1 ] == ASSERT_FALSE ) sign = NEGATIVE;

                    userAssertions.push_back( Literal( var, sign ) );
                }
            }
        }
    }

    return userAssertions;
}

void
DebugUserInterfaceGUI::informProgramCoherent(
    const vector< Var >& answerSet )
{
    cout << INFO_COHERENT;

    if ( answerSet.empty() )
        cout << PART_DELIMITER;

    for ( const Var& v : answerSet )
        cout << PART_DELIMITER << VariableNames::getName( v );

    cout << MESSAGE_DELIMITER;
    cout.flush();
}

void
DebugUserInterfaceGUI::informComputingCore()
{
    cout << INFO_COMPUTING_CORE << MESSAGE_DELIMITER;
    cout.flush();
}

void
DebugUserInterfaceGUI::informComputingQuery()
{
    cout << INFO_COMPUTING_QUERY << MESSAGE_DELIMITER;
    cout.flush();
}
