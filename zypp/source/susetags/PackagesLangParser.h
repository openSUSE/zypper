/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/source/susetags/PackagesLangParser.h
 *
*/
#ifndef ZYPP_SOURCE_SUSETAGS_PACKAGESLANGPARSER_H
#define ZYPP_SOURCE_SUSETAGS_PACKAGESLANGPARSER_H

#include <iosfwd>
#include <list>

#include "zypp/Pathname.h"
#include "zypp/Package.h"
#include "zypp/source/susetags/PackagesParser.h"
#include "zypp/source/susetags/SuseTagsImpl.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace source
  { /////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////
    namespace susetags
    { /////////////////////////////////////////////////////////////////
      
      /** \deprecated Just temporary.
       * \throws ParseException and others.
      */
      void parsePackagesLang( parser::ParserProgress::Ptr progress, SuseTagsImpl::Ptr sourceimpl, const Pathname & file_r, const Locale & lang_r, const PkgContent & content_r );

      /////////////////////////////////////////////////////////////////
    } // namespace susetags
    ///////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////
  } // namespace source
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_SOURCE_SUSETAGS_PACKAGESLANGPARSER_H
