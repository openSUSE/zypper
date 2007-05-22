/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp2/repository/memory/DPatternImpl.cc
 *
*/
#include "zypp2/repository/memory/DPatternImpl.h"

using namespace std;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
namespace repository
{ /////////////////////////////////////////////////////////////////
namespace memory
{
///////////////////////////////////////////////////////////////////
//
//	METHOD NAME : PatternImpl::PatternImpl
//	METHOD TYPE : Ctor
//
DPatternImpl::DPatternImpl(data::Pattern_Ptr ptr)
{}

///////////////////////////////////////////////////////////////////
//
//	METHOD NAME : PatternImpl::~PatternImpl
//	METHOD TYPE : Dtor
//
DPatternImpl::~DPatternImpl()
{}

Source_Ref DPatternImpl::source() const
{
  return Source_Ref::noSource;
}

TranslatedText DPatternImpl::summary() const
{
  return _summary;
}

TranslatedText DPatternImpl::description() const
{
  return _description;
}

TranslatedText DPatternImpl::category() const
{
  return _category;
}

bool DPatternImpl::userVisible() const
{
  return _visible;
}

Label DPatternImpl::order() const
{
  return _order;
}

Pathname DPatternImpl::icon() const
{
  return _icon;
}

/////////////////////////////////////////////////////////////////
} // namespace detail
///////////////////////////////////////////////////////////////////
}
/////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
