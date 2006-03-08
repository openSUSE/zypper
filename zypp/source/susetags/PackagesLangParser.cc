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

#include "zypp/ZYppFactory.h"

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
        NVRAD _nvrad;
	int _count;
        std::set<NVRAD> _notfound;
	Arch _system_arch;

	PackagesLangParser (const PkgContent & content_r, const Locale & lang_r)
	    : _content( content_r )
	    , _lang( lang_r)
	    , _count(0)
        {
	    ZYpp::Ptr z = getZYpp();
	    _system_arch = z->architecture();
	}

        /* Consume SingleTag data. */
        virtual void consume( const SingleTag & stag_r )
        {
          if ( stag_r.name == "Pkg" )
          {
            std::vector<std::string> words;
            str::split( stag_r.value, std::back_inserter(words) );

            if ( str::split( stag_r.value, std::back_inserter(words) ) != 4 )
              ZYPP_THROW( ParseException( "[" + _file_r.asString() + "] Parse error in tag Pkg, expected [name version release arch], found: [" + stag_r.value + "]" ) );

	    Arch arch( words[3] );
	    if (!arch.compatibleWith( _system_arch )) {
		_current = NULL;
		return;
	    }

            _nvrad = NVRAD( words[0], Edition(words[1],words[2]), arch );
	    PkgContent::const_iterator it = _content.find(_nvrad);
	    if (it == _content.end())
            {
              // package not found in the master package list
		_current = NULL;
                _notfound.insert(_nvrad);
	    }
	    else
            {
              //WAR << "Data for package " << words[0] << " " << words[1] << " " << words[2] << "  " << Arch(words[3]) << " coming..." << endl;
		_count++;
		_current = it->second;
	    }

          }
	  else if ( stag_r.name == "Sum" )
          {
	    if (_current != NULL)
	      _current->_summary = TranslatedText( stag_r.value, _lang);
          }
        }

        /* Consume MulitTag data. */
        virtual void consume( const MultiTag & mtag_r )
        {
          if ( mtag_r.name == "Des" )
            {
	      if ( _current != NULL )
                 _current->_description = TranslatedText (mtag_r.values, _lang);
            }
        }
      };

      ////////////////////////////////////////////////////////////////////////////

      void parsePackagesLang( const Pathname & file_r, const Locale & lang_r, const PkgContent & content_r )
      {
        PackagesLangParser p (content_r, lang_r);
	MIL << "Starting with " << content_r.size() << " packages" << endl;
        try
        {
          p.parse( file_r );
        }
        catch(zypp::parser::tagfile::ParseException &e)
        {
          ZYPP_CAUGHT(e);
          ERR << "Packages Lang " << file_r << " is broken." << std::endl;
          return;
        }
        
        MIL << "Ending after " << p._count << " langs with " << content_r.size() << " packages and " << p._notfound.size() << " not founds." <<endl;
        WAR << "Not found packages:" << std::endl;
        for ( std::set<NVRAD>::const_iterator it = p._notfound.begin(); it != p._notfound.end(); ++it)
        {
          NVRAD nvrad = *it;
          WAR << "-> " << nvrad.name << " " << nvrad.edition << " " << nvrad.arch << std::endl;
        }
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
