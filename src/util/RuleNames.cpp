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
#include <iostream>

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
    std::size_t parenthesisIndex = debugAtom.find( '(' );

    if ( string::npos == parenthesisIndex )
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

vector< Literal >
RuleNames::getLiterals(
    const string& debugAtom )
{
    vector< Literal > literals;
    string groundRule = getGroundRule( debugAtom );

    // remove all spaces
    groundRule.erase( remove_if( groundRule.begin(), groundRule.end(), ::isspace ), groundRule.end() );

    regex atomRegex( REGEX_ATOM );
    smatch atomMatch;

    while( regex_search( groundRule, atomMatch, atomRegex ) )
    {
        Var variable;
        unsigned int sign = POSITIVE;
        string atom = atomMatch.str();

        // remove any prefix "not" due to initial removal of all spaces
        if ( atom.find( "not" ) == 0 )
        {
            atom = atom.substr( 3 );
            sign = NEGATIVE;
        }

        if ( VariableNames::getVariable( atom, variable ) )
        {
            literals.push_back( Literal( variable, sign ) );
        }
        groundRule = atomMatch.suffix().str();
    }

    return literals;
}

vector< Var >
RuleNames::getVariables(
    const string& debugAtom )
{
    vector< Var > variables;
    for ( const Literal& literal : getLiterals( debugAtom ) )
    {
        variables.push_back( literal.getVariable() );
    }

    return variables;
}

vector< Var >
RuleNames::getVariablesOfSupportingRules(
    const string& atom )
{
    vector< Var > variables;

    Var atomVariable = 0;
    VariableNames::getVariable( atom, atomVariable );

    // get the predicate symbol out of the atom
    string predicateSymbol = atom;
    vector< string > terms;
    std::size_t parenthesisPos = atom.find('(');
    if ( parenthesisPos != string::npos )
    {
        predicateSymbol = atom.substr( 0, parenthesisPos );
        std::size_t termStart = atom.find( '(' ) + 1;
        std::size_t termLength = atom.length() - termStart - 1;
        terms = getTerms( atom.substr( termStart, termLength ) );
    }

    vector< pair< string, map< string, string > > > debugSymbolsWithPartialSubstitution;

    for ( const auto& ruleMapEntry : ruleMap )
    {
        string head = ruleMapEntry.second;
        char separator = '|';

        if ( head.find( ":-" ) != string::npos )
        {
            head = ruleMapEntry.second.substr( 0, head.find( ":-" ) );
        }
        else if ( head.find( '|' ) == string::npos && head.find( '{' ) == string::npos )
        {
            // no disjunction, choice rule or normal rule -> skip
            head = "";
        }

        if ( head.find( '{' ) != string::npos )
        {
            // choice rule, get the choices
            separator = ';';
            std::size_t openPos = head.find( '{' );
            std::size_t closePos = head.find( '}' );

            head = head.substr( openPos + 1, closePos - openPos - 1 );
        }

        // remove all spaces
        head.erase( remove_if( head.begin(), head.end(), ::isspace ), head.end() );
        if ( head[ head.length() - 1 ] == '.' )
        {
            // delete dot at the end
            head = head.substr( 0, head.length() - 1 );
        }

        // iterate over all head atoms and check for a match
        while ( head.length() > 0 )
        {
            std::size_t separatorPos = head.find( separator );
            string headAtom = head.substr( 0, separatorPos );

            if ( separatorPos != string::npos ) head = head.substr( separatorPos + 1 );
            else head = "";

            if ( headAtom.find( predicateSymbol ) == 0
                 && ( headAtom.length() == predicateSymbol.length()
                      || headAtom[ predicateSymbol.length() ] == '(' ) )
            {
                vector< string > headAtomVariables;

                // check if they have the same arity
                if ( headAtom.find( '(' ) != string::npos )
                {
                    std::size_t termStart = headAtom.find( '(' ) + 1;
                    std::size_t termLength = headAtom.length() - termStart - 1;
                    headAtomVariables = getTerms( headAtom.substr( termStart, termLength ) );

                }

                if ( headAtomVariables.size() == terms.size() )
                {
                    // same arity --> build substitution
                    map< string, string > substitution;
                    bool matchingSubstitution = true;

                    for ( unsigned int i = 0; i < headAtomVariables.size(); i ++ )
                    {
                        // handle case where a variable of the head atom occurs twice.
                        // e.g. pred(1,2) and pred(X,X) are not compliant
                        if ( substitution.count( headAtomVariables[ i ] ) == 0 )
                        {
                            substitution[ headAtomVariables[ i ] ] = terms[ i ];
                        }
                        else if ( substitution[ headAtomVariables[ i ] ] != terms[ i ] )
                        {
                            matchingSubstitution = false;
                        }
                    }

                    if ( matchingSubstitution )
                    {
                        debugSymbolsWithPartialSubstitution.push_back( make_pair( ruleMapEntry.first, substitution ) );
                    }
                }
            }
        }
    }

    // iterate over the found symbols with their substitutions
    for ( const auto& debugSymbolWithSubstitution : debugSymbolsWithPartialSubstitution )
    {
        string debugSymbol = debugSymbolWithSubstitution.first;
        map< string, string > partialSubstitution = debugSymbolWithSubstitution.second;

        // find the corresponding debug symbol inside the variable names
        for ( const string& variableName : VariableNames::getVariableNames() )
        {
            if ( variableName.find( debugSymbol ) != string::npos )
            {
                // candidate, check for substitution
                map< string, string > substitution = getSubstitutionMap( variableName );

                bool substitutionMatches = true;

                // check if the partial substitution matches
                for ( const auto& sub : partialSubstitution )
                {
                    if ( substitution[ sub.first ] != sub.second )
                    {
                        substitutionMatches = false;
                    }
                }

                // extract H(r) \ { atom } U B+(r) U { a | not a in B-(r) }
                if ( substitutionMatches )
                {
                    // get all boolean variables of the ground rule
                    for ( Var v : getVariables( variableName ) )
                    {
                        if ( v != atomVariable )
                            variables.push_back( v );
                    }
                }
            }
        }
    }

    return variables;
}

