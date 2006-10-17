/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/source/susetags/SuseTagsSelectionImpl.cc
 *
*/
#include "zypp/source/susetags/SuseTagsSelectionImpl.h"

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
//	METHOD NAME : SelectionImpl::SelectionImpl
//	METHOD TYPE : Ctor
//
SuseTagsSelectionImpl::SuseTagsSelectionImpl()
{}

///////////////////////////////////////////////////////////////////
//
//	METHOD NAME : SelectionImpl::~SelectionImpl
//	METHOD TYPE : Dtor
//
SuseTagsSelectionImpl::~SuseTagsSelectionImpl()
{}


TranslatedText SuseTagsSelectionImpl::summary() const
{
  return _summary;
}

TranslatedText SuseTagsSelectionImpl::description() const
{
  return _description;
}

Label SuseTagsSelectionImpl::category() const
{
  return _category;
}

bool SuseTagsSelectionImpl::visible() const
{
  return _visible;
}

Label SuseTagsSelectionImpl::order() const
{
  return _order;
}

const std::set<std::string> SuseTagsSelectionImpl::suggests() const
  {
    return _suggests;
  }

const std::set<std::string> SuseTagsSelectionImpl::recommends() const
  {
    return _recommends;
  }

const std::set<std::string> SuseTagsSelectionImpl::install_packages( const Locale & lang) const
  {
    //_inspacks[lang];
    //if(_inspacks.contains(lang))
    return ( _inspacks.find(lang)->second);
    //else
    //return std::set<std::string>();
  }

Source_Ref SuseTagsSelectionImpl::source() const
{
  return _source;
}
/////////////////////////////////////////////////////////////////
} // namespace detail
///////////////////////////////////////////////////////////////////
}
/////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
