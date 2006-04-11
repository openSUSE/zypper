/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/parser/tagfile/PatternTagFileParser.cc
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

      Pattern::Ptr parsePattern(  Source_Ref source_r, const Pathname & file_r )
      {
        MIL << "Starting to parse pattern " << file_r << std::endl;
        PatternTagFileParser p;
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

      PatternTagFileParser::PatternTagFileParser()
      {
        patImpl = new SuseTagsPatternImpl;
      }

      void PatternTagFileParser::consume( const SingleTag &tag )
      {
        if ( tag.name == "Sum" )
        {
          patImpl->_summary.setText(tag.value, Locale(tag.modifier));
        }
        else if ( tag.name == "Ver" )
        {
          patImpl->_parser_version = tag.value;
        }
        else if ( tag.name == "Pat" )
        {
          std::string line = tag.value;
          std::vector<std::string> words;

          if (str::split( line, std::back_inserter(words), " " ) != 4 )
            ZYPP_THROW( parser::tagfile::ParseException( "Expected [name version release arch] ], got [" + tag.value +"]") );

          patImpl->_name    = words[0];
          patImpl->_version = words[1];
          patImpl->_release = words[2];
          patImpl->_arch    = words[3];
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
          patImpl->_requires = tag.values;
        }
        else if ( tag.name == "Rec" )
        {
          patImpl->_recommends = tag.values;
        }
        else if ( tag.name == "Prv" )
        {
          patImpl->_provides = tag.values;
        }
        else if ( tag.name == "Obs" )
        {
          patImpl->_obsoletes = tag.values;
        }
        else if ( tag.name == "Con" )
        {
          patImpl->_conflicts = tag.values;
        }
        else if ( tag.name == "Sup" )
        {
          patImpl->_supplements = tag.values;
        }
        else if ( tag.name == "Sug" )
        {
          patImpl->_suggests = tag.values;
        }
        else if ( tag.name == "Fre" )
        {
          patImpl->_freshens = tag.values;
        }
        else if ( tag.name == "Prq" )		// package requires
        {
          patImpl->_pkgrequires = tag.values;
        }
        else if ( tag.name == "Prc" )		// package recommends
        {
          patImpl->_pkgrecommends = tag.values;
        }
        else if ( tag.name == "Psg" )		// package suggests
        {
          patImpl->_pkgsuggests = tag.values;
        }
      }

      static void parseDeps( const std::list<std::string> & strdeps, Dependencies & deps, Dep deptag, const Resolvable::Kind & kind = ResTraits<Pattern>::kind )
      {
        CapFactory f;
        for (std::list<std::string>::const_iterator it = strdeps.begin(); it != strdeps.end(); it++)
        {
          Capability cap = f.parse( kind, *it );
	  deps[deptag].insert( cap );
        }
	return;
      }

      void PatternTagFileParser::endParse()
      {
        #warning FIXME how to insert the specific language packages
	Dependencies _deps;

	parseDeps( patImpl->_recommends, _deps, Dep::RECOMMENDS );
	parseDeps( patImpl->_requires, _deps, Dep::REQUIRES );
	parseDeps( patImpl->_conflicts, _deps, Dep::CONFLICTS );
	parseDeps( patImpl->_provides, _deps, Dep::PROVIDES );
	parseDeps( patImpl->_obsoletes, _deps, Dep::OBSOLETES );
	parseDeps( patImpl->_suggests, _deps, Dep::SUGGESTS );
	parseDeps( patImpl->_supplements, _deps, Dep::SUPPLEMENTS );
	parseDeps( patImpl->_freshens, _deps, Dep::FRESHENS );
	parseDeps( patImpl->_pkgrecommends, _deps, Dep::RECOMMENDS, ResTraits<Package>::kind );
	parseDeps( patImpl->_pkgrequires, _deps, Dep::REQUIRES, ResTraits<Package>::kind );
	parseDeps( patImpl->_pkgsuggests, _deps, Dep::SUGGESTS, ResTraits<Package>::kind );

        Arch arch;
        if (!patImpl->_arch.empty())
          arch = Arch(patImpl->_arch);

        NVRAD nvrad = NVRAD( patImpl->_name, Edition( patImpl->_version, patImpl->_release, std::string() ), arch, _deps );
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
