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

#ifndef DEBUGUSERINTERFACEGUI_H
#define DEBUGUSERINTERFACEGUI_H

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
    void printHistory( const vector< Literal >& assertionHistory ) {};
    string askHistoryFilename() { return ""; };
    void queryResponse( const vector< Var >& variables );
    vector< Literal > getAssertions();
    unsigned int chooseAssertionToUndo( const vector< Literal >& assertionHistory );
    TruthValue askUnfoundedTruthValue( const Var& variable );
    void greetUser() {};
    void informComputingCore();
    void informComputingQuery();
    void informUnfoundedCase();
    void informPossiblySupportingRule( const Literal& unfoundedAssertion, const string& supportingRule );
    void informSavedHistory( const string& filename ) {};
    void informLoadedHistory( const string& filename ) {};
    void informCouldNotSaveHistory( const string& filename ) {};
    void informCouldNotLoadHistory( const string& filename ) {};
    void informAnalyzedDisjointCores( const unsigned int numCores ) {};
    void informAssertionAlreadyPresent( const string& variable ) {};
    void informAssertionIsFact( const string& variable ) {};
    void informProgramCoherent( const vector< Var >& answerSet );

private:
    string lastMessage;
};

#endif /* DEBUGUSERINTERFACEGUI_H */
