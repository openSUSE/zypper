/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp2/repository/memory/PatternImpl.h
 *
*/
#ifndef ZYPP_DETAIL_MEMORY_PATTERNIMPL_H
#define ZYPP_DETAIL_MEMORY_PATTERNIMPL_H

#include "zypp/detail/PatternImplIf.h"
#include "zypp/data/ResolvableData.h"
#include "zypp/Source.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace repo
  { /////////////////////////////////////////////////////////////////
    namespace memory
    {

      ///////////////////////////////////////////////////////////////////
      //
      //	CLASS NAME : PatternImpl
      //
      /**
      */
      struct PatternImpl : public zypp::detail::PatternImplIf
      {
public:
        PatternImpl(data::Pattern_Ptr ptr);
        virtual ~PatternImpl();

        virtual TranslatedText summary() const;
        virtual TranslatedText description() const;
        virtual TranslatedText category() const;
        virtual bool userVisible() const;
        virtual Label order() const;
        virtual Pathname icon() const;
        virtual Source_Ref source() const;
        
        TranslatedText _summary;
        TranslatedText _description;
        TranslatedText _category;
        bool           _visible;
        std::string    _order;
        Pathname       _icon;
      };
      ///////////////////////////////////////////////////////////////////

      /////////////////////////////////////////////////////////////////
    } // namespace memory
    ///////////////////////////////////////////////////////////////////
  } // namespace repository
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_DETAIL_PATTERNIMPL_H
