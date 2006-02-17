/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/detail/PatternImpl.cc
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


    TranslatedText SuseTagsPatternImpl::summary() const
    { return _summary; }

    TranslatedText SuseTagsPatternImpl::description() const
    { return _description; }

    Label SuseTagsPatternImpl::category() const
    { return _category; }

    bool SuseTagsPatternImpl::visible() const
    { return _visible; }

    Label SuseTagsPatternImpl::order() const
    { return _order; }

    /////////////////////////////////////////////////////////////////
  } // namespace detail
  ///////////////////////////////////////////////////////////////////
  }
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
