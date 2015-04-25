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

#include <algorithm>
#include <cctype>
#include <utility>
#include <regex>

#include "Assert.h"
#include "Constants.h"
#include "Trace.h"

#define REGEX_CONSTANT "(_*[a-z][A-Za-z0-9]*)"
#define REGEX_INTEGER "([0-9]*)"
#define REGEX_ATOM REGEX_CONSTANT "(\\(((,)?(" REGEX_CONSTANT "|" REGEX_INTEGER "))*\\))?"

map< string, string > RuleNames::ruleMap;
map< string, vector< string > > RuleNames::variablesMap;

const string&
RuleNames::getRule(
    const string& debugAtom )
{
    unsigned int parenthesisIndex = debugAtom.find( '(' );

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
RuleNames::getGroundRule(
    const string& debugAtom )
{
    string rule = getRule( debugAtom );
    map< string, string > substitution = getSubstitutionMap( debugAtom );

    for ( const auto& pair : substitution )
    {
        regex variableRegex( "([,;( ])" + pair.first + "(?=[,;) ])" );
        rule = regex_replace( rule, variableRegex, "$1@" + pair.first + "@" );
        regex variableRegex2( "@" + pair.first + "@" );
        rule = regex_replace( rule, variableRegex2, pair.second );
    }

    return rule;
}

string
RuleNames::getSubstitution(
    const string& debugAtom )
{
    string substitution = "";
    map< string, string > substitutionMap = getSubstitutionMap( debugAtom );

    if ( substitutionMap.size() > 0 )
    {
        substitution = "{ ";
        auto iterator = substitutionMap.begin();
        substitution += iterator->first + "/" + iterator->second;

        for ( iterator ++; iterator != substitutionMap.end(); iterator ++ )
        {
            substitution += ", " + iterator->first + "/" + iterator->second;
        }
        substitution += " }";
    }

    return substitution;
}

map< string, string >
RuleNames::getSubstitutionMap(
    const string& debugAtom )
{
    map< string, string > substitution;

    if ( debugAtom.find( '(' ) != string::npos )
    {
        std::size_t parenthesisIndex = debugAtom.find( '(' );
        string debugConstant = debugAtom.substr( 0, parenthesisIndex );
        string term = debugAtom.substr( parenthesisIndex + 1, debugAtom.length() - parenthesisIndex - 2);

        vector< string > terms = getTerms( term );
        vector< string > variables = variablesMap[ debugConstant ];

        assert_msg( terms.size() == variables.size(), "#terms to be substituted is different from #variables" );

        for ( unsigned int i = 0; i < terms.size(); i++ )
        {
            substitution[ variables[ i ] ] = terms[ i ];
        }
    }

    return substitution;
}

void
RuleNames::addRule(
    string debugAtom,
    string rule,
    vector< string > variables )
{
    ruleMap[ debugAtom ] = rule;
    variablesMap[ debugAtom ] = variables;
}

vector< string >
RuleNames::getTerms(
    const string& term )
{
    int openBrackets = 0; // required for nested terms, i.e. _debug1(a(b,c),d)
    vector< string > terms;
    string currentTerm = "";

    for ( unsigned int i = 0; i < term.length(); i++ )
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

    if ( term.length() > 0 )
        terms.push_back( currentTerm );

    return terms;
}

vector< Var >
RuleNames::getVariables(
    const string& debugAtom )
{
    vector< Var > variables;
    string groundRule = getGroundRule( debugAtom );

    groundRule.erase( remove_if( groundRule.begin(), groundRule.end(), ::isspace ), groundRule.end() );

    regex atomRegex( REGEX_ATOM );
    smatch atomMatch;

    while( regex_search( groundRule, atomMatch, atomRegex ) )
    {
        Var variable;
        if ( VariableNames::getVariable( atomMatch.str(), variable ) )
        {
            variables.push_back( variable );
        }
        groundRule = atomMatch.suffix().str();
    }

    return variables;
}
