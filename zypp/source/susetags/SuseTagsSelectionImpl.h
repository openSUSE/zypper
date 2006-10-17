/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/source/susetags/SuseTagsSelectionImpl.h
 *
*/
#ifndef ZYPP_DETAIL_SUSETAGS_SELECTIONIMPL_H
#define ZYPP_DETAIL_SUSETAGS_SELECTIONIMPL_H

#include "zypp/detail/SelectionImplIf.h"
#include "zypp/Source.h"

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
      //	CLASS NAME : SelectionImpl
      //
      /**
      */
      struct SuseTagsSelectionImpl : public zypp::detail::SelectionImplIf
      {
public:
        SuseTagsSelectionImpl();
        virtual ~SuseTagsSelectionImpl();

        virtual TranslatedText summary() const;
        virtual TranslatedText description() const;
        virtual Label category() const;
        virtual bool visible() const;
        virtual Label order() const;

        virtual const std::set<std::string> suggests() const PURE_VIRTUAL;
        virtual const std::set<std::string> recommends() const PURE_VIRTUAL;
        virtual const std::set<std::string> install_packages( const Locale & lang = Locale("") ) const;
        virtual Source_Ref source() const;

        TranslatedText _summary;
        TranslatedText _description;
        std::string _parser_version;
        std::string _name;
        std::string _version;
        std::string _release;
        std::string _arch;
        std::string _order;
        std::string _category;
        bool _visible;

        std::set<std::string> _suggests;
        std::set<std::string> _recommends;
        std::set<std::string> _requires;
        std::set<std::string> _conflicts;
        std::set<std::string> _provides;
        std::set<std::string> _obsoletes;
        std::map< Locale, std::set<std::string> > _inspacks;
        std::map< Locale, std::set<std::string> > _delpacks;

        Source_Ref _source;
      };
      ///////////////////////////////////////////////////////////////////

      /////////////////////////////////////////////////////////////////
    } // namespace susetags
    ///////////////////////////////////////////////////////////////////
  } // namespace source
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_DETAIL_SELECTIONIMPL_H
