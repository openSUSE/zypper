/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/ResObject.cc
 *
*/
#include "zypp/ResObject.h"
#include "zypp/Repository.h"
#include "zypp/sat/SolvAttr.h"
extern "C"
{
#include "satsolver/repo.h"
}

class SearchQuery
{
  void search(Repo *repo, Id p, Id key, const char *match, int flags)
  {
    repo_search( repo, p, key, match, flags, SearchQuery::repo_search_cb, (void*) this);
  }
  
  static int repo_search_cb(void *cbdata, ::Solvable *s, ::Repodata *data, ::Repokey *key, ::KeyValue *kv)
  {
    SearchQuery *q = (SearchQuery *) cbdata;
  }
};

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  IMPL_PTR_TYPE(ResObject);

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : ResObject::ResObject
  //	METHOD TYPE : Ctor
  //
  ResObject::ResObject( const sat::Solvable & solvable_r )
  : Resolvable( solvable_r )
  {}

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : ResObject::~ResObject
  //	METHOD TYPE : Dtor
  //
  ResObject::~ResObject()
  {}

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : ResObject::dumpOn
  //	METHOD TYPE : std::ostream &
  //
  std::ostream & ResObject::dumpOn( std::ostream & str ) const
  {
    return Resolvable::dumpOn( str << "[S" << repository().numericId() << ":" << mediaNr() << "]" );
  }

  ///////////////////////////////////////////////////////////////////
  //
  //	ResObject interface forwarded to implementation
  //
  ///////////////////////////////////////////////////////////////////

  Text ResObject::summary() const
  { return Text(); }

  Text ResObject::description() const
  { return Text(); }

  Text ResObject::insnotify() const
  { return Text(); }

  Text ResObject::delnotify() const
  { return Text(); }

  License ResObject::licenseToConfirm() const
  { return License(); }

  Vendor ResObject::vendor() const
  { return Vendor(); }

  ByteCount ResObject::size() const
  { return ByteCount(); }

  Repository ResObject::repository() const
  { return Repository(); }

  ByteCount ResObject::downloadSize() const
  { return ByteCount(); }

  unsigned ResObject::mediaNr() const
  { return 0; }

  bool ResObject::installOnly() const
  { return false; }

  Date ResObject::buildtime() const
  { return Date(); }

  Date ResObject::installtime() const
  { return Date(); }

  const DiskUsage & ResObject::diskusage() const
  {
    static DiskUsage _du;
    return _du;
  }

   /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////

#include "zypp/ResObjects.h"
             
///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  ResObject::Ptr makeResObject( const sat::Solvable & solvable_r )
  {
    ResKind kind( solvable_r.kind() );
#define OUTS(X)  if ( kind == ResTraits<X>::kind ) return make<X>( solvable_r );
    OUTS( Atom );
    OUTS( Message );
    OUTS( Package );
    OUTS( Patch );
    OUTS( Pattern );
    OUTS( Product );
    OUTS( Script );
    OUTS( SrcPackage );
#undef OUTS
    return 0;
  }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