vector< pair< string, vector< Literal > > >
RuleNames::getSupportingRules(
    const string& atom )
{
    vector< pair< string, vector< Literal > > > supportingRules;

    Var atomVariable = 0;
    VariableNames::getVariable( atom, atomVariable );

    // get the predicate symbol out of the atom
    string predicateSymbol = atom;
    vector< string > terms;
    std::size_t parenthesisPos = atom.find('(');
    if ( parenthesisPos != string::npos )
    {
        predicateSymbol = atom.substr( 0, parenthesisPos );
        std::size_t termStart = atom.find( '(' ) + 1;
        std::size_t termLength = atom.length() - termStart - 1;
        terms = getTerms( atom.substr( termStart, termLength ) );
    }

    vector< pair< string, map< string, string > > > debugSymbolsWithPartialSubstitution;

    for ( const auto& ruleMapEntry : ruleMap )
    {
        string head = ruleMapEntry.second;
        char separator = '|';

        if ( head.find( ":-" ) != string::npos )
        {
            head = ruleMapEntry.second.substr( 0, head.find( ":-" ) );
        }
        else if ( head.find( '|' ) == string::npos && head.find( '{' ) == string::npos )
        {
            // no disjunction, choice rule or normal rule -> skip
            head = "";
        }

        if ( head.find( '{' ) != string::npos )
        {
            // choice rule, get the choices
            separator = ';';
            std::size_t openPos = head.find( '{' );
            std::size_t closePos = head.find( '}' );

            head = head.substr( openPos + 1, closePos - openPos - 1 );
        }

        // remove all spaces
        head.erase( remove_if( head.begin(), head.end(), ::isspace ), head.end() );
        if ( head[ head.length() - 1 ] == '.' )
        {
            // delete dot at the end
            head = head.substr( 0, head.length() - 1 );
        }

        // iterate over all head atoms and check for a match
        while ( head.length() > 0 )
        {
            std::size_t separatorPos = head.find( separator );
            string headAtom = head.substr( 0, separatorPos );

            if ( separatorPos != string::npos ) head = head.substr( separatorPos + 1 );
            else head = "";

            if ( headAtom.find( predicateSymbol ) == 0
                 && ( headAtom.length() == predicateSymbol.length()
                      || headAtom[ predicateSymbol.length() ] == '(' ) )
            {
                vector< string > headAtomVariables;

                // check if they have the same arity
                if ( headAtom.find( '(' ) != string::npos )
                {
                    std::size_t termStart = headAtom.find( '(' ) + 1;
                    std::size_t termLength = headAtom.length() - termStart - 1;
                    headAtomVariables = getTerms( headAtom.substr( termStart, termLength ) );

                }

                if ( headAtomVariables.size() == terms.size() )
                {
                    // same arity --> build substitution
                    map< string, string > substitution;
                    bool matchingSubstitution = true;

                    for ( unsigned int i = 0; i < headAtomVariables.size(); i ++ )
                    {
                        // handle case where a variable of the head atom occurs twice.
                        // e.g. pred(1,2) and pred(X,X) are not compliant
                        if ( substitution.count( headAtomVariables[ i ] ) == 0 )
                        {
                            substitution[ headAtomVariables[ i ] ] = terms[ i ];
                        }
                        else if ( substitution[ headAtomVariables[ i ] ] != terms[ i ] )
                        {
                            matchingSubstitution = false;
                        }
                    }

                    if ( matchingSubstitution )
                    {
                        debugSymbolsWithPartialSubstitution.push_back( make_pair( ruleMapEntry.first, substitution ) );
                    }
                }
            }
        }
    }

    // iterate over the found symbols with their substitutions
    for ( const auto& debugSymbolWithSubstitution : debugSymbolsWithPartialSubstitution )
    {
        string debugSymbol = debugSymbolWithSubstitution.first;
        map< string, string > partialSubstitution = debugSymbolWithSubstitution.second;

        // find the corresponding debug symbol inside the variable names
        for ( const string& variableName : VariableNames::getVariableNames() )
        {
            if ( variableName.find( debugSymbol ) != string::npos )
            {
                // candidate, check for substitution
                map< string, string > substitution = getSubstitutionMap( variableName );

                bool substitutionMatches = true;

                // check if the partial substitution matches
                for ( const auto& sub : partialSubstitution )
                {
                    if ( substitution[ sub.first ] != sub.second )
                    {
                        substitutionMatches = false;
                    }
                }

                // extract H(r) \ { atom } U B+(r) U { a | not a in B-(r) }
                if ( substitutionMatches )
                {
                    // get all literals of the ground rule
                    vector< Literal > literals;

                    string groundRule = getGroundRule( variableName );
                    string groundHead = groundRule;

                    size_t consPos = groundRule.find( ":-" );
                    if ( consPos != string::npos )
                    {
                        groundHead = groundRule.substr( 0, consPos );
                    }

                    // normalize the head: remove all spaces and
                    // 'l { a1;...;an } u', 'a1|...|an' to 'a1,...,an'
                    if ( groundHead.find( '{' ) != string::npos )
                    {
                        size_t openCurlyPos = groundHead.find('{');
                        size_t closeCurlyPos = groundHead.find('}');
                        groundHead = groundHead.substr( openCurlyPos + 1, closeCurlyPos - openCurlyPos - 1 );
                        replace( groundHead.begin(), groundHead.end(), ';', ',' );
                    }
                    else if ( groundHead.find( '|' ) != string::npos )
                    {
                        replace( groundHead.begin(), groundHead.end(), '|', ',' );
                    }

                    groundHead.erase( remove_if( groundHead.begin(), groundHead.end(), ::isspace ), groundHead.end() );

                    for ( const Literal& literal : getLiterals( variableName ) )
                    {
                        if ( literal.getVariable() != atomVariable )
                        {
                            // if the literal appears in the head --> flip it
                            if ( groundHead.find( VariableNames::getName( literal.getVariable() ) ) != string::npos )
                            {
                                literals.push_back( literal.getOppositeLiteral() );
                            }
                            else
                            {
                                literals.push_back( literal );
                            }
                        }
                    }

                    // construct the pair
                    supportingRules.push_back( make_pair( getGroundRule( variableName ), literals ) );
                }
            }
        }
    }

    return supportingRules;
}
