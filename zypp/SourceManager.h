/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/SourceManager.h
 *
*/
#ifndef ZYPP_SOURCEMANAGER_H
#define ZYPP_SOURCEMANAGER_H

#include <iosfwd>
#include <map>

#include "zypp/base/ReferenceCounted.h"
#include "zypp/base/NonCopyable.h"

#include "zypp/Source.h"
#include "zypp/Url.h"
#include "zypp/Pathname.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  DEFINE_PTR_TYPE(SourceManager)

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : SourceManager
  //
  /** Manage a ResObject pool. */
  class SourceManager : public base::ReferenceCounted, private base::NonCopyable
  {
    friend std::ostream & operator<<( std::ostream & str, const SourceManager & obj );


  public:
    static SourceManager_Ptr sourceManager();

  private:
    /** Default ctor */
    SourceManager();
  public:
    /** Dtor */
    ~SourceManager();

  public:
    /**
     * \throws Exception
     */
    Source & findSource(const unsigned id);

    /**
     * \throws Exception
     */
    unsigned addSource(const Url & url_r, const Pathname & path_r = "/");

    /**
     * \throws Exception
     */
    void removeSource(const unsigned id);

  private:
    typedef std::map<unsigned, RW_pointer<Source> > SourceMap;
    SourceMap _sources;

    static unsigned _next_id;

  private:
    /** Singleton */
    static SourceManager_Ptr _source_manager;


  };
  ///////////////////////////////////////////////////////////////////

  /** \relates SourceManager Stream output */
  std::ostream & operator<<( std::ostream & str, const SourceManager & obj );

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_SOURCEMANAGER_H
