/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/source/susetags/PackagesLangParser.cc
 *
*/
#include <iostream>
#include "zypp/base/Logger.h"

#include "zypp/parser/ParseException.h"

#include "zypp/source/susetags/PackagesLangParser.h"
#include "zypp/parser/tagfile/TagFileParser.h"
#include "zypp/Package.h"
#include "zypp/source/susetags/SuseTagsImpl.h"
#include "zypp/source/susetags/SuseTagsPackageImpl.h"

#include "zypp/ZYppFactory.h"

using std::endl;
using namespace zypp::parser;
using namespace zypp::parser::tagfile;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
namespace source
{ /////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
namespace susetags
{ /////////////////////////////////////////////////////////////////

struct PackagesLangParser : public parser::tagfile::TagFileParser
{
  const PkgContent & _content;
  const Locale & _lang;
  PkgImplPtr _current;

  NVRA _nvra;
  int _count;
  std::set<NVRA> _notfound;
  Arch _system_arch;

  SuseTagsImpl::Ptr _sourceImpl;

  PackagesLangParser ( parser::ParserProgress::Ptr progress, SuseTagsImpl::Ptr sourceimpl , const PkgContent & content_r, const Locale & lang_r )
      : parser::tagfile::TagFileParser(progress)
      ,_content( content_r )
      , _lang( lang_r)
      , _count(0)
      , _sourceImpl( sourceimpl )

  {
    ZYpp::Ptr z = getZYpp();
    _system_arch = z->architecture();
  }

  /* Consume SingleTag data. */
  virtual void consume( const SingleTag & stag_r )
  {
    if ( stag_r.name == "Pkg" )
    {
      std::vector<std::string> words;
      str::split( stag_r.value, std::back_inserter(words) );

      if ( str::split( stag_r.value, std::back_inserter(words) ) != 4 )
        ZYPP_THROW( ParseException( "[" + _file_r.asString() + "] Parse error in tag Pkg, expected [name version release arch], found: [" + stag_r.value + "]" ) );

      Arch arch( words[3] );
      _nvra = NVRA( words[0], Edition(words[1],words[2]), arch );
      // only discard the package if it is not compatible AND it does not provide data
      // to other packages

      if (!arch.compatibleWith( _system_arch ) && !_sourceImpl->_provides_shared_data[_nvra])
      {
        _current = NULL;
        return;
      }
      _count++;
    }
    else if ( stag_r.name == "Sum" )
    {
      _sourceImpl->_package_data[_nvra]._summary = TranslatedText( stag_r.value, _lang);
    }
  }

  /* Consume MulitTag data. */
  virtual void consume( const MultiTag & mtag_r )
  {
    //if ( _current == NULL )
    //  return;

    if ( mtag_r.name == "Des" )
    {
      _sourceImpl->_package_data[_nvra]._description = TranslatedText (mtag_r.values, _lang);
    }
    else if ( mtag_r.name == "Ins" )
    {
      _sourceImpl->_package_data[_nvra]._insnotify = TranslatedText (mtag_r.values, _lang);
    }
    else if ( mtag_r.name == "Del" )
    {
      _sourceImpl->_package_data[_nvra]._delnotify = TranslatedText (mtag_r.values, _lang);
    }
    else if ( mtag_r.name == "Eul" )
    {
      _sourceImpl->_package_data[_nvra]._license_to_confirm = TranslatedText (mtag_r.values, _lang);
    }
  }
};

////////////////////////////////////////////////////////////////////////////

void parsePackagesLang( parser::ParserProgress::Ptr progress, SuseTagsImpl::Ptr sourceimpl, const Pathname & file_r, const Locale & lang_r, const PkgContent & content_r )
{
  PackagesLangParser p( progress, sourceimpl, content_r, lang_r);
  MIL <<  "Package descriptions/translations parser: [" << file_r << "]. Source [" << sourceimpl->selfSourceRef().alias() << "] at URL:[" << sourceimpl->selfSourceRef().url().asString() << "]. Starting with " << content_r.size() << " packages" << std::endl;
  try
  {
    p.parse( file_r );
  }
  catch (ParseException &e)
  {
    ZYPP_CAUGHT(e);
    ERR <<  "Bad Source [" << sourceimpl->selfSourceRef().alias() << "] at URL:[" << sourceimpl->selfSourceRef().url().asString() << "]. Packages descriptions/translations " << file_r << " is broken. You will not see translations." << std::endl;
    return;
  }

  MIL <<  "Source [" << sourceimpl->selfSourceRef().alias() << "] at URL:[" << sourceimpl->selfSourceRef().url().asString() << "]. packages.LANG parser done. [ Total packages: " << content_r.size() << " ] [ Package data: " << sourceimpl->_package_data.size() << " ]" << std::endl;

  return;
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
