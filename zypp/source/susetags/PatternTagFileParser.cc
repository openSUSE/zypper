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

      Pattern::Ptr parsePattern( const Pathname & file_r )
      {
        MIL << "Starting to parse pattern " << file_r << std::endl;
        PatternTagFileParser p;
        p.parse( file_r );
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

	  if (str::split( line, std::back_inserter(words), " " ) < 3)
	    ZYPP_THROW( parser::tagfile::ParseException( "Expected [name version release [arch] ], got [" + tag.value +"]") );

          patImpl->_name = words[0];
          patImpl->_version = words[1];
          patImpl->_release = words[2];
          if (words.size() > 3) patImpl->_arch = words[3];
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
        else if ( tag.name == "Prq" )		// package requires
        {
          patImpl->_pkgrequires = tag.values;
        }
        else if ( tag.name == "Prc" )		// package recommends
        {
          patImpl->_pkgrecommends = tag.values;
        }
      }

      void PatternTagFileParser::endParse()
      {
        #warning FIXME how to insert the specific language packages
        CapFactory _f;
	Dependencies _deps;

        for (std::list<std::string>::const_iterator it = patImpl->_recommends.begin(); it != patImpl->_recommends.end(); it++)
        {
          Capability _cap = _f.parse( ResTraits<Pattern>::kind, *it );
	  _deps[Dep::RECOMMENDS].insert(_cap);
        }

        for (std::list<std::string>::const_iterator it = patImpl->_requires.begin(); it != patImpl->_requires.end(); it++)
        {
          Capability _cap = _f.parse( ResTraits<Pattern>::kind, *it );
	  _deps[Dep::REQUIRES].insert(_cap);
        }

        for (std::list<std::string>::const_iterator it = patImpl->_conflicts.begin(); it != patImpl->_conflicts.end(); it++)
        {
          Capability _cap = _f.parse( ResTraits<Pattern>::kind, *it );
	  _deps[Dep::CONFLICTS].insert(_cap);
        }

        for (std::list<std::string>::const_iterator it = patImpl->_provides.begin(); it != patImpl->_provides.end(); it++)
        {
          Capability _cap = _f.parse( ResTraits<Pattern>::kind, *it );
	  _deps[Dep::PROVIDES].insert(_cap);
        }

        for (std::list<std::string>::const_iterator it = patImpl->_obsoletes.begin(); it != patImpl->_obsoletes.end(); it++)
        {
          Capability _cap = _f.parse( ResTraits<Pattern>::kind, *it );
	  _deps[Dep::OBSOLETES].insert(_cap);
        }

        for (std::list<std::string>::const_iterator it = patImpl->_pkgrecommends.begin(); it != patImpl->_pkgrecommends.end(); it++)
        {
          Capability _cap = _f.parse( ResTraits<Package>::kind, *it );
	  _deps[Dep::RECOMMENDS].insert(_cap);
        }

        for (std::list<std::string>::const_iterator it = patImpl->_pkgrequires.begin(); it != patImpl->_pkgrequires.end(); it++)
        {
          Capability _cap = _f.parse( ResTraits<Package>::kind, *it );
	  _deps[Dep::REQUIRES].insert(_cap);
        }

        NVRAD nvrad = NVRAD( patImpl->_name, Edition(patImpl->_version, patImpl->_release, std::string()), Arch(patImpl->_arch), _deps );
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
