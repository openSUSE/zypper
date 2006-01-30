/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/parser/tagfile/SelectionSelFileParser.cc
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

#include "zypp/source/susetags/SelectionSelFileParser.h"
#include <boost/regex.hpp>


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
        SelectionSelFileParser p;
        p.parse( file_r );
        return p.result;

      }

      SelectionSelFileParser::SelectionSelFileParser()
      {
        selImpl = shared_ptr<SuseTagsSelectionImpl>(new SuseTagsSelectionImpl);
      }

      void SelectionSelFileParser::consume( const SingleTag &tag )
      {
        if ( tag.name == "Sum" )
        {
          selImpl->_summary.setText(tag.value, LanguageCode(tag.modifier));
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
          DBG << "[" << words[0] << "]" << "[" << words[1] << "]" << "[" << words[2] << "]" << "[" << words[3] << "]" << std::endl;
          selImpl->_name = words[0];
          selImpl->_version = words[1];
          selImpl->_release = words[2];
          selImpl->_arch = words[3];
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
      
      void SelectionSelFileParser::consume( const MultiTag &tag )
      {
        if ( tag.name == "Req" )
        {
          selImpl->_requires = tag.values;
        }
        if ( tag.name == "Rec" )
        {
          selImpl->_recommends = tag.values;
        }
        else if ( tag.name == "Con" )
        {
          selImpl->_conflicts = tag.values;
        }
        else if ( tag.name == "Ins" )
        {
          selImpl->_inspacks[LanguageCode(tag.modifier)] = tag.values;
        }
      }

      void SelectionSelFileParser::endParse()
      {
        CapSet _required_selections;
        CapSet _recommended_selections;
        CapSet _conflicting_selections;
        CapSet _packages;
        #warning FIXME how to insert the specific language packages
        CapFactory _f;
        /*
        for (std::list<std::string>::const_iterator it = selImpl->_inspacks[LanguageCode()].begin(); it != selImpl->_inspacks[LanguageCode()].end(); it++)
        {
          Capability _cap = _f.parse( ResTraits<Package>::kind, *it);
          _packages.insert(_cap);
        }
        
        for (std::list<std::string>::const_iterator it = selImpl->_recommends.begin(); it != selImpl->_recommends.end(); it++)
        {
          Capability _cap = _f.parse( ResTraits<Selection>::kind, *it );
          _recommended_selections.insert(_cap);
        }

        for (std::list<std::string>::const_iterator it = selImpl->_requires.begin(); it != selImpl->_requires.end(); it++)
        {
          Capability _cap = _f.parse( ResTraits<Selection>::kind, *it );
          _required_selections.insert(_cap);
        }

        for (std::list<std::string>::const_iterator it = selImpl->_conflicts.begin(); it != selImpl->_conflicts.end(); it++)
        {
          Capability _cap = _f.parse( ResTraits<Selection>::kind, *it );
          _conflicting_selections.insert(_cap);
        }
        */
        NVRAD nvrad = NVRAD( selImpl->_name, Edition(selImpl->_version, selImpl->_release, std::string()), Arch(selImpl->_arch) );
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
