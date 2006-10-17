/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/source/susetags/SelectionTagFileParser.cc
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
#include "zypp/ZYpp.h"

#include "zypp/parser/ParserProgress.h"
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

Selection::Ptr parseSelection( parser::ParserProgress::Ptr progress, Source_Ref source_r, const Pathname & file_r )
{
  MIL <<  "Parsing selection " << file_r << " on source [" << source_r.alias() << "] at URL:[" << source_r.url().asString() << "]." << std::endl;

  SelectionTagFileParser p(progress);
  try
  {
    p.parse( file_r );
  }
  catch (zypp::parser::tagfile::ParseException &e)
  {
    ZYPP_CAUGHT(e);
    ERR <<  "Selection " << file_r << " on source [" << source_r.alias() << "] at URL:[" << source_r.url().asString() << "] is broken. Ignoring selection." << std::endl;

    return 0L;
  }
  // attach the source
  p.selImpl->_source = source_r;
  return p.result;

}

SelectionTagFileParser::SelectionTagFileParser( parser::ParserProgress::Ptr progress )
    : parser::tagfile::TagFileParser(progress)
{
  selImpl = new SuseTagsSelectionImpl;
  _locales = zypp::getZYpp()->getRequestedLocales();
}

void SelectionTagFileParser::consume( const SingleTag &tag )
{
  //MIL << "about to consume " << tag.name << " with " << tag.value << std::endl;
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

    switch ( words.size() )
    {
    case 4: // name version release arch
      selImpl->_name    = words[0];
      selImpl->_version = words[1];
      selImpl->_release = words[2];
      selImpl->_arch    = words[3];
      break;
    case 3: // name version release [arch]
      selImpl->_name    = words[0];
      selImpl->_version = words[1];
      selImpl->_release = words[2];
      break;
    case 2: // name [version release] arch
      selImpl->_name    = words[0];
      selImpl->_arch    = words[1];
      break;
    case 1: // name [version release arch]
      selImpl->_name    = words[0];
      break;
    default:
      ZYPP_THROW( parser::tagfile::ParseException( "Selection " + _file_r.asString() + ". Expected [name [version] [release] [arch] ], got [" + tag.value +"]"));
      break;
    }
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
  if ( tag.name == "Des" )
  {
    std::string buffer;
    for (std::list<std::string>::const_iterator it = tag.values.begin(); it != tag.values.end(); ++it)
    {
      buffer += (*it + "\n");
    }
    selImpl->_description.setText(buffer, Locale(tag.modifier));
  }
  if ( tag.name == "Req" )
  {
    selImpl->_requires.insert( tag.values.begin(), tag.values.end());
  }
  else if ( tag.name == "Rec" )
  {
    selImpl->_recommends.insert( tag.values.begin(), tag.values.end());
  }
  else if ( tag.name == "Prv" )
  {
    selImpl->_provides.insert( tag.values.begin(), tag.values.end());
  }
  else if ( tag.name == "Con" )
  {
    selImpl->_conflicts.insert( tag.values.begin(), tag.values.end());
  }
  else if ( tag.name == "Obs" )
  {
    selImpl->_obsoletes.insert( tag.values.begin(), tag.values.end());
  }
  else if ( tag.name == "Ins" )
  {
    selImpl->_inspacks[Locale(tag.modifier)].insert( tag.values.begin(), tag.values.end());
  }
  else if ( tag.name == "Del" )
  {
    selImpl->_delpacks[Locale(tag.modifier)].insert( tag.values.begin(), tag.values.end());
  }
}

void SelectionTagFileParser::endParse()
{
#warning Dont do this language stuff in selections
  CapFactory _f;
  Dependencies _deps;

  // get the inspacks without locale modifier

  for (std::set<std::string>::const_iterator it = selImpl->_inspacks[Locale()].begin(); it != selImpl->_inspacks[Locale()].end(); it++)
    {
      Capability _cap = _f.parse( ResTraits<Package>::kind, *it);
      _deps[Dep::RECOMMENDS].insert(_cap);
    }

  // for every requested locale, get the corresponding locale-specific inspacks
  for (ZYpp::LocaleSet::const_iterator loc = _locales.begin(); loc != _locales.end(); ++loc)
  {
    Locale l( *loc );
    std::set<std::string> locale_packs = selImpl->_inspacks[l];
    if (locale_packs.empty())
    {
      l = Locale( l.language().code() );
      locale_packs = selImpl->_inspacks[l];
    }
    for (std::set<std::string>::const_iterator it = locale_packs.begin(); it != locale_packs.end(); it++)
      {
        Capability _cap = _f.parse( ResTraits<Package>::kind, *it);
        _deps[Dep::RECOMMENDS].insert(_cap);
      }
  }

  // get the delpacks without locale modifier

  for (std::set<std::string>::const_iterator it = selImpl->_delpacks[Locale()].begin(); it != selImpl->_delpacks[Locale()].end(); it++)
    {
      Capability _cap = _f.parse( ResTraits<Package>::kind, *it);
      _deps[Dep::OBSOLETES].insert(_cap);
    }

  // for every requested locale, get the corresponding locale-specific delpacks
#warning fallback to LanguageCode (i.e. en) if Locale (i.e. en_US) doesn't match
  for (ZYpp::LocaleSet::const_iterator loc = _locales.begin(); loc != _locales.end(); ++loc)
  {
    for (std::set<std::string>::const_iterator it = selImpl->_delpacks[*loc].begin(); it != selImpl->_delpacks[*loc].end(); it++)
      {
        Capability _cap = _f.parse( ResTraits<Package>::kind, *it);
        _deps[Dep::OBSOLETES].insert(_cap);
      }
  }

  // now the real recommends

  for (std::set<std::string>::const_iterator it = selImpl->_recommends.begin(); it != selImpl->_recommends.end(); it++)
    {
      Capability _cap = _f.parse( ResTraits<Selection>::kind, *it );
      _deps[Dep::RECOMMENDS].insert(_cap);
    }

  for (std::set<std::string>::const_iterator it = selImpl->_requires.begin(); it != selImpl->_requires.end(); it++)
    {
      Capability _cap = _f.parse( ResTraits<Selection>::kind, *it );
      _deps[Dep::REQUIRES].insert(_cap);
    }

  for (std::set<std::string>::const_iterator it = selImpl->_provides.begin(); it != selImpl->_provides.end(); it++)
    {
      Capability _cap = _f.parse( ResTraits<Selection>::kind, *it );
      _deps[Dep::PROVIDES].insert(_cap);
    }

  for (std::set<std::string>::const_iterator it = selImpl->_conflicts.begin(); it != selImpl->_conflicts.end(); it++)
    {
      Capability _cap = _f.parse( ResTraits<Selection>::kind, *it );
      _deps[Dep::CONFLICTS].insert(_cap);
    }

  for (std::set<std::string>::const_iterator it = selImpl->_obsoletes.begin(); it != selImpl->_obsoletes.end(); it++)
    {
      Capability _cap = _f.parse( ResTraits<Selection>::kind, *it );
      _deps[Dep::OBSOLETES].insert(_cap);
    }
#warning: The set<string> dependencies are still kept in the selImpl but are not needed anymore
  Arch arch;
  Edition edition = Edition::noedition;
  if (!selImpl->_arch.empty())
    arch = Arch(selImpl->_arch);

  if ( ! selImpl->_version.empty() )
    edition = Edition(selImpl->_version, selImpl->_release, std::string());

  NVRAD nvrad = NVRAD( selImpl->_name, edition, arch, _deps );
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
