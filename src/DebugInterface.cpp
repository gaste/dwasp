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

#include "DebugInterface.h"
#include "Solver.h"

void
DebugInterface::debug()
{
    for(unsigned int i = 0; i < assumptions.size(); i++)
        cout << "ASSUM " << assumptions[i] << endl;
    vector< Literal > assumptionsOR;
    solver.setComputeUnsatCores(true);
    solver.minimizeUnsatCore(true);
    while(solver.solve( assumptions, assumptionsOR ) == INCOHERENT)
    {
        assert( solver.getUnsatCore() != NULL );
        const Clause& unsatCore = *(solver.getUnsatCore());
        for( unsigned int i = 0; i < unsatCore.size(); i++ )
        {
            Var v = unsatCore[ i ].getVariable();
            cout << "_debug ----> " << v << endl;
        }
        //doSomething;
    }

    assert(0);
}
