//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
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
