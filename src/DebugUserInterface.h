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

#ifndef DEBUGUSERINTERFACE_H
#define DEBUGUSERINTERFACE_H

#include <vector>

#include "Literal.h"
#include "util/Constants.h"

using namespace std;

/**
 * Commands available to the user
 */
enum UserCommand {
	SHOW_CORE,
	SHOW_CORE_GROUND_RULES,
	SHOW_CORE_NONGROUND_RULES,
	SHOW_HISTORY,
    ASK_QUERY,
    SAVE_HISTORY,
    LOAD_HISTORY,
    ASSERT_VARIABLE,
	EXIT
};

/**
 * Specifies methods to interact with the user
 */
class DebugUserInterface
{
public:
	virtual ~DebugUserInterface() = 0;
	virtual UserCommand promptCommand() = 0;
	virtual void printCore( const vector< Literal >& core ) = 0;
	virtual void printCoreGroundRules( const vector< Literal >& core ) = 0;
	virtual void printCoreUngroundRules( const vector< Literal >& core ) = 0;
	virtual void printHistory( const vector< Var >& queryHistory, const vector< TruthValue >& answerHistory ) = 0;
	virtual string askHistoryFilename() = 0;
	virtual TruthValue askTruthValue( const Var variable ) = 0;
	virtual Literal getAssertion() = 0;
	virtual void greetUser() = 0;
	virtual void informSolving() = 0;
	virtual void informComputingQueryVariable() = 0;
	virtual void informSavedHistory( const string& filename ) = 0;
	virtual void informLoadedHistory( const string& filename ) = 0;
	virtual void informCouldNotSaveHistory( const string& filename ) = 0;
	virtual void informCouldNotLoadHistory( const string& filename ) = 0;
};

inline DebugUserInterface::~DebugUserInterface() { }

#endif /* DEBUGUSERINTERFACE_H */
