//---------------------------------------------------------------------------------------
//  This file is part of the Lomse library.
//  Copyright (c) 2010-2011 Lomse project
//
//  Lomse is free software; you can redistribute it and/or modify it under the
//  terms of the GNU General Public License as published by the Free Software Foundation,
//  either version 3 of the License, or (at your option) any later version.
//
//  Lomse is distributed in the hope that it will be useful, but WITHOUT ANY
//  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
//  PARTICULAR PURPOSE.  See the GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License along
//  with Lomse; if not, see <http://www.gnu.org/licenses/>.
//
//  For any comment, suggestion or feature request, please contact the manager of
//  the project at cecilios@users.sourceforge.net
//
//---------------------------------------------------------------------------------------

#ifndef __LOMSE_VISITOR_H__
#define __LOMSE_VISITOR_H__

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
	virtual void start_visit(T* pElement) {}
	virtual void end_visit(T* pElement) {}

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
