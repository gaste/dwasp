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

#include "Dimacs.h"

#include "../Solver.h"
#include "../Clause.h"
#include "../util/Constants.h"
#include "../util/ErrorMessage.h"
#include "../util/Trace.h"
#include "../util/Options.h"

#include <cassert>
#include <sstream>
#include <unordered_set>
using namespace std;

Dimacs::Dimacs(
    Solver& s ) : solver( s )
{
}

//
//DimacsFormat::DimacsFormat(
//    ProgramBuilder* p ) : programBuilder( p ), weighted( false ), 
//                          cnfWeighted( false ), maxWeight( 0 )
//{
//}
//

void
Dimacs::parse()
{
    this->parse( cin );
}

void
Dimacs::parse(
    istream& input )
{
    char type;
    
    while( input >> type )
    {
        switch ( type )
        {
        case COMMENT_DIMACS:
            readComment( input );
            break;

        case FORMULA_INFO_DIMACS:
            readFormulaInfo( input );            
            insertVariables( numberOfVariables );
            //readAllClauses( input );
            break;
            
        default:
			input.putback( type );
			readClause( input );
            //ErrorMessage::errorDuringParsing( "Unexpected symbol.");
        }
    }
    if( !input.good() && !input.eof() )
    {
        ErrorMessage::errorDuringParsing( "Unexpected symbol.");
    }
}

void
Dimacs::readComment( 
    istream & input )
{
    char buf[ 10240 ];

    input.getline( buf, 10240 );
    if( input.gcount() > 10239 )
        ErrorMessage::errorDuringParsing( "Comment too long.");
}

void
Dimacs::readFormulaInfo(
    istream& input )
{
    char type;
    input >> type;
    
//    if( type == 'w' )
//    {
//        weighted = true;
//        input >> type;
//        if( type != 'c' )
//        {
//            cerr << ERRORPARSING << endl;
//            exit( 0 );
//        }
//    }
    /*else*/ 
    if( type != 'c' )
    {
        ErrorMessage::errorDuringParsing( "Expected a 'c'.");
    }
        
    input >> type;
    if( type != 'n' )
    {
        ErrorMessage::errorDuringParsing( "Expected a 'n'.");        
    }
    
    input >> type;
    if( type != 'f' )
    {
        ErrorMessage::errorDuringParsing( "Expected a 'f'.");
    }
    
    input >> numberOfVariables;    
    input >> numberOfClauses;
    
//    if( weighted )
//        input >> maxWeight;
//    else if( wasp::options.dimacsMaxsat )
//    {
//        weighted = true;        
//        cnfWeighted = true;
//        maxWeight = 2;
//    }       
}

void
Dimacs::readAllClauses(
    istream& input )
{
//    if( !weighted )
//    {
    trace_msg( parser, 1, "Starting iteration on " << numberOfClauses << " clauses.");
    for( unsigned int i = 0; i < numberOfClauses; i++ )
    {
        readClause( input );
    }
//    }
//    else if( !cnfWeighted )
//    {
//        for( unsigned int i = 0; i < numberOfClauses; i++ )
//        {
//            readClauseWeighted( input );
//        }
//    }
//    else
//    {
//        for( unsigned int i = 0; i < numberOfClauses; i++ )
//        {
//            readClauseCnfWeighted( input );
//        }
//    }
    
//    for( unsigned int i = 1; i <= numberOfVariables; i++ )
//    {
//        if( variablesAppearingWithBothPolarity.find( i ) == variablesAppearingWithBothPolarity.end() )
//        {
//            if( readVariables.find( i ) != readVariables.end() )
//            {
//                programBuilder->addTrueSupportedAtom( i );
//            }
//            else
//            {
//                programBuilder->addFalseAtom( i );
//            }
//        }
//    }
}

