/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/detail/SelectionImpl.cc
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
    { return _summary; }

    TranslatedText SuseTagsSelectionImpl::description() const
    { return _summary; }

    Label SuseTagsSelectionImpl::category() const
    { return _category; }

    bool SuseTagsSelectionImpl::visible() const
    { return _visible; }

    Label SuseTagsSelectionImpl::order() const
    { return _order; }

    /////////////////////////////////////////////////////////////////
  } // namespace detail
  ///////////////////////////////////////////////////////////////////
  }
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
