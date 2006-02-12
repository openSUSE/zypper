/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/detail/SuseTagsProductImpl.cc
 *
*/
#include "zypp/source/susetags/SuseTagsProductImpl.h"

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
    //	METHOD NAME : SuseTagsProductImpl::SuseTagsProductImpl
    //	METHOD TYPE : Ctor
    //
    SuseTagsProductImpl::SuseTagsProductImpl()
    {}

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : SuseTagsProductImpl::~SuseTagsProductImpl
    //	METHOD TYPE : Dtor
    //
    SuseTagsProductImpl::~SuseTagsProductImpl()
    {}


    std::string SuseTagsProductImpl::category() const
    { return _category; }

    Label SuseTagsProductImpl::vendor() const
    { return _vendor; }

    Label SuseTagsProductImpl::displayName() const
    { return _displayName; }

    /////////////////////////////////////////////////////////////////
  } // namespace detail
  ///////////////////////////////////////////////////////////////////
  }
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
