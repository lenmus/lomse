//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Lomse is copyrighted work (c) 2010-2016. All rights reserved.
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

#ifndef __LOMSE_VISITOR_H__
#define __LOMSE_VISITOR_H__

//---------------------------------------------------------------------------------------
// macro for avoiding warnings when a parameter is not used
#ifdef UNUSED
#elif defined(__GNUC__)
    #define UNUSED(x) UNUSED_ ## x __attribute__((unused))
#elif defined(__LCLINT__)
    #define UNUSED(x) /*@unused@*/ x
#else
    #define UNUSED(x) /* x */
#endif



namespace lomse
{

//---------------------------------------------------------------------------------------
// The root base class for all visitors
class BaseVisitor
{
public:
	virtual ~BaseVisitor() {}

protected:
    BaseVisitor() {}

};


//---------------------------------------------------------------------------------------
// Base class for visitors
template<class T>
class Visitor : virtual public BaseVisitor
{
public:
	virtual ~Visitor() {}
	virtual void start_visit(T* UNUSED(pElement)) {}
	virtual void end_visit(T* UNUSED(pElement)) {}

protected:
    Visitor() : BaseVisitor() {}

};


//---------------------------------------------------------------------------------------
// Base class for objects accepting visitors
class Visitable
{
public:
	virtual ~Visitable() {}
	virtual void accept_visitor(BaseVisitor& v)=0;
};


} //namespace lomse


#endif      //__LOMSE_VISITOR_H__
