/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/RepoInfo.cc
 *
*/
#include <iostream>

#include "zypp/base/Logger.h"
#include "zypp/base/DefaultIntegral.h"
#include "zypp/media/MediaAccess.h"
#include "zypp/parser/xml/XmlEscape.h"

#include "zypp/RepoInfo.h"
#include "zypp/repo/RepoInfoBaseImpl.h"

using namespace std;
using zypp::xml::escape;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : RepoInfo::Impl
  //
  /** RepoInfo implementation. */
  struct RepoInfo::Impl : public repo::RepoInfoBase::Impl
  {
    Impl()
      : repo::RepoInfoBase::Impl()
      , gpgcheck(indeterminate)
      ,	keeppackages(indeterminate)
      , type(repo::RepoType::NONE_e)
    {}

    ~Impl()
    {}

  public:
    static const unsigned defaultPriority = 99;

    void setProbedType( const repo::RepoType & t ) const
    {
      if ( type == repo::RepoType::NONE
           && t != repo::RepoType::NONE )
      {
        // lazy init!
        const_cast<Impl*>(this)->type = t;
      }
    }

  public:
    TriBool gpgcheck;
    TriBool keeppackages;
    Url gpgkey_url;
    repo::RepoType type;
    Url mirrorlist_url;
    std::set<Url> baseUrls;
    Pathname path;
    std::string service;
    std::string targetDistro;
    Pathname metadatapath;
    Pathname packagespath;
    DefaultIntegral<unsigned,defaultPriority> priority;
  public:

  private:
    friend Impl * rwcowClone<Impl>( const Impl * rhs );
    /** clone for RWCOW_pointer */
    Impl * clone() const
    { return new Impl( *this ); }
  };
  ///////////////////////////////////////////////////////////////////

  /** \relates RepoInfo::Impl Stream output */
  inline std::ostream & operator<<( std::ostream & str, const RepoInfo::Impl & obj )
  {
    return str << "RepoInfo::Impl";
  }

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : RepoInfo
  //
  ///////////////////////////////////////////////////////////////////

  const RepoInfo RepoInfo::noRepo;

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : RepoInfo::RepoInfo
  //	METHOD TYPE : Ctor
  //
  RepoInfo::RepoInfo()
  : _pimpl( new Impl() )
  {}

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : RepoInfo::~RepoInfo
  //	METHOD TYPE : Dtor
  //
  RepoInfo::~RepoInfo()
  {
    //MIL << std::endl;
  }

  unsigned RepoInfo::priority() const
  { return _pimpl->priority; }
  unsigned RepoInfo::defaultPriority()
  { return Impl::defaultPriority; }
  void RepoInfo::setPriority( unsigned newval_r )
  {
    _pimpl->priority = newval_r ? newval_r : Impl::defaultPriority;
  }

  void RepoInfo::setGpgCheck( bool check )
  {
    _pimpl->gpgcheck = check;
  }

  void RepoInfo::setMirrorListUrl( const Url &url )
  {
    _pimpl->mirrorlist_url = url;
  }

  void RepoInfo::setGpgKeyUrl( const Url &url )
  {
    _pimpl->gpgkey_url = url;
  }

  void RepoInfo::addBaseUrl( const Url &url )
  {
    _pimpl->baseUrls.insert(url);
  }

  void RepoInfo::setBaseUrl( const Url &url )
  {
    _pimpl->baseUrls.clear();
    addBaseUrl(url);
  }

  void RepoInfo::setPath( const Pathname &path )
  {
    _pimpl->path = path;
  }

  void RepoInfo::setType( const repo::RepoType &t )
  {
    _pimpl->type = t;
  }

  void RepoInfo::setProbedType( const repo::RepoType &t ) const
  { _pimpl->setProbedType( t ); }


  void RepoInfo::setMetadataPath( const Pathname &path )
  {
    _pimpl->metadatapath = path;
  }

  void RepoInfo::setPackagesPath( const Pathname &path )
  {
    _pimpl->packagespath = path;
  }

  void RepoInfo::setKeepPackages( bool keep )
  {
    _pimpl->keeppackages = keep;
  }

  void RepoInfo::setService( const std::string& name )
  {
    _pimpl->service = name;
  }

  void RepoInfo::setTargetDistribution(
      const std::string & targetDistribution)
  {
    _pimpl->targetDistro = targetDistribution;
  }

  bool RepoInfo::gpgCheck() const
  { return indeterminate(_pimpl->gpgcheck) ? true : (bool) _pimpl->gpgcheck; }

  Pathname RepoInfo::metadataPath() const
  { return _pimpl->metadatapath; }

  Pathname RepoInfo::packagesPath() const
  { return _pimpl->packagespath; }

  repo::RepoType RepoInfo::type() const
  { return _pimpl->type; }

  Url RepoInfo::mirrorListUrl() const
  { return _pimpl->mirrorlist_url; }

  Url RepoInfo::gpgKeyUrl() const
  { return _pimpl->gpgkey_url; }

  std::set<Url> RepoInfo::baseUrls() const
  {
    RepoInfo::url_set replaced_urls;
    repo::RepoVariablesUrlReplacer replacer;
    for ( url_set::const_iterator it = _pimpl->baseUrls.begin();
          it != _pimpl->baseUrls.end();
          ++it )
    {
      replaced_urls.insert(replacer(*it));
    }
    return replaced_urls;

    return _pimpl->baseUrls;
  }

  Pathname RepoInfo::path() const
  { return _pimpl->path; }

  std::string RepoInfo::service() const
  { return _pimpl->service; }

  std::string RepoInfo::targetDistribution() const
  { return _pimpl->targetDistro; }

  RepoInfo::urls_const_iterator RepoInfo::baseUrlsBegin() const
  {
    return make_transform_iterator( _pimpl->baseUrls.begin(),
                                    repo::RepoVariablesUrlReplacer() );
    //return _pimpl->baseUrls.begin();
  }

  RepoInfo::urls_const_iterator RepoInfo::baseUrlsEnd() const
  {
    //return _pimpl->baseUrls.end();
    return make_transform_iterator( _pimpl->baseUrls.end(),
                                    repo::RepoVariablesUrlReplacer() );
  }

  RepoInfo::urls_size_type RepoInfo::baseUrlsSize() const
  { return _pimpl->baseUrls.size(); }

  bool RepoInfo::baseUrlsEmpty() const
  { return _pimpl->baseUrls.empty(); }

  // false by default (if not set by setKeepPackages)
  bool RepoInfo::keepPackages() const
  {
    if (indeterminate(_pimpl->keeppackages))
    {
      if (_pimpl->baseUrls.empty())
        return false;
      else if ( media::MediaAccess::downloads( *baseUrlsBegin() ) )
        return true;
      else
        return false;
    }

    return (bool) _pimpl->keeppackages;
  }


  bool RepoInfo::hasLicense() const
  { return false; }

  ManagedFile RepoInfo::getLicense( const Locale & lang_r )
  { return ManagedFile(); }

  LocaleSet RepoInfo::getLicenseLocales() const
  { return LocaleSet(); }


  std::ostream & RepoInfo::dumpOn( std::ostream & str ) const
  {
    RepoInfoBase::dumpOn(str);
    for ( urls_const_iterator it = baseUrlsBegin();
          it != baseUrlsEnd();
          ++it )
    {
      str << "- url         : " << *it << std::endl;
    }
    str << "- path        : " << path() << std::endl;
    str << "- type        : " << type() << std::endl;
    str << "- priority    : " << priority() << std::endl;

    str << "- gpgcheck    : " << gpgCheck() << std::endl;
    str << "- gpgkey      : " << gpgKeyUrl() << std::endl;
    str << "- keeppackages: " << keepPackages() << std::endl;
    str << "- service     : " << service() << std::endl;

    if (!targetDistribution().empty())
      str << "- targetdistro: " << targetDistribution() << std::endl;

    return str;
  }

  std::ostream & RepoInfo::dumpAsIniOn( std::ostream & str ) const
  {
    RepoInfoBase::dumpAsIniOn(str);

    if ( ! _pimpl->baseUrls.empty() )
      str << "baseurl=";
    for ( url_set::const_iterator it = _pimpl->baseUrls.begin();
          it != _pimpl->baseUrls.end();
          ++it )
    {
      str << *it << endl;
    }

    if ( ! _pimpl->path.empty() )
      str << "path="<< path() << endl;

    if ( ! (_pimpl->mirrorlist_url.asString().empty()) )
      str << "mirrorlist=" << _pimpl->mirrorlist_url << endl;

    str << "type=" << type().asString() << endl;

    if ( priority() != defaultPriority() )
      str << "priority=" << priority() << endl;

    if (!indeterminate(_pimpl->gpgcheck))
      str << "gpgcheck=" << (gpgCheck() ? "1" : "0") << endl;
    if ( ! (gpgKeyUrl().asString().empty()) )
      str << "gpgkey=" <<gpgKeyUrl() << endl;

    if (!indeterminate(_pimpl->keeppackages))
      str << "keeppackages=" << keepPackages() << endl;

    if( ! service().empty() )
      str << "service=" << service() << endl;

    return str;
  }

  std::ostream & RepoInfo::dumpAsXMLOn( std::ostream & str) const
  { return dumpAsXMLOn(str, ""); }

  std::ostream & RepoInfo::dumpAsXMLOn( std::ostream & str, const std::string & content) const
  {
    string tmpstr;
    str
      << "<repo"
      << " alias=\"" << escape(alias()) << "\""
      << " name=\"" << escape(name()) << "\"";
    if (type() != repo::RepoType::NONE)
      str << " type=\"" << type().asString() << "\"";
    str
      << " enabled=\"" << enabled() << "\""
      << " autorefresh=\"" << autorefresh() << "\""
      << " gpgcheck=\"" << gpgCheck() << "\"";
    if (!(tmpstr = gpgKeyUrl().asString()).empty())
      str << " gpgkey=\"" << escape(tmpstr) << "\"";
    if (!(tmpstr = mirrorListUrl().asString()).empty())
      str << " mirrorlist=\"" << escape(tmpstr) << "\"";
    str << ">" << endl;

    for (RepoInfo::urls_const_iterator urlit = baseUrlsBegin();
         urlit != baseUrlsEnd(); ++urlit)
      str << "<url>" << escape(urlit->asString()) << "</url>" << endl;

    str << "</repo>" << endl;
    return str;
  }


  std::ostream & operator<<( std::ostream & str, const RepoInfo & obj )
  {
    return obj.dumpOn(str);
  }


  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
