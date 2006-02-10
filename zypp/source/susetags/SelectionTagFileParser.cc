/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/parser/tagfile/SelectionTagFileParser.cc
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

#include "zypp/source/susetags/SelectionTagFileParser.h"
#include <boost/regex.hpp>

#undef ZYPP_BASE_LOGGER_LOGGROUP
#define ZYPP_BASE_LOGGER_LOGGROUP "SelectionsTagFileParser"

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

      Selection::Ptr parseSelection( const Pathname & file_r )
      {
	SelectionTagFileParser p;
	p.parse( file_r );
	return p.result;

      }

      SelectionTagFileParser::SelectionTagFileParser()
      {
	selImpl = shared_ptr<SuseTagsSelectionImpl>(new SuseTagsSelectionImpl);
      }

      void SelectionTagFileParser::consume( const SingleTag &tag )
      {
	if ( tag.name == "Sum" )
	{
	  selImpl->_summary.setText(tag.value, Locale(tag.modifier));
	}
	else if ( tag.name == "Ver" )
	{
	  selImpl->_parser_version = tag.value;
	}
	else if ( tag.name == "Sel" )
	{
	  std::string line = tag.value;
	  std::vector<std::string> words;
	  str::split( line, std::back_inserter(words), " " );

	  selImpl->_name = words[0];
	  selImpl->_version = words[1];
	  selImpl->_release = words[2];
	  if (words.size() > 3) selImpl->_arch = words[3];
	}
	else if ( tag.name == "Vis" )
	{
	  selImpl->_visible = (tag.value == "true") ? true : false;
	}
	else if ( tag.name == "Cat" )
	{
	  selImpl->_category = tag.value;
	}
	 else if ( tag.name == "Ord" )
	{
	  selImpl->_order = tag.value;
	}
      }

      void SelectionTagFileParser::consume( const MultiTag &tag )
      {
	if ( tag.name == "Req" )
	{
	  selImpl->_requires = tag.values;
	}
	else if ( tag.name == "Rec" )
	{
	  selImpl->_recommends = tag.values;
	}
	else if ( tag.name == "Prv" )
	{
	  selImpl->_provides = tag.values;
	}
	else if ( tag.name == "Con" )
	{
	  selImpl->_conflicts = tag.values;
	}
	else if ( tag.name == "Obs" )
	{
	  selImpl->_obsoletes = tag.values;
	}
	else if ( tag.name == "Ins" )
	{
	  selImpl->_inspacks[Locale(tag.modifier)] = tag.values;
	}
      }

      void SelectionTagFileParser::endParse()
      {
	#warning FIXME how to insert the specific language packages
	CapFactory _f;
	Dependencies _deps;

	for (std::list<std::string>::const_iterator it = selImpl->_inspacks[Locale()].begin(); it != selImpl->_inspacks[Locale()].end(); it++)
	{
	  Capability _cap = _f.parse( ResTraits<Package>::kind, *it);
	  _deps[Dep::RECOMMENDS].insert(_cap);
	}

	for (std::list<std::string>::const_iterator it = selImpl->_recommends.begin(); it != selImpl->_recommends.end(); it++)
	{
	  Capability _cap = _f.parse( ResTraits<Selection>::kind, *it );
	  _deps[Dep::RECOMMENDS].insert(_cap);
	}

	for (std::list<std::string>::const_iterator it = selImpl->_requires.begin(); it != selImpl->_requires.end(); it++)
	{
	  Capability _cap = _f.parse( ResTraits<Selection>::kind, *it );
	  _deps[Dep::REQUIRES].insert(_cap);
	}

	for (std::list<std::string>::const_iterator it = selImpl->_provides.begin(); it != selImpl->_provides.end(); it++)
	{
	  Capability _cap = _f.parse( ResTraits<Selection>::kind, *it );
	  _deps[Dep::PROVIDES].insert(_cap);
	}

	for (std::list<std::string>::const_iterator it = selImpl->_conflicts.begin(); it != selImpl->_conflicts.end(); it++)
	{
	  Capability _cap = _f.parse( ResTraits<Selection>::kind, *it );
	  _deps[Dep::CONFLICTS].insert(_cap);
	}

	for (std::list<std::string>::const_iterator it = selImpl->_obsoletes.begin(); it != selImpl->_obsoletes.end(); it++)
	{
	  Capability _cap = _f.parse( ResTraits<Selection>::kind, *it );
	  _deps[Dep::OBSOLETES].insert(_cap);
	}

	NVRAD nvrad = NVRAD( selImpl->_name, Edition(selImpl->_version, selImpl->_release, std::string()), Arch(selImpl->_arch), _deps );
	result = detail::makeResolvableFromImpl( nvrad, selImpl );
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
