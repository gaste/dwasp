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

#ifndef DEBUGUSERINTERFACECLI_H
#define DEBUGUSERINTERFACECLI_H

#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "DebugUserInterface.h"
#include "Literal.h"
#include "util/Constants.h"

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
	void printCore( const vector< Literal >& core );
	void printCoreGroundRules( const vector< Literal >& core );
    void printCoreUngroundRules( const vector< Literal >& core );
	void printHistory( const vector< Literal >& assertionHistory );
	string askHistoryFilename();
	TruthValue askTruthValue( const Var variable );
	Literal getAssertion();
	unsigned int chooseAssertionToUndo( const vector< Literal >& assertionHistory );
    void greetUser();
    void informSolving();
    void informComputingQueryVariable();
    void informSavedHistory( const string& filename );
    void informLoadedHistory( const string& filename );
    void informCouldNotSaveHistory( const string& filename );
    void informCouldNotLoadHistory( const string& filename );
    void informAnalyzedDisjointCores( const unsigned int numCores );
    void informAssertionAlreadyPresent( const string& variable );
    void informAssertionIsFact( const string& variable );
    void informNoQueryPossible();

private:
	inline void promptInput(string& input) { cout << "WDB> "; getline(cin, input); }
	void printHelp();
	static map< string, cmd > commandMap;
};

#endif /* DEBUGUSERINTERFACECLI_H */
