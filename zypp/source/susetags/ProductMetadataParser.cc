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

#include "zypp/ZYppFactory.h"
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

        if (!file)
        {
            ZYPP_THROW (Exception("Bad source ["+ source_r.alias() +"] at URL:[" + source_r.url().asString() + "]. Can't open product file: [" + file_r.asString() + "]"));
        }

        std::string buffer;
	volatile_content = false;
        boost::regex e("^(([A-Z]+)(\\.([_A-Z0-9a-z]+)){0,1}) (.+)$");
        while(file && !file.eof())
        {
          getline(file, buffer);
          boost::smatch what;
          if(boost::regex_match(buffer, what, e, boost::match_extra))
          {
            if ( what.size() < 5 )
            {
              ZYPP_THROW (Exception("Corrupt source? ["+ source_r.alias() +"] at URL:[" + source_r.url().asString() + "]. Can't parse line: [" + buffer + "]"));
            }

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
            else if(key == "VENDOR")
              prodImpl->_vendor = value;
            else if(key == "SHORTLABEL")
              prodImpl->_shortlabel = value;
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
	    else if(key == "UPDATEURLS")
	    {
		std::list<std::string> items;
		boost::algorithm::split(items, value, is_space());
		std::list<std::string>::const_iterator
		    b = items.begin(),
		    e = items.end(),
		    i;
		for (i = b; i != e; ++i)
		{
		    // Url class throws in case of invalid URL
		    try
		    {
			Url url = *i;
			prodImpl->_update_urls.push_back( url );
		    }
		    catch( ... )
		    {
                      ZYPP_THROW (Exception("Bad source ["+ source_r.alias() +"] at URL:[" + source_r.url().asString() + "]. Ilegal update Url: [" + *i + "]"));
		    }
		}
	    }
            else if(key == "ARCH")
              parseLine( key, modifier, value, prodImpl->_arch);
            else if(key == "DEFAULTBASE")
              prodImpl->_default_base = value;
            else if(key == "PREREQUIRES")
              parseDependencies( key, value, prodImpl->_deps, Dep::PREREQUIRES);
            else if(key == "REQUIRES")
              parseDependencies( key, value, prodImpl->_deps, Dep::REQUIRES);
            else if(key == "PROVIDES")
              parseDependencies( key, value, prodImpl->_deps, Dep::PROVIDES);
            else if(key == "CONFLICTS")
              parseDependencies( key, value, prodImpl->_deps, Dep::CONFLICTS);
            else if(key == "OBSOLETES")
              parseDependencies( key, value, prodImpl->_deps, Dep::OBSOLETES);
            else if(key == "RECOMMENDS")
              parseDependencies( key, value, prodImpl->_deps, Dep::RECOMMENDS);
            else if(key == "SUGGESTS")
              parseDependencies( key, value, prodImpl->_deps, Dep::SUGGESTS);
            else if(key == "SUPPLEMENTS")
              parseDependencies( key, value, prodImpl->_deps, Dep::SUPPLEMENTS);
            else if(key == "ENHANCES")
              parseDependencies( key, value, prodImpl->_deps, Dep::ENHANCES);
            else if(key == "LINGUAS")
              parseLine( key, value, prodImpl->_languages);
            else if(key == "LABEL")
              parseLine( key, modifier, value, prodImpl->_summary);
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
            else if(key == "META")
              parseFileCheckSum( key, value, prodImpl->_descr_files_checksums);
            else if(key == "KEY")
              parseFileCheckSum( key, value, prodImpl->_signing_keys);
	    else if(key == "VOLATILE_CONTENT")
	      volatile_content = true;
            else
            {
              WAR << "In source ["+ source_r.alias() +"] at URL:[" + source_r.url().asString() + "]. Unknown key: [" + key + "] with value [" + value + "]" << std::endl;
              //ZYPP_THROW (Exception("Corrupt source ["+ source_r.alias() +"] at URL:[" + source_r.url().asString() + "]. Unknown key: [" + key + "] with value [" + value + "]"));
            }
          }
          else if (!buffer.empty())
          {
            WAR << "Ignoring line [" << buffer << "] in source ["<< source_r.alias() << "] at URL:[" + source_r.url().asString() << "]." << std::endl;
          }
        } // end while
        // finished parsing, store result
        // Collect basic Resolvable data

        try
        {
	  // calculate product architecture by looking through ARCH.xxx lines (key of prodImpl->_arch)
	  //  and taking the 'best' (first) architectures.

	  Arch sysarch( getZYpp()->architecture() );
	  Arch prodarch( Arch_noarch );		// default to noarch

	  // find matching ARCH.xxx line

	  std::map< std::string, std::list<std::string> >::const_iterator it = prodImpl->_arch.find( sysarch.asString() );

	  // if no matching ARCH.xxx line found, search best matching

	  if (it == prodImpl->_arch.end()) {
	    WAR << "Product does not fully support systems architecture (" << sysarch << ")" << endl;

	    for (std::map< std::string, std::list<std::string> >::const_iterator it1 = prodImpl->_arch.begin(); it1 != prodImpl->_arch.end(); ++it1) {
	      Arch arch( it1->first );
	      if (!arch.compatibleWith( sysarch )) {	// filter out incompatbile ones
		continue;
	      }
	      if (arch.compare( prodarch ) > 0) {	// found better than current
		prodarch = arch;			//  set new current
		it = it1;
	      }
	    }
	  }

	  // oops, still no match found ?

	  if (it == prodImpl->_arch.end()
	      || it->second.empty())
	  {
	    ERR << "Product incompatible with systems architecture (" << sysarch << ")" << endl;
	  }
	  else {
	    MIL << "Found matching/best arch " << it->first << endl;

	    prodarch = Arch( it->second.front() );	// first arch of matching ARCH.xxx line is best
	  }

	  MIL << "Product arch is " << prodarch << endl;

          NVRAD dataCollect( prodImpl->_name, Edition( prodImpl->_version ), prodarch, prodImpl->_deps );
          result = detail::makeResolvableFromImpl( dataCollect, prodImpl );
        }
        catch (const Exception & excpt_r)
        {
          ZYPP_THROW(Exception("Error creating product: " + excpt_r.msg()));
        }

	prodImpl->_source = source_r;
	prodImpl->_category = source_r.baseSource() ? "base" : "add-on";
	INT << "Product category set to " << prodImpl->_category << endl;
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

      void ProductMetadataParser::parseDependencies( const string &key, const string &value, Dependencies & deps, Dep deptag )
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
		else if (skind != "package") ERR << "Bad kind in content::" << key << " '" << skind << "'" << endl;
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
	    try {
		deps[deptag].insert( f.parse( kind, name ) );
	    }
	    catch (Exception & excpt_r) {
		ZYPP_CAUGHT( excpt_r );
		ERR << "Ignoring invalid " << key << " entry '" << name << "'" << endl;
	    }
	  }
      }

      void ProductMetadataParser::parseFileCheckSum( const std::string &key, const std::string &value, std::map<std::string, CheckSum> &container)
      {
        std::list<std::string> splitted;
        str::split( value, std::back_inserter(splitted), " ");
        if (splitted.size() != 3)
        {
          ZYPP_THROW (Exception("Parse error in checksum entry. Expected [algorithm checksum filename], got [" + value + "]"));
        }
        else
        {
          std::string checksum_type = splitted.front();
          splitted.pop_front();
          std::string checksum_str = splitted.front();
          splitted.pop_front();
          std::string filename = splitted.front();
          splitted.pop_front();
          MIL << "Checksum for " << filename << " is " << checksum_str << " (" << checksum_type << ")" << std::endl;
          container[filename] = CheckSum(checksum_type, checksum_str);
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
