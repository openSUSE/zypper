#include <iostream>
#include <iterator>
#include <functional>
#include <algorithm>
#include <zypp/base/Logger.h>
#include <zypp/Package.h>
#include <zypp/Selection.h>
#include <zypp/detail/PackageImpl.h>

#define TAG INT << __PRETTY_FUNCTION__ << std::endl

using namespace std;

inline void OUT( zypp::Resolvable::constPtr p )
{
  if ( p )
    MIL << *p << endl;
  else
    MIL << "NULL" << endl;
}

#if 0
struct PI : public zypp::detail::MessageImplIf
{
  virtual std::string text() const { return "message text"; }
  virtual std::string type() const { return "message type"; }
  virtual ~PI(){}
};

template<class _Impl>
  typename _Impl::ResType::Ptr
  makeResolvable( zypp::base::shared_ptr<_Impl> & impl_r )
  {
    return zypp::detail::makeResolvableAndImpl( "n",
                                                zypp::Edition("v","r"),
                                                zypp::Arch(),
                                                impl_r );
  }
{
  zypp::base::shared_ptr<PI> pi;
  OUT( makeResolvable( pi ) );
}
#endif

template<class _Impl>
  typename _Impl::ResType::Ptr
  makeResolvable( zypp::base::shared_ptr<_Impl> & impl_r )
  {
    return zypp::detail::makeResolvableAndImpl( "n",
                                                zypp::Edition("v","r"),
                                                zypp::Arch(),
                                                impl_r );
  }


using namespace zypp;

/******************************************************************
**
**
**	FUNCTION NAME : main
**	FUNCTION TYPE : int
**
**	DESCRIPTION :
*/
int main( int argc, char * argv[] )
{
  INT << "===[START]==========================================" << endl;

  base::shared_ptr<detail::PackageImpl> pi;
  Package::Ptr p( makeResolvable( pi ) );
  OUT( p );

  DBG << isKind<Resolvable>(p) << endl;
  DBG << isKind<Package>(p) << endl;
  DBG << isKind<Selection>(p) << endl;

  Resolvable::constPtr r( p );
  DBG << isKind<Resolvable>(r) << endl;
  DBG << isKind<Package>(r) << endl;
  DBG << isKind<Selection>(r) << endl;

  Package::constPtr pp = asKind<Package>(r);

  MIL << asKind<Resolvable>(r) << endl;
  MIL << asKind<ResObject>(r) << endl;


  INT << "===[END]============================================" << endl;
  return 0;
}

