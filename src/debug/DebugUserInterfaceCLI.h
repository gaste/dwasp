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

#ifndef DEBUGUSERINTERFACECLI_H
#define DEBUGUSERINTERFACECLI_H

#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "../Literal.h"
#include "../util/Constants.h"
#include "DebugUserInterface.h"

using namespace std;

struct cmd
{
    UserCommand command;
    string helpText;
};

/**
 * CLI implementation of the debug user interface.
 */
class DebugUserInterfaceCLI : public DebugUserInterface
{
public:
	DebugUserInterfaceCLI() {};
	~DebugUserInterfaceCLI() {};
	UserCommand promptCommand();
	void printCore( const vector< Literal >& core, const vector< Literal >& coreAssertions );
	void printHistory( const vector< Literal >& assertionHistory );
	void queryResponse( const vector< Var >& variables );
	unsigned int chooseAssertionToUndo( const vector< Literal >& assertionHistory );
	string askHistoryFilename();
    vector< Literal > getAssertions();
    TruthValue askUnfoundedTruthValue( const Var& variable ) { return askTruthValue( variable ); }
    void greetUser();
    void informComputingCore();
    void informComputingQuery();
    void informUnfoundedCase();
    void informPossiblySupportingRule( const Literal& unfoundedAssertion, const string& supportingRule );
    void informSavedHistory( const string& filename );
    void informLoadedHistory( const string& filename );
    void informCouldNotSaveHistory( const string& filename );
    void informCouldNotLoadHistory( const string& filename );
    void informAnalyzedDisjointCores( const unsigned int numCores );
    void informAssertionAlreadyPresent( const string& variable );
    void informAssertionIsFact( const string& variable );
    void informProgramCoherent( const vector< Var >& answerSet );

private:
    TruthValue askTruthValue( const Var variable );
    void printCoreLiterals ( const vector< Literal >& core, const vector< Literal >& coreAssertions );
    void printCoreGroundRules( const vector< Literal >& core, const vector< Literal >& coreAssertions );
    void printCoreNonGroundRules( const vector< Literal >& core, const vector< Literal >& coreAssertions );
	inline void promptInput( string& input ) { cout << "WDB> "; getline( cin, input ); }
	void printHelp();

	static map< string, cmd > commandMap;
	bool forceNextCommand = false;
	UserCommand nextCommand = UserCommand::ASK_QUERY;
	Literal nextAssertion = Literal::null;
};

#endif /* DEBUGUSERINTERFACECLI_H */
