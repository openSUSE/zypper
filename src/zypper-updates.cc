#include <iostream>
#include <fstream>
#include <sstream>
#include <streambuf>

#include "zypp/Patch.h"
#include "zypp/base/LogControl.h"
#include "zypp/base/Logger.h"
#include "zypp/base/String.h"
#include "zypp/Locale.h"
#include "zypp/ZYpp.h"
#include "zypp/ZYppFactory.h"
#include "zypp/base/Logger.h"
#include "zypp/Digest.h"
#include "zypp/parser/xml_escape_parser.hpp"

#include "zypper-updates.h"

using namespace std;
using namespace zypp;

extern std::list<zypp::RepoInfo> repos;
extern std::list<Error> errors;

string read_line_from_file( const Pathname &file )
{
  string buffer;
  string token;
  std::ifstream is(file.asString().c_str());
  if ( is.good() )
  {
    while(is && !is.eof())
    {
      getline(is, buffer);
      token += buffer;
    }
    is.close();
  }
  return token;
 }

// ----------------------------------------------------------------------------

void write_line_to_file( const Pathname &file, const std::string &line )
{
  std::ofstream os(file.asString().c_str());
  if ( os.good() )
  {
    os << line << endl;;
  }
  os.close();
}

// ----------------------------------------------------------------------------

Edition read_old_version()
{
  string line = read_line_from_file(XML_FILE_VERSION);
  return Edition(line);
}

void save_version( const Edition &edition )
{
  write_line_to_file( XML_FILE_VERSION, edition.asString() );
}


string read_old_token()
{
  return read_line_from_file(TOKEN_FILE);
}

void save_token( const std::string &token )
{
  write_line_to_file( TOKEN_FILE, token );
}

static std::string xml_escape( const std::string &text )
{
  iobind::parser::xml_escape_parser parser;
  return parser.escape(text);
}

void render_error( const Edition &version, std::ostream &out )
{
  out << "<?xml version='1.0'?>" << std::endl;
  out << "<update-status version=\"" << version.asString() << "\">" << std::endl;
  out << " <errors>" << std::endl;
  for ( std::list<Error>::const_iterator it = errors.begin(); it != errors.end(); ++it )
  {
    out << "   <error>" << xml_escape(it->description) << "</error>" << endl;
  }
  out << " </errors>" << std::endl;
  out << "</update-status>" << std::endl;
}

void render_result( const Edition &version, std::ostream &out, const zypp::ResPool &pool)
{
  int count = 0;
  int security_count = 0;
  
  out << "<?xml version='1.0'?>" << std::endl;
  out << "<update-status version=\"" << version.asString() << "\">" << std::endl;
  
  //out << " <metadata token=\"" << token << "\"/>" << std::endl;
  out << " <errors>" << std::endl;
  for ( std::list<Error>::const_iterator it = errors.begin(); it != errors.end(); ++it )
  {
    out << " <error>" << xml_escape(it->description) << "</error>" << endl;
  }
  out << " </errors>" << std::endl;
  
  out << " <update-sources>" << std::endl;
  for ( std::list<RepoInfo>::const_iterator it = repos.begin(); it != repos.end(); ++it )
  {
    out << "  <source url=\"" << *(it->baseUrlsBegin()) << "\" alias=\"" << it->alias() << "\"/>" << std::endl;
  }
  out << " </update-sources>" << std::endl;
  out << " <update-list>" << std::endl;
  for ( ResPool::byKind_iterator it = pool.byKindBegin<Patch>(); it != pool.byKindEnd<Patch>(); ++it )
  {
    Resolvable::constPtr res = it->resolvable();
    Patch::constPtr patch = asKind<Patch>(res);
    MIL << patch->name() << " " << patch->edition() << " " << "[" << patch->category() << "]" << ( it->status().isNeeded() ? " [needed]" : " [unneeded]" )<< std::endl;
    if ( it->status().isNeeded() )
    {
      out << " <update category=\"" << patch ->category() << "\" name=\"" << patch->name() << "\" edition=\"" << patch->edition() << "\"" << ">" << std::endl;
      out << " <summary>" << xml_escape(patch->summary()) << "</summary>" << endl;
      out << " <description>" << xml_escape(patch->description()) << "</description>" << endl;
      if ( patch->repository() != Repository::noRepository )
        out << "<source url=\"" << *(patch->repository().info().baseUrlsBegin()) << "\" alias=\"" << patch->repository().info().alias() << "\"/>" << std::endl;
      out << "</update>" << endl;
      
      count++;
      if (patch->category() == "security")
        security_count++;
    }
  }
  out << " </update-list>" << std::endl;
  out << " <update-summary total=\"" << count << "\" security=\"" << security_count << "\"/>" << std::endl;
  out << "</update-status>" << std::endl;
}
