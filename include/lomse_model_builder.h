//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-2012 Cecilio Salmeron. All rights reserved.
//
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
//
//    * Redistributions of source code must retain the above copyright notice, this 
//      list of conditions and the following disclaimer.
//
//    * Redistributions in binary form must reproduce the above copyright notice, this
//      list of conditions and the following disclaimer in the documentation and/or
//      other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY 
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
// OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
// SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
// TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
// BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
// ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
// DAMAGE.
//
// For any comment, suggestion or feature request, please contact the manager of
// the project at cecilios@users.sourceforge.net
//---------------------------------------------------------------------------------------

#ifndef __LOMSE_MODEL_BUILDER_H__
#define __LOMSE_MODEL_BUILDER_H__

#include <ostream>

using namespace std;

namespace lomse
{

//forward declarations
class InternalModel;
class ImoDocument;
class ImoObj;
class ImoScore;
class ImoKeySignature;
class ImoNote;

//---------------------------------------------------------------------------------------
// ModelBuilder. Implements the final step of LDP compiler: code generation.
// Traverses the parse tree and creates the internal model
class ModelBuilder
{
public:
    ModelBuilder() {}
    virtual ~ModelBuilder() {}

    ImoDocument* build_model(InternalModel* IModel);
    void structurize(ImoObj* pImo);

};

//---------------------------------------------------------------------------------------
// PitchAssigner. Implements the algorithm to traverse the score and assign pitch to
// notes, based on notated pitch, and taking into account key signature and notated
// accidentals introduced by previous notes on the same measure.
class PitchAssigner
{
protected:
    int m_accidentals[7];

public:
    PitchAssigner();
    virtual ~PitchAssigner() {}

    void assign_pitch(ImoScore* pScore);


protected:
    void reset_accidentals(ImoKeySignature* pKey);
    void update_context_accidentals(ImoNote* pNote);
    void compute_pitch(ImoNote* pNote);

};


}   //namespace lomse

#endif      //__LOMSE_MODEL_BUILDER_H__
