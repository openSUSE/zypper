/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/detail/SelectionImplIf.h
 *
*/
#ifndef ZYPP_DETAIL_SELECTIONIMPLIF_H
#define ZYPP_DETAIL_SELECTIONIMPLIF_H

#include "zypp/detail/ResObjectImplIf.h"
#include "zypp/TranslatedText.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  class Selection;

  ///////////////////////////////////////////////////////////////////
  namespace detail
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : SelectionImplIf
    //
    /** Abstract Selection implementation interface.
    */
    class SelectionImplIf : public ResObjectImplIf
    {
    public:
      typedef Selection ResType;

    public:

        /** */
	virtual Label category() const PURE_VIRTUAL;

        /** */
	virtual bool visible() const PURE_VIRTUAL;

        /** */
	virtual Label order() const PURE_VIRTUAL;

        /** */
        virtual const std::set<std::string> install_packages( const Locale & lang = Locale("") ) const;
    };
    ///////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////////////////
  } // namespace detail
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_DETAIL_SELECTIONIMPLIF_H
