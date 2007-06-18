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

#include "zypp/RepoInfo.h"

using namespace std;
using namespace boost;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : RepoInfo::Impl
  //
  /** RepoInfo implementation. */
  struct RepoInfo::Impl
  {
    
    Impl()
      : enabled (indeterminate),
        autorefresh(indeterminate),
        type(repo::RepoType::NONE_e)
    {}
        
    ~Impl()
    {
      //MIL << std::endl;
    }
  public:
    boost::tribool enabled;
    boost::tribool autorefresh;
    repo::RepoType type;
    Url mirrorlist_url;
    std::set<Url> baseUrls;
    std::string alias;
    std::string name;
    Pathname filepath;
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

  
  
  RepoInfo & RepoInfo::setEnabled( boost::tribool enabled )
  {
    _pimpl->enabled = enabled;
    return *this;
  }

  RepoInfo & RepoInfo::setAutorefresh( boost::tribool autorefresh )
  {
    _pimpl->autorefresh = autorefresh;
    return *this;
  }

  RepoInfo & RepoInfo::setMirrorListUrl( const Url &url )
  {
    _pimpl->mirrorlist_url = url;
    return *this;
  }

  RepoInfo & RepoInfo::addBaseUrl( const Url &url )
  {
    _pimpl->baseUrls.insert(url);
    return *this;
  }

  RepoInfo & RepoInfo::setAlias( const std::string &alias )
  {
    _pimpl->alias = alias;
    return *this;
  }

  RepoInfo & RepoInfo::setType( const repo::RepoType &t )
  {
    _pimpl->type = t;
    return *this;
  }

  RepoInfo & RepoInfo::setName( const std::string &name )
  {
    _pimpl->name = name;
    return *this;
  }

  RepoInfo & RepoInfo::setFilepath( const Pathname &filepath )
  {
    _pimpl->filepath = filepath;
    return *this;
  }
  
  tribool RepoInfo::enabled() const
  { return _pimpl->enabled; }

  tribool RepoInfo::autorefresh() const
  { return _pimpl->autorefresh; }

  std::string RepoInfo::alias() const
  { return _pimpl->alias; }

  std::string RepoInfo::name() const
  { return _pimpl->name; }

  Pathname RepoInfo::filepath() const
  { return _pimpl->filepath; }
  
  repo::RepoType RepoInfo::type() const
  { return _pimpl->type; }

  Url RepoInfo::mirrorListUrl() const
  { return _pimpl->mirrorlist_url; }

  std::set<Url> RepoInfo::baseUrls() const
  { return _pimpl->baseUrls; }
    
  RepoInfo::urls_const_iterator RepoInfo::baseUrlsBegin() const
  { return _pimpl->baseUrls.begin(); }
    
  RepoInfo::urls_const_iterator RepoInfo::baseUrlsEnd() const
  { return _pimpl->baseUrls.end(); }
  
  std::ostream & RepoInfo::dumpOn( std::ostream & str ) const
  {
    str << "--------------------------------------" << std::endl;
    str << "- alias       : " << alias() << std::endl;
    std::set<Url> url_set(baseUrls());
    for ( std::set<Url>::const_iterator it = url_set.begin();
          it != url_set.end();
          ++it )
    {
      str << "- url         : " << *it << std::endl;
    }
    
    str << "- type        : " << type() << std::endl;
    str << "- enabled     : " << enabled() << std::endl;
    str << "- autorefresh : " << autorefresh() << std::endl;
    //str << "- path        : " << path() << std::endl;
    return str;
  }

  std::ostream & RepoInfo::dumpRepoOn( std::ostream & str ) const
  {
    str << "[" << alias() << "]" << endl;
    str << "name = " << name() << endl;

    if ( ! baseUrls().empty() )
      str << "baseurl = ";
    for ( urls_const_iterator it = baseUrlsBegin();
          it != baseUrlsEnd();
          ++it )
    {
      str << *it << endl;
    }
    str << "mirrorlist = " << mirrorListUrl() << endl;
    str << "type = " << type().asString() << endl;
    str << "enabled = " << (enabled() ? "1" : "0") << endl;
    return str;
  }

  std::ostream & operator<<( std::ostream & str, const RepoInfo & obj )
  {
    return obj.dumpOn(str);
  }
  
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
