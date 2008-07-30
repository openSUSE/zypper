/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file       zypp/repo/RepoInfoBaseImpl.h
 *
 */
#ifndef REPOINFOBASEIMPL_H_
#define REPOINFOBASEIMPL_H_

#include <string>

#include "zypp/TriBool.h"
#include "zypp/Pathname.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace repo
  { /////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //    CLASS NAME : RepoInfoBase::Impl
  //
  struct RepoInfoBase::Impl
  {
    Impl()
      : enabled (indeterminate)
      , autorefresh (indeterminate)
    {}

    Impl(const std::string & alias_)
      : enabled(indeterminate)
      , autorefresh(indeterminate)
    { setAlias(alias_); }

    ~Impl()
    {}

  public:
    TriBool enabled;
    TriBool autorefresh;
    std::string alias;
    std::string escaped_alias;
    std::string name;
    Pathname filepath;
  public:

    void setAlias(const std::string & alias_);

  private:
    friend Impl * rwcowClone<Impl>( const Impl * rhs );
    /** clone for RWCOW_pointer */
    Impl * clone() const
    { return new Impl( *this ); }
  };
  ///////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////////////////
  } // namespace repo
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////

#endif /*REPOINFOBASEIMPL_H_*/
