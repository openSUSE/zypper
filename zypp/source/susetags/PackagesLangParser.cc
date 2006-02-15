/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/source/susetags/PackagesLangParser.cc
 *
*/
#include <iostream>
#include "zypp/base/Logger.h"

#include "zypp/source/susetags/PackagesLangParser.h"
#include "zypp/parser/tagfile/TagFileParser.h"
#include "zypp/Package.h"
#include "zypp/source/susetags/SuseTagsPackageImpl.h"


using std::endl;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace source
  { /////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////
    namespace susetags
    { /////////////////////////////////////////////////////////////////

      using namespace parser::tagfile;

      struct PackagesLangParser : public parser::tagfile::TagFileParser
      {
        const PkgContent & _content;
	const Locale & _lang;
	PkgImplPtr _current;
	int _count;

	PackagesLangParser (const PkgContent & content_r, const Locale & lang_r)
	    : _content( content_r )
	    , _lang( lang_r)
	    , _count( 0 )
        { }

        /* Consume SingleTag data. */
        virtual void consume( const SingleTag & stag_r )
        {
          if ( stag_r.name == "Pkg" )
          {
            std::vector<std::string> words;
            str::split( stag_r.value, std::back_inserter(words) );

            if ( str::split( stag_r.value, std::back_inserter(words) ) != 4 )
              ZYPP_THROW( ParseException( "Pkg" ) );

            NVRAD nvrad ( words[0], Edition(words[1],words[2]), Arch(words[3]) );
	    PkgContent::const_iterator it = _content.find(nvrad);
	    if (it == _content.end()) {
		WAR << words[0] << " " << words[1] << " " << words[2] << "  " << Arch(words[3]) << " not found" << endl;
		_current = NULL;
	    }
	    else {
		_count++;
		_current = it->second;
	    }

          }
	  else if ( stag_r.name == "Sum" )
          {
	    if (_current != NULL) {
	      _current->_summary = TranslatedText( stag_r.value, _lang);
	    }
	    else ERR << "No current for " << stag_r.value << endl;
          }
        }

        /* Consume MulitTag data. */
        virtual void consume( const MultiTag & mtag_r )
        {
          if ( mtag_r.name == "Des" )
            {
	      if ( _current != NULL )
                 _current->_description = TranslatedText (mtag_r.values, _lang);
	      else
		ERR << "No current for Des" << endl;
            }
        }
      };

      ////////////////////////////////////////////////////////////////////////////

      void parsePackagesLang( const Pathname & file_r, const Locale & lang_r, const PkgContent & content_r )
      {
        PackagesLangParser p (content_r, lang_r);
	MIL << "Starting with " << content_r.size() << " packages" << endl;
        p.parse( file_r );
	MIL << "Ending after " << p._count << " langs with " << content_r.size() << " packages" <<endl;
        return;
      }

      /////////////////////////////////////////////////////////////////
    } // namespace susetags
    ///////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////
  } // namespace source
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
