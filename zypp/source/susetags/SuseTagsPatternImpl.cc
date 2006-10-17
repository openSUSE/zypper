/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/source/susetags/SuseTagsPatternImpl.cc
 *
*/
#include "zypp/source/susetags/SuseTagsPatternImpl.h"

using namespace std;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
namespace source
{ /////////////////////////////////////////////////////////////////
namespace susetags
{
///////////////////////////////////////////////////////////////////
//
//	METHOD NAME : PatternImpl::PatternImpl
//	METHOD TYPE : Ctor
//
SuseTagsPatternImpl::SuseTagsPatternImpl()
{}

///////////////////////////////////////////////////////////////////
//
//	METHOD NAME : PatternImpl::~PatternImpl
//	METHOD TYPE : Dtor
//
SuseTagsPatternImpl::~SuseTagsPatternImpl()
{}

Source_Ref SuseTagsPatternImpl::source() const
{
  return _source;
}

TranslatedText SuseTagsPatternImpl::summary() const
{
  return _summary;
}

TranslatedText SuseTagsPatternImpl::description() const
{
  return _description;
}

TranslatedText SuseTagsPatternImpl::category() const
{
  return _category;
}

bool SuseTagsPatternImpl::userVisible() const
{
  return _visible;
}

Label SuseTagsPatternImpl::order() const
{
  return _order;
}

Pathname SuseTagsPatternImpl::icon() const
{
  return _icon;
}

const CapSet & SuseTagsPatternImpl::includes() const
{
  return _includes;
}

const CapSet & SuseTagsPatternImpl::extends() const
{
  return _extends;
}


/////////////////////////////////////////////////////////////////
} // namespace detail
///////////////////////////////////////////////////////////////////
}
/////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
