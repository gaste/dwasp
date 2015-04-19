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

#include "RuleNames.h"
#include "Assert.h"

map< string, string > RuleNames::ruleMap;
map< string, vector< string > > RuleNames::variablesMap;

const string&
RuleNames::getRule( string debugAtom )
{
    size_t parenthesisIndex = debugAtom.find( '(' );

    if (parenthesisIndex == string::npos )
    {
        return ruleMap[ debugAtom ];
    }
    else
    {
        return ruleMap[ debugAtom.substr( 0, parenthesisIndex ) ];
    }
}

string
RuleNames::getSubstitution( string debugAtom )
{
    string substitution = "";
    size_t parenthesisIndex = debugAtom.find( '(' );

    if ( parenthesisIndex != string::npos )
    {
        string debugConstant = debugAtom.substr( 0, parenthesisIndex );
        string term = debugAtom.substr( parenthesisIndex + 1, debugAtom.length() - parenthesisIndex - 2);
        vector< string > terms = getTerms( term );
        vector< string > variables = variablesMap[ debugConstant ];

        assert_msg( terms.size() == variables.size(), "#terms to be substituted is different from #variables" );

        substitution = "{ ";
        substitution += variables[0] + "/" + terms[ 0 ];
        for ( size_t i = 1; i < terms.size(); i++ )
        {
            substitution += ", " + variables[i] + "/" + terms[ i ];
        }
        substitution += " }";
    }

    return substitution;
}

void
RuleNames::addRule( string debugAtom, string rule, vector< string > variables )
{
    ruleMap[ debugAtom ] = rule;
    variablesMap[ debugAtom ] = variables;
}

vector< string >
RuleNames::getTerms( string term )
{
    int openBrackets = 0;
    vector< string > terms;
    string currentTerm = ""; // required for nested terms, i.e. _debug1(a(b,c),d)

    for ( size_t i = 0; i < term.length(); i++ )
    {
        if ( term[ i ] == '(' )
        {
            openBrackets++;
        }
        else if ( term[ i ] == ')' )
        {
            openBrackets--;
        }
        else if ( term[ i ] == ',' && openBrackets == 0 )
        {
            terms.push_back( currentTerm );
            currentTerm = "";
        }
        else
        {
            currentTerm += term[ i ];
        }
    }

    terms.push_back( currentTerm );

    return terms;
}
