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

#ifndef RULENAMES_H
#define RULENAMES_H

#include <map>
#include <string>
#include <vector>

#include "../Literal.h"
#include "VariableNames.h"

using namespace std;

class RuleNames
{
    public:
        static void addRule( string debugAtom, string rule, vector< string > variables );
        static const string& getRule( const Literal& debugLiteral ) { return getRule( debugLiteral.getVariable() ); }
        static const string& getRule( const Var debugVariable ) { return getRule( VariableNames::getName( debugVariable ) ); }
        static const string& getRule( const string& debugAtom );
        static string getGroundRule( const Literal& debugLiteral ) { return getGroundRule( debugLiteral.getVariable() ); }
        static string getGroundRule( const Var debugVariable ) { return getGroundRule( VariableNames::getName( debugVariable ) ); }
        static string getGroundRule( const string& debugAtom );
        static string getSubstitution( const Literal& debugLiteral ) { return getSubstitution( debugLiteral.getVariable() ); };
        static string getSubstitution( const Var debugVariable ) { return getSubstitution( VariableNames::getName( debugVariable ) ); };
        static string getSubstitution( const string& debugAtom );
        static map< string, string > getSubstitutionMap( const Literal& debugLiteral ) { return getSubstitutionMap( debugLiteral.getVariable() ); }
        static map< string, string > getSubstitutionMap( const Var debugVariable ) { return getSubstitutionMap( VariableNames::getName( debugVariable ) ); }
        static map< string, string > getSubstitutionMap( const string& debugAtom );
        static vector< Var > getVariables( const Literal& debugLiteral ) { return getVariables( debugLiteral.getVariable() ); }
        static vector< Var > getVariables( const Var debugVariable ) { return getVariables( VariableNames::getName( debugVariable ) ); }
        static vector< Var > getVariables( const string& debugAtom );

    private:
        static vector< string > getTerms( const string& term );
        static map< string, string > ruleMap;
        static map< string, vector< string > > variablesMap;
};

#endif /* RULENAMES_H */
