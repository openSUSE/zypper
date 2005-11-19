#include <iostream>
#include <iterator>
#include <functional>
#include <algorithm>
#include <zypp/base/Logger.h>
#include <zypp/Message.h>
#include <zypp/Package.h>

#define TAG INT << __PRETTY_FUNCTION__ << std::endl

using namespace std;

inline void OUT( zypp::Resolvable::constPtr p )
{
  if ( p )
    MIL << *p << endl;
  else
    MIL << "NULL" << endl;
}

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

  zypp::base::shared_ptr<PI> pi;
  OUT( makeResolvable( pi ) );



#if 0
  /* NVRA */
  zypp::base::shared_ptr<PI> pi;
  Package::Ptr p( makeResolvable( /*NVRA*/ pi ) );
  OUT( p );

  p = makeResolvable( /*NVRA*/ pi );
  OUT( p );
#endif

  INT << "===[END]============================================" << endl;
  return 0;
}

