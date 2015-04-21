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
#include <string>
#include <vector>

#include "DebugUserInterface.h"
#include "Literal.h"
#include "util/Constants.h"

/**
 * CLI implementation of the debug user interface.
 */
class DebugUserInterfaceCLI : public DebugUserInterface
{
public:
	DebugUserInterfaceCLI() {};
	~DebugUserInterfaceCLI() {};
	UserCommand promptCommand();
	void printCore( vector< Literal >& literals );
	void printHistory( vector< Var > queryHistory, vector< TruthValue > answerHistory );
	TruthValue askTruthValue( Var variable );

private:
	inline void promptInput (string& input) { cout << endl << "WDB> "; getline(cin, input); }
};

#endif /* DEBUGUSERINTERFACECLI_H */
