/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/parser/tagfile/ProductMetadataParser.cc
 *
*/
#include <iostream>
#include <fstream>
#include <sstream>

#include <boost/tokenizer.hpp>
#include <boost/algorithm/string.hpp>

#include "zypp/base/Logger.h"
#include "zypp/base/Exception.h"
#include "zypp/base/PtrTypes.h"
#include "zypp/base/String.h"

#include "zypp/CapFactory.h"

#include "zypp/source/susetags/ProductMetadataParser.h"
#include "zypp/source/susetags/SuseTagsProductImpl.h"
#include <boost/regex.hpp>

#undef ZYPP_BASE_LOGGER_LOGGROUP
#define ZYPP_BASE_LOGGER_LOGGROUP "ProductMetadataParser"

using namespace std;
using namespace boost;

typedef find_iterator<string::iterator> string_find_iterator;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace source
  { /////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////
    namespace susetags
    { /////////////////////////////////////////////////////////////////
      ProductMetadataParser::ProductMetadataParser()
      {
        prodImpl = new SuseTagsProductImpl;
      }
      ///////////////////////////////////////////////////////////////////
      //
      //	METHOD NAME : Parser::parse
      //	METHOD TYPE : void
      //
      void ProductMetadataParser::parse( const Pathname & file_r, Source_Ref source_r )
      {
        std::ifstream file(file_r.asString().c_str());

        if (!file) {
            ZYPP_THROW (Exception("Can't read product file :" + file_r.asString()));
        }

        std::string buffer;
        while(file && !file.eof())
        {
          getline(file, buffer);
          boost::regex e("^(([A-Z]+)(\\.([_A-Z0-9a-z]+)){0,1}) (.+)$");
          boost::smatch what;
          if(boost::regex_match(buffer, what, e, boost::match_extra))
          {
            if ( what.size() < 5 )
              std::cout << "ups!!!!" << std::endl;

            std::string key = what[2];
            std::string value = what[5];
            std::string modifier = what[4];
            if(key == "PRODUCT")
              prodImpl->_name = value;
            else if(key == "VERSION")
              prodImpl->_version = value;
            else if(key == "DISTPRODUCT")
              prodImpl->_dist = value;
            else if(key == "DISTVERSION")
              prodImpl->_dist_version = value;
             else if(key == "BASEPRODUCT")
              prodImpl->_base_product = value;
            else if(key == "BASEVERSION")
              prodImpl->_base_version = value;
            else if(key == "YOUTYPE")
              prodImpl->_you_type = value;
            else if(key == "YOUPATH")
              prodImpl->_you_path = value;
            else if(key == "YOUURL")
              prodImpl->_you_url = value;
            else if(key == "VENDOR")
              prodImpl->_vendor = value;
            else if(key == "RELNOTESURL")
	    {
		// Url class throws in case of invalid URL
		try
    		{
    		    Url url (value) ;
    		    prodImpl->_release_notes_url = url;
    		}
    		catch( ... )
    		{
		    prodImpl->_release_notes_url = Url();
    		}
	    }
            else if(key == "ARCH")
              parseLine( key, modifier, value, prodImpl->_arch);
            else if(key == "DEFAULTBASE")
              prodImpl->_default_base = value;
            else if(key == "REQUIRES")
              parseRequires( key, value, prodImpl->_requires);
            else if(key == "LINGUAS")
              parseLine( key, value, prodImpl->_languages);
            else if(key == "LABEL")
              parseLine( key, modifier, value, prodImpl->_label);
            else if(key == "DESCRDIR")
              prodImpl->_description_dir = value;
            else if(key == "DATADIR")
              prodImpl->_data_dir = value;
            else if(key == "FLAGS")
              parseLine( key, value, prodImpl->_flags);
            else if(key == "LANGUAGE")
              prodImpl->_language = value;
            else if(key == "TIMEZONE")
              prodImpl->_timezone = value;
            else
              DBG << "parse error" << std::endl;
          }
          else if (!buffer.empty())
          {
            DBG << "** No Match found:  " << buffer << std::endl;
          }
        } // end while
        // finished parsing, store result
        // Collect basic Resolvable data
        CapFactory _f;
        Dependencies deps;
        try
        {
          for (CapSet::const_iterator it = prodImpl->_requires.begin(); it != prodImpl->_requires.end(); it++)
          {
            deps[Dep::REQUIRES].insert( *it );
          }

          NVRAD dataCollect( prodImpl->_name, Edition( prodImpl->_version ), Arch_noarch, deps );
          result = detail::makeResolvableFromImpl( dataCollect, prodImpl);
        }
        catch (const Exception & excpt_r)
        {
          ERR << excpt_r << endl;
          throw "Cannot create product object";
        }

	prodImpl->_source = source_r;
      }

      void ProductMetadataParser::parseLine( const string &key, const string &modif, const string &value, map< string, list<string> > &container)
      {
        if ( modif.size() == 0)
          parseLine( key, value, container["default"]);
        else
          parseLine( key, value, container[modif]);
      }

      void ProductMetadataParser::parseLine( const std::string &key, const std::string &lang, const std::string &value, TranslatedText &container)
      {
        if ( lang.size() == 0)
          container.setText(value, Locale());
        else
          container.setText(value, Locale(lang));
      }

      void ProductMetadataParser::parseLine( const string &key, const string &modif, const string &value, map< string, string > &container)
      {
        if( modif.size() == 0)
          container["default"] = value;
        else
          container[modif] = value;
      }

      void ProductMetadataParser::parseLine( const string &key, const string &value, std::list<std::string> &container)
      {
          str::split( value, std::back_inserter(container), " ");
      }

      void ProductMetadataParser::parseRequires( const string &key, const string &value, CapSet &container)
      {
	  std::list<std::string> splitted;
          str::split( value, std::back_inserter(splitted), " ");
	  Resolvable::Kind kind;
	  std::string name;
	  CapFactory f;
	  for (std::list<std::string>::const_iterator it = splitted.begin(); it != splitted.end(); ++it) {
	    string name = *it;
	    string::size_type colon = name.find(":");
	    kind  = ResTraits<Package>::kind;
	    if (colon != string::npos) {
		string skind( name, 0, colon );
		name.erase( 0, colon+1 );
		DBG << "kind " << skind << ", name " << name << endl;
		if (skind == "pattern") kind = ResTraits<Pattern>::kind;
		else if (skind == "patch") kind = ResTraits<Patch>::kind;
		else if (skind == "selection") kind = ResTraits<Selection>::kind;
		else if (skind == "product") kind = ResTraits<Product>::kind;
		else if (skind != "package") ERR << "Bad kind in content::REQUIRES '" << skind << "'" << endl;
	    }
	    std::list<std::string>::const_iterator next = it;
	    ++next;
	    if (next != splitted.end()) {			// check for "op edition"
		string val = *next;
		if (val.find_first_of("<>=") != string::npos)
		{
		    if (++next != splitted.end()) {
			name += val;
			name += *next;
			it = next;
		    }
		}
	    }
	    DBG << "capability " << kind << ":" << name << endl;
	    try {
		container.insert( f.parse( kind, name ) );
	    }
	    catch (Exception & excpt_r) {
		ZYPP_CAUGHT( excpt_r );
		ERR << "Ignoring invalid REQUIRES entry '" << name << "'" << endl;
	    }
	  }
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
