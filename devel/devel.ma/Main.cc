#include <iostream>

#include "zypp/base/Easy.h"
#include "zypp/base/LogTools.h"
#include "zypp/base/InputStream.h"
#include "zypp/base/ReferenceCounted.h"
#include "zypp/base/NonCopyable.h"
#include "zypp/base/PtrTypes.h"

#include "zypp/TmpPath.h"
#include "zypp/RepoManager.h"

using std::endl;
using namespace zypp;

///////////////////////////////////////////////////////////////////

RepoManager makeRepoManager( const Pathname & mgrdir_r )
{

  RepoManagerOptions mgropt;
  mgropt.repoCachePath    = mgrdir_r/"cache";
  mgropt.repoRawCachePath = mgrdir_r/"raw_cache";
  mgropt.knownReposPath   = mgrdir_r/"repos";

  return RepoManager( mgropt );
}

struct Impl : public base::ReferenceCounted
{
  Impl() : i(13) {}
  int i;
};
DEFINE_PTR_TYPE(Impl);
IMPL_PTR_TYPE(Impl);

inline std::ostream & operator<<( std::ostream & str, const Impl & obj )
{ return str << &obj; }

#define TCODE \
  SEC << endl; \
  MIL << "n " << _nimpl << endl; \
  MIL << "n " << (_nimpl?1:0) << endl; \
  MIL << "p " << _pimpl << endl; \
  MIL << "p " << (_pimpl?1:0) << endl; \
  MIL << "P " << _Pimpl << endl; \
  MIL << "P " << (_Pimpl?1:0) << endl; \
  MIL << "= " << (_nimpl == _pimpl) << endl; \
  MIL << "! " << (_nimpl != _pimpl) << endl; \
  MIL << "= " << (_Pimpl == _pimpl) << endl; \
  MIL << "! " << (_Pimpl != _pimpl) << endl; \
  MIL << "= " << (_pimpl == _pimpl) << endl; \
  MIL << "! " << (_pimpl != _pimpl) << endl;

/******************************************************************
**
**      FUNCTION NAME : main
**      FUNCTION TYPE : int
*/
int main( int argc, char * argv[] )
{
  INT << "===[START]==========================================" << endl;

  {
    RW_pointer<Impl,rw_pointer::Intrusive<Impl> > _nimpl;
    RW_pointer<Impl,rw_pointer::Intrusive<Impl> > _pimpl(new Impl);
    RW_pointer<Impl,rw_pointer::Intrusive<Impl> > _Pimpl(new Impl);
    TCODE;
  }
  {
    RW_pointer<Impl,rw_pointer::Shared<Impl> >    _nimpl;
    RW_pointer<Impl,rw_pointer::Shared<Impl> >    _pimpl(new Impl);
    RW_pointer<Impl,rw_pointer::Shared<Impl> >    _Pimpl(new Impl);
   TCODE;
  }
  {
    RW_pointer<Impl,rw_pointer::Scoped<Impl> >    _nimpl;
    RW_pointer<Impl,rw_pointer::Scoped<Impl> >    _pimpl(new Impl);
    RW_pointer<Impl,rw_pointer::Scoped<Impl> >    _Pimpl(new Impl);
    TCODE;
  }

  INT << "===[END]============================================" << endl << endl;
  return 0;

  RepoManager repoManager( makeRepoManager( "/ROOT" ) );
  RepoInfoList repos = repoManager.knownRepositories();
  SEC << repos << endl;

  if ( repos.empty() )
  {
    RepoInfo nrepo;
    nrepo
	.setAlias( "factorytest" )
	.setName( "Test Repo for factory." )
	.setEnabled( true )
	.setAutorefresh( false )
	.addBaseUrl( Url("ftp://dist.suse.de/install/stable-x86/") );

    repoManager.addRepository( nrepo );
    repos = repoManager.knownRepositories();
    SEC << repos << endl;

//    SEC << "refreshMetadat" << endl;
//    repoManager.refreshMetadata( nrepo );
//    SEC << "buildCache" << endl;
//    repoManager.buildCache( nrepo );
//    SEC << "------" << endl;
  }

  INT << "===[END]============================================" << endl << endl;
  return 0;
}
