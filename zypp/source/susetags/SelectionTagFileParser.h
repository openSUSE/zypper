/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/source/susetags/SelectionTagFileParser.h
 *
*/
#ifndef ZYPP_PARSER_TAGFILE_SELECTIONTAGFILEPARSER_H
#define ZYPP_PARSER_TAGFILE_SELECTIONTAGFILEPARSER_H

#include <iosfwd>
#include <set>
#include <map>
#include <list>

#include "zypp/parser/tagfile/TagFileParser.h"
#include "zypp/parser/tagfile/ParseException.h"
#include "zypp/Selection.h"
#include "zypp/source/susetags/SuseTagsSelectionImpl.h"

#include "zypp/ZYppFactory.h"
#include "zypp/Pathname.h"
#include "zypp/Source.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace source
  { /////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////
    namespace susetags
    { /////////////////////////////////////////////////////////////////

      ///////////////////////////////////////////////////////////////////
      //
      //	CLASS NAME : SelectionTagFileParser
      //
      /** Tagfile parser. */
      struct SelectionTagFileParser : public zypp::parser::tagfile::TagFileParser
      {
        Selection::Ptr result;
        detail::ResImplTraits<SuseTagsSelectionImpl>::Ptr selImpl;

        SelectionTagFileParser( parser::ParserProgress::Ptr progress );
        virtual ~SelectionTagFileParser()
        {}

        void consume( const SingleTag &tag );
        void consume( const MultiTag &tag );
        void endParse();

private:
        ZYpp::LocaleSet _locales;
      };
      ///////////////////////////////////////////////////////////////////
      Selection::Ptr parseSelection(  parser::ParserProgress::Ptr progress, Source_Ref source_r, const Pathname & file_r );
      /////////////////////////////////////////////////////////////////
    } // namespace source
    ///////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////
  } // namespace susetags
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
//
#endif //  ZYPP_PARSER_TAGFILE_SELECTIONTAGFILEPPARSER_H
