//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
//---------------------------------------------------------------------------------------

#ifndef __LOMSE_FUNCTOR_H__
#define __LOMSE_FUNCTOR_H__

namespace lomse
{

template <typename T> class Functor
{
public:
	virtual ~Functor() {}
	virtual T operator ()() = 0;
};

}   //namespace lomse

#endif      //__LOMSE_FUNCTOR_H__
