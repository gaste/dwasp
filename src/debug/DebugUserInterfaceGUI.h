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

#ifndef DEBUGUSERINTERFACEPROTOCOLBUFFERS_H
#define DEBUGUSERINTERFACEPROTOCOLBUFFERS_H

#include <iostream>
#include <string>
#include <vector>

#include "../Literal.h"
#include "../util/Constants.h"
#include "DebugUserInterface.h"

using namespace std;

/**
 * Implementation of the debug user interface using Protocol Buffers.
 */
class DebugUserInterfaceGUI : public DebugUserInterface
{
public:
    DebugUserInterfaceGUI() {};
    ~DebugUserInterfaceGUI() {};
    UserCommand promptCommand();
    void printCore( const vector< Literal >& core, const vector< Literal >& coreAssertions );
    void printCoreGroundRules( const vector< Literal >& core, const vector< Literal >& coreAssertions ) {};
    void printCoreUngroundRules( const vector< Literal >& core, const vector< Literal >& coreAssertions ) {};
    void printHistory( const vector< Literal >& assertionHistory ) {};
    string askHistoryFilename() { return ""; };
    TruthValue askTruthValue( const Var variable );
    Literal getAssertion() { return Literal::null; };
    unsigned int chooseAssertionToUndo( const vector< Literal >& assertionHistory ) { return 0; };
    void greetUser() {};
    void informSolving() {};
    void informComputingQueryVariable() {};
    void informSavedHistory( const string& filename ) {};
    void informLoadedHistory( const string& filename ) {};
    void informCouldNotSaveHistory( const string& filename ) {};
    void informCouldNotLoadHistory( const string& filename ) {};
    void informAnalyzedDisjointCores( const unsigned int numCores ) {};
    void informAssertionAlreadyPresent( const string& variable ) {};
    void informAssertionIsFact( const string& variable ) {};
    void informNoQueryPossible() {};
    void informProgramCoherent() {};
};

#endif /* DEBUGUSERINTERFACEPROTOCOLBUFFERS_H */
