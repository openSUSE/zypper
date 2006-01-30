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
        //result = detail::makeResolvableFromImpl( nvrad, selImpl )
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