void
Dimacs::readClause(
    istream& input )
{   
    unordered_set< int > tempSet;

    bool trivial = false;

    Clause* clause = new Clause();

    //a variable containing the literal to read
    int next;

    //read the next literal
    input >> next;

    if( next == 0 )
    {
        ErrorMessage::errorDuringParsing( "Empty clause are not allowed.");
    }

    //insert the first literal in the set
    tempSet.insert( next );    
    Literal literal = solver.getLiteral( next );
    clause->addLiteral( literal );        

    int firstLiteral = next;
    input >> next;   
    while( next != 0 )
    {
        //insert the current literal in the set
        tempSet.insert( next );
        
        //if the opposite literal was already in the set the clause is a tautology
        if( tempSet.find( -next ) != tempSet.end() )
            trivial = true;

        Literal literal = solver.getLiteral( next );
        clause->addLiteral( literal );
        literal.addClause( clause );
        input >> next;
    }    
    
    if( clause->size() == 1 )
    {
        trivial = true;
        if( addedLiterals.insert( firstLiteral ).second )
        {
            trace_msg( parser, 2, "Unary clause with literal " << solver.getLiteral( firstLiteral ) << ". Adding literal to the solver." );
            solver.addTrueLiteral( solver.getLiteral( firstLiteral ) );
        }
        else
        {
            trace_msg( parser, 2, "Found duplicated unary clause with literal " << solver.getLiteral( firstLiteral ) << ". Skipping this clause." );
        }
    }
    
    //if the clause is not trivial add it in the formula
    if( !trivial )
    {
        trace_msg( parser, 1, "Adding clause " << *clause << " in solver.");
        solver.addClause( clause );
        solver.getLiteral( firstLiteral ).addClause( clause );
    }
    else
    {
        trace_msg( parser, 1, "Deleting clause " << *clause << " because is trivial." );
        delete clause;
    }
}
//
//void
//DimacsFormat::readClauseWeighted(
//    istream& input )
//{
//    unordered_set< int > tempSet;
//    unsigned int weight;
//    input >> weight;
//    
//    if( weight == maxWeight )
//    {
//        readClause( input );
//        return;
//    }
//    
//    List< int > clause;
//    bool trivial = false;
//    
//    int next;
//    input >> next;
//    do
//    {
//        assert( next != 0 );
//        
//        tempSet.insert( next );
//        
//        if( tempSet.find( -next ) != tempSet.end() )
//            trivial = true;                
//        
//        //Change polarity
//        clause.push_back( -next );
//        input >> next; 
//    } while( next != 0 );
//    
//    if( trivial )
//        return;
//    
//    WeakConstraint* w;
//    if( wasp::options.weakConstraintsHeuristic || clause.size() == 1 )
//        w = programBuilder->startWeakConstraint( clause.size() + 1, weight, 1 );
//    else
//        w = programBuilder->startWeakConstraint( clause.size(), weight, 1 );    
//    
//    while( !clause.empty() )
//    {
//        int id = clause.front();
//        if( readVariables.find( id ) != readVariables.end() )
//            variablesAppearingWithBothPolarity.insert( abs( id ) );
//        readVariables.insert( -id );
//        
//        if( id < 0 )
//            programBuilder->addLiteralInWeakConstraint( w, -id, false );
//        else
//            programBuilder->addLiteralInWeakConstraint( w, id, true );        
//        clause.pop_front();
//    }
//    programBuilder->endWeakConstraint( w );        
//    
//    assert( clause.empty() );
//}
//
//void
//DimacsFormat::readClauseCnfWeighted(
//    istream& input )
//{
//    List< int > clause;
//    bool trivial = false;
//    unordered_set< int > tempSet;
//    
//    int next;
//    input >> next;
//    do
//    {
//        assert( next != 0 );
//        
//        tempSet.insert( next );
//        
//        if( tempSet.find( -next ) != tempSet.end() )
//            trivial = true;
//        
//        //Change polarity
//        clause.push_back( -next );
//        input >> next; 
//    } while( next != 0 );
//    
//    if( trivial )
//        return;
//    
//    WeakConstraint* w;
//    if( wasp::options.weakConstraintsHeuristic || clause.size() == 1 )
//        w = programBuilder->startWeakConstraint( clause.size() + 1, 1, 1 );
//    else
//        w = programBuilder->startWeakConstraint( clause.size(), 1, 1 );    
//    
//    while( !clause.empty() )
//    {
//        int id = clause.front();
//        
//        if( readVariables.find( id ) != readVariables.end() )
//            variablesAppearingWithBothPolarity.insert( abs( id ) );
//        readVariables.insert( -id );
//        
//        if( id < 0 )
//            programBuilder->addLiteralInWeakConstraint( w, -id, false );
//        else
//            programBuilder->addLiteralInWeakConstraint( w, id, true );        
//        clause.pop_front();
//    }
//    programBuilder->endWeakConstraint( w );        
//    
//    assert( clause.empty() );
//}

void
Dimacs::insertVariables(
    unsigned int numberOfVariables )
{
    trace_msg( parser, 1, "Adding " << numberOfVariables << " variables." );
    for( unsigned int i = 1; i <= numberOfVariables; i++ )
    {
        stringstream ss;
        ss << i;
        solver.addVariable( ss.str() );
    }
}
