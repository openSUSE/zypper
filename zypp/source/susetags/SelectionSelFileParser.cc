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

      static void dumpRegexpResults( const boost::smatch &what )
      {
        for ( unsigned int k=0; k < what.size(); k++)
        {
          DBG << "[match "<< k << "] [" << what[k] << "]" << std::endl; 
        }
      }

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
          selImpl->_summary[tag.modifier] = tag.value;
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
          selImpl->_inspacks[tag.modifier] = tag.values;
        }
      }

      ///////////////////////////////////////////////////////////////////
      //
      //	METHOD NAME : Parser::parse
      //	METHOD TYPE : void
      //
      void SelectionSelFileParser::parse( const Pathname & file_r)
      {
        std::ifstream file(file_r.asString().c_str());
        std::string buffer;
        // read vendor
        while(!file.eof())
        {
          getline(file, buffer);
          boost::smatch what;
          if(boost::regex_match(buffer, what, boost::regex("^#(.*)$"), boost::match_extra))
          {
            DBG << "comment" << std::endl;
            // comment # something
            // str::strtonum(buffer, entry_r.count);
            dumpRegexpResults(what);
          }
          else if(boost::regex_match(buffer, what, boost::regex("^\\+([^[:space:]^\\.]+)(\\.([^[:space:]]+))?:$"), boost::match_extra))
          {
            MultiTag tag;
            tag.name = what[1];
            tag.modifier = what[3];

            DBG << "start list" << std::endl;
            dumpRegexpResults(what);
            // start of list +Something.lang:
            // lang is optional
            // str::strtonum(buffer, entry_r.count);
            std::string element;
            boost::smatch element_what;
            getline(file, element);
            // while we dont find the list terminator
            while( ! boost::regex_match(element, element_what, boost::regex("^\\-([^[:space:]^\\.]+)(\\.([^[:space:]]+))?:$"), boost::match_extra))
            {
              tag.values.insert(element);
              DBG << element << std::endl;
              getline(file, element);
              //dumpRegexpResults(element_what);
            }
            DBG << "end list" << std::endl;
            consume(tag);
            // end of list
          }
          else if(boost::regex_match(buffer, what, boost::regex("^=([^[:space:]^\\.]+)(\\.([^[:space:]]+))?:[[:space:]]+(.*)$"), boost::match_extra))
          {
            SingleTag tag;
            tag.name = what[1];
            tag.modifier = what[3];
            tag.value = what[4];
            DBG << "assign" << std::endl;
            // start of list
            // str::strtonum(buffer, entry_r.count);
            dumpRegexpResults(what);
            consume(tag);
          }
          else if(boost::regex_match(buffer, what, boost::regex("^([[:space:]]*)$"), boost::match_extra))
          {
            DBG << "empty line" << std::endl;
          }
          else
          {
            DBG << "parse error: " << buffer << std::endl;
          }
        }
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
