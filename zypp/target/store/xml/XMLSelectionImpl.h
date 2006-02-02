/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/detail/XMLSelectionImpl.h
 *
*/
#ifndef ZYPP_TARGET_XMLSELECTIONIMPL_H
#define ZYPP_TARGET_XMLSELECTIONIMPL_H

#include "zypp/detail/SelectionImplIf.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  namespace storage
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : XMLSelectionImpl
    //
    /**
    */
    struct XMLSelectionImpl : public zypp::detail::SelectionImplIf
    {
      XMLSelectionImpl();
      virtual ~XMLSelectionImpl();

      /** selection summary (FIXME: localized) */
      TranslatedText summary() const;
      /** */
      TranslatedText description() const;

      /** selection category */
      Label category() const;

      /** selection visibility (for hidden selections) */
      bool visible() const;

      /** selection presentation order */
      Label order() const;
      
      TranslatedText _summary;
      std::string _name;
      std::string _version;
      std::string _release;
      std::string _arch;
      std::string _order;
      std::string _category;
      bool _visible;
    };
    ///////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////////////////
  } // namespace detail
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_DETAIL_SELECTIONIMPL_H
