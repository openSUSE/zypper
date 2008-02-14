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

#include <zypp/base/Logger.h>
#include "zypp/ResObject.h"
#include "zypp/Repository.h"
#include "zypp/sat/SolvAttr.h"
#include "zypp/sat/Solvable.h"
#include "zypp/sat/Repo.h"

extern "C"
{
#include "satsolver/repo.h"
}

using namespace zypp;
using namespace std;

class SearchQuery
{
  public:

  SearchQuery( const sat::Solvable &solvable_r )
    : _solvable(solvable_r)
  {

  }

  void lookup( const sat::SolvAttr &attr )
  {
    //search( _solvable.repo().get(), _solvable.id(), attr.idStr().id(), 0, 0 );
    //return repo_lookup_str(_solvable.get(), attr.idStr().id());
  }

  void search(Repo *repo, Id p, Id key, const char *match, int flags)
  {
    repo_search( repo, p, key, match, flags, SearchQuery::repo_search_cb, (void*) this);
  }

  static int repo_search_cb(void *cbdata, ::Solvable *s, ::Repodata *data, ::Repokey *key, ::KeyValue *kv)
  {
    cout << "found attribute" << endl;
    SearchQuery *q = (SearchQuery *) cbdata;
    const char *keyname;
    keyname = id2str(data->repo->pool, key->name);
    switch(key->type)
    {
      case TYPE_ID:
      //if (data->localpool)
      //  kv->str = stringpool_id2str(&data->spool, kv->id);
      //else
      //  kv->str = id2str(data->repo->pool, kv->id);
      //  printf("%s: %s\n", keyname, kv->str);
      break;
      case TYPE_STR:
        q->_result = kv->str;
      break;
    }
    return 1;
  }

  sat::Solvable _solvable;
  std::string _result;
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
  {
    return lookupStrAttribute( sat::SolvAttr::summary );
  }

  Text ResObject::description() const
  { return lookupStrAttribute( sat::SolvAttr::description ); }

  Text ResObject::insnotify() const
  { return lookupStrAttribute( sat::SolvAttr::insnotify ); }

  Text ResObject::delnotify() const
  { return lookupStrAttribute( sat::SolvAttr::delnotify ); }

  License ResObject::licenseToConfirm() const
  { return lookupStrAttribute( sat::SolvAttr::eula ); }

  ByteCount ResObject::size() const
  { return 1024 * lookupNumAttribute( sat::SolvAttr::size ); }

  ByteCount ResObject::downloadSize() const
  { return 1024 * lookupNumAttribute( sat::SolvAttr::downloadsize ); }

  RepoInfo ResObject::repoInfo() const
  { return repo().info(); }

  unsigned ResObject::mediaNr() const
  { return 1; }

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
