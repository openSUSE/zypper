/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/source/susetags/PatternTagFileParser.cc
 *
*/
#include <iostream>
#include <fstream>
#include <sstream>

#include <boost/tokenizer.hpp>
#include <boost/algorithm/string.hpp>

#include "zypp/base/Logger.h"
#include "zypp/base/PtrTypes.h"
#include "zypp/base/String.h"
#include "zypp/CapFactory.h"

#include "zypp/source/susetags/PatternTagFileParser.h"
#include <boost/regex.hpp>

#undef ZYPP_BASE_LOGGER_LOGGROUP
#define ZYPP_BASE_LOGGER_LOGGROUP "PatternsTagFileParser"

using namespace std;
using namespace boost;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace source
  { /////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////
    namespace susetags
    { /////////////////////////////////////////////////////////////////

      struct PatternTagFileParser::Scrap
      {
        std::string _parser_version;
        std::string _name;
        std::string _version;
        std::string _release;
        std::string _arch;
        std::list<std::string> _suggests;
        std::list<std::string> _recommends;
        std::list<std::string> _requires;
        std::list<std::string> _conflicts;
        std::list<std::string> _provides;
        std::list<std::string> _obsoletes;
        std::list<std::string> _freshens;
        std::list<std::string> _supplements;
        std::list<std::string> _pkgsuggests;
        std::list<std::string> _pkgrecommends;
        std::list<std::string> _pkgrequires;
        std::list<std::string> _includes;
        std::list<std::string> _extends;
      };

      Pattern::Ptr parsePattern( parser::ParserProgress::Ptr progress, Source_Ref source_r, const Pathname & file_r )
      {
        MIL << "Starting to parse pattern " << file_r << std::endl;
        PatternTagFileParser p(progress);
        try
        {
          p.parse( file_r );
        }
        catch(zypp::parser::tagfile::ParseException &e)
        {
          ZYPP_CAUGHT(e);
          ERR << "Pattern " << file_r << " is broken." << std::endl;
          return 0L;
        }
        // attach the source
        p.patImpl->_source = source_r;
        return p.result;
      }

      PatternTagFileParser::PatternTagFileParser( parser::ParserProgress::Ptr progress )
        : parser::tagfile::TagFileParser(progress)
      {
        patImpl = new SuseTagsPatternImpl;
        _scrap.reset( new Scrap );
      }

      void PatternTagFileParser::consume( const SingleTag &tag )
      {
        if ( tag.name == "Sum" )
        {
          patImpl->_summary.setText(tag.value, Locale(tag.modifier));
        }
        else if ( tag.name == "Ver" )
        {
          _scrap->_parser_version = tag.value;
        }
        else if ( tag.name == "Pat" )
        {
          std::string line = tag.value;
          std::vector<std::string> words;

          if (str::split( line, std::back_inserter(words), " " ) != 4 )
            ZYPP_THROW( parser::tagfile::ParseException( "Expected [name version release arch] ], got [" + tag.value +"]") );

          _scrap->_name    = words[0];
          _scrap->_version = words[1];
          _scrap->_release = words[2];
          _scrap->_arch    = words[3];
        }
        else if ( tag.name == "Vis" )
        {
          patImpl->_visible = (tag.value == "true") ? true : false;
        }
        else if ( tag.name == "Cat" )
        {
          patImpl->_category.setText(tag.value, Locale(tag.modifier));
        }
        else if ( tag.name == "Ico" )
        {
          patImpl->_icon = tag.value;
        }
         else if ( tag.name == "Ord" )
        {
          patImpl->_order = tag.value;
        }
      }

      void PatternTagFileParser::consume( const MultiTag &tag )
      {
        if ( tag.name == "Des" )
        {
          std::string buffer;
          for (std::list<std::string>::const_iterator it = tag.values.begin(); it != tag.values.end(); ++it)
          {
            buffer += (*it + "\n");
          }
          patImpl->_description.setText(buffer, Locale(tag.modifier));
        }
        if ( tag.name == "Req" )
        {
          _scrap->_requires = tag.values;
        }
        else if ( tag.name == "Rec" )
        {
          _scrap->_recommends = tag.values;
        }
        else if ( tag.name == "Prv" )
        {
          _scrap->_provides = tag.values;
        }
        else if ( tag.name == "Obs" )
        {
          _scrap->_obsoletes = tag.values;
        }
        else if ( tag.name == "Con" )
        {
          _scrap->_conflicts = tag.values;
        }
        else if ( tag.name == "Sup" )
        {
          _scrap->_supplements = tag.values;
        }
        else if ( tag.name == "Sug" )
        {
          _scrap->_suggests = tag.values;
        }
        else if ( tag.name == "Fre" )
        {
          _scrap->_freshens = tag.values;
        }
        else if ( tag.name == "Prq" )		// package requires
        {
          _scrap->_pkgrequires = tag.values;
        }
        else if ( tag.name == "Prc" )		// package recommends
        {
          _scrap->_pkgrecommends = tag.values;
        }
        else if ( tag.name == "Psg" )		// package suggests
        {
          _scrap->_pkgsuggests = tag.values;
        }
        else if ( tag.name == "Inc" )		// UI hint: includes
        {
          _scrap->_includes = tag.values;
        }
        else if ( tag.name == "Ext" )		// UI hint: extends
        {
          _scrap->_extends = tag.values;
        }
      }

      static void parseDeps( const std::list<std::string> & strdeps, CapSet & capset, const Resolvable::Kind & kind = ResTraits<Pattern>::kind )
      {
        CapFactory f;
        for (std::list<std::string>::const_iterator it = strdeps.begin(); it != strdeps.end(); it++)
        {
          Capability cap = f.parse( kind, *it );
	  capset.insert( cap );
        }
	return;
      }

      void PatternTagFileParser::endParse()
      {
        #warning FIXME how to insert the specific language packages
	Dependencies _deps;

	parseDeps( _scrap->_recommends,    _deps[Dep::RECOMMENDS] );
	parseDeps( _scrap->_requires,      _deps[Dep::REQUIRES] );
	parseDeps( _scrap->_conflicts,     _deps[Dep::CONFLICTS] );
	parseDeps( _scrap->_provides,      _deps[Dep::PROVIDES] );
	parseDeps( _scrap->_obsoletes,     _deps[Dep::OBSOLETES] );
	parseDeps( _scrap->_suggests,      _deps[Dep::SUGGESTS] );
	parseDeps( _scrap->_supplements,   _deps[Dep::SUPPLEMENTS] );
	parseDeps( _scrap->_freshens,      _deps[Dep::FRESHENS] );
	parseDeps( _scrap->_pkgrecommends, _deps[Dep::RECOMMENDS], ResTraits<Package>::kind );
	parseDeps( _scrap->_pkgrequires,   _deps[Dep::REQUIRES],   ResTraits<Package>::kind );
	parseDeps( _scrap->_pkgsuggests,   _deps[Dep::SUGGESTS],   ResTraits<Package>::kind );
	parseDeps( _scrap->_includes,      patImpl->_includes );
	parseDeps( _scrap->_extends,       patImpl->_extends );

        Arch arch;
        if (!_scrap->_arch.empty())
          arch = Arch(_scrap->_arch);

        NVRAD nvrad = NVRAD( _scrap->_name, Edition( _scrap->_version, _scrap->_release, std::string() ), arch, _deps );
        result = detail::makeResolvableFromImpl( nvrad, patImpl );
      }
       /////////////////////////////////////////////////////////////////
    } // namespace tagfile
    ///////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////
  } // namespace parser
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
