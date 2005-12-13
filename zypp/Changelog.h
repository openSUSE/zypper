/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/Changelog.h
 *
*/
#ifndef ZYPP_CHANGELOG_H
#define ZYPP_CHANGELOG_H

#include <string>
#include <list>

#include "zypp/Date.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : ChangelogEntry
  //
  /** Single entry in a change log
  */
  class ChangelogEntry
  {
  public:
    /** Default ctor */
    ChangelogEntry( const Date & d,
                    const std::string & a,
                    const std::string & t )
    : _date( d ), _author( a ), _text( t )
    {};
    /** Dtor */
    ~ChangelogEntry()
    {}
    Date date() const { return _date; }
    std::string author() const { return _author; }
    std::string text() const { return _text; }

  private:
    Date _date;
    std::string _author;
    std::string _text;
  };

  /** List of ChangelogEntry. */
  typedef std::list<ChangelogEntry> Changelog;

  /** \relates ChangelogEntry */
  std::ostream & operator<<( std::ostream & out, const ChangelogEntry & obj );

  ///////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////

#endif // ZYPP_CHANGELOG_H
