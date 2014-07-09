/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file       zypp/repo/RepoInfoBase.cc
 *
 */
#include <iostream>

#include "zypp/ZConfig.h"
#include "zypp/repo/RepoVariables.h"

#include "zypp/repo/RepoInfoBase.h"
#include "zypp/TriBool.h"
#include "zypp/Pathname.h"

using namespace std;

///////////////////////////////////////////////////////////////////
namespace zypp
{
  ///////////////////////////////////////////////////////////////////
  namespace repo
  {

  ///////////////////////////////////////////////////////////////////
  /// \class RepoInfoBase::Impl
  /// \brief RepoInfoBase data
  ///////////////////////////////////////////////////////////////////
  struct RepoInfoBase::Impl
  {
    Impl()
      : _enabled( indeterminate )
      , _autorefresh( indeterminate )
    {}

    Impl( const std::string & alias_r )
      : _enabled( indeterminate )
      , _autorefresh( indeterminate )
    { setAlias( alias_r ); }

  public:
    TriBool	_enabled;
    TriBool	_autorefresh;
    std::string	_alias;
    std::string	_escaped_alias;
    std::string	_name;
    Pathname	_filepath;

  public:

    void setAlias( const std::string & alias_r )
    {
      _alias = _escaped_alias = alias_r;
      // replace slashes with underscores
      str::replaceAll( _escaped_alias, "/", "_" );
    }

  private:
    friend Impl * rwcowClone<Impl>( const Impl * rhs );
    /** clone for RWCOW_pointer */
    Impl * clone() const
    { return new Impl( *this ); }
  };
  ///////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //    CLASS NAME : RepoInfoBase
  //
  ///////////////////////////////////////////////////////////////////

  RepoInfoBase::RepoInfoBase()
    : _pimpl( new Impl() )
  {}

  RepoInfoBase::RepoInfoBase(const string & alias)
    : _pimpl( new Impl(alias) )
  {}

  RepoInfoBase::~RepoInfoBase()
  {}

  void RepoInfoBase::setEnabled( bool enabled )
  { _pimpl->_enabled = enabled; }

  void RepoInfoBase::setAutorefresh( bool autorefresh )
  { _pimpl->_autorefresh = autorefresh; }

  void RepoInfoBase::setAlias( const std::string &alias )
  { _pimpl->setAlias(alias); }

  void RepoInfoBase::setName( const std::string &name )
  { _pimpl->_name = name; }

  void RepoInfoBase::setFilepath( const Pathname &filepath )
  { _pimpl->_filepath = filepath; }

  // true by default (if not set by setEnabled())
  bool RepoInfoBase::enabled() const
  { return indeterminate(_pimpl->_enabled) ? true : (bool) _pimpl->_enabled; }

  // false by default (if not set by setAutorefresh())
  bool RepoInfoBase::autorefresh() const
  { return indeterminate(_pimpl->_autorefresh) ? false : (bool) _pimpl->_autorefresh; }

  std::string RepoInfoBase::alias() const
  { return _pimpl->_alias; }

  std::string RepoInfoBase::escaped_alias() const
  { return _pimpl->_escaped_alias; }

  std::string RepoInfoBase::name() const
  {
    if ( _pimpl->_name.empty() )
      return alias();
    return repo::RepoVariablesStringReplacer()( _pimpl->_name );
  }

  std::string RepoInfoBase::label() const
  {
    if ( ZConfig::instance().repoLabelIsAlias() )
      return alias();
    return name();
  }

  Pathname RepoInfoBase::filepath() const
  { return _pimpl->_filepath; }


  std::ostream & RepoInfoBase::dumpOn( std::ostream & str ) const
  {
    str << "--------------------------------------" << std::endl;
    str << "- alias       : " << alias() << std::endl;
    str << "- name        : " << name() << std::endl;
    str << "- enabled     : " << enabled() << std::endl;
    str << "- autorefresh : " << autorefresh() << std::endl;

    return str;
  }

  std::ostream & RepoInfoBase::dumpAsIniOn( std::ostream & str ) const
  {
    // we save the original data without variable replacement
    str << "[" << alias() << "]" << endl;
    str << "name=" << name() << endl;
    str << "enabled=" << (enabled() ? "1" : "0") << endl;
    str << "autorefresh=" << (autorefresh() ? "1" : "0") << endl;

    return str;
  }

  std::ostream & RepoInfoBase::dumpAsXmlOn( std::ostream & str, const std::string & content ) const
  {
    return str << "<!-- there's no XML representation of RepoInfoBase -->" << endl;
  }

  std::ostream & operator<<( std::ostream & str, const RepoInfoBase & obj )
  {
    return obj.dumpOn(str);
  }

  } // namespace repo
  ///////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
