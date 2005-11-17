#include <iostream>
#include <iterator>
#include <functional>
#include <algorithm>
#include <zypp/base/Logger.h>
#include <zypp/base/PtrTypes.h>
#include "main.h"
//#include "main2.h"

#define TAG INT << __PRETTY_FUNCTION__ << std::endl

using namespace std;

inline void OUT( Resolvable::constPtr p )
{
  if ( p )
    MIL << *p << endl;
  else
    MIL << "NULL" << endl;
}

inline void OUT( Package::constPtr p )
{
  if ( p ) {
    MIL << *p << ' ' << p->packagedata() << endl;
  }
  else
    MIL << "NULL" << endl;
}
inline void OUT( Package::Ptr p )
{
  OUT( Package::constPtr( p ) );
}


struct PI : public PackageImpl
{
  virtual string packagedata() const { return "PI::packagedata"; }
};


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

  /* NVRA */
  zypp::base::shared_ptr<PI> pi;
  Package::Ptr p( makeResolvable( /*NVRA*/ pi ) );
  OUT( p );

  p = makeResolvable( /*NVRA*/ pi );
  OUT( p );

  INT << "===[END]============================================" << endl;
  return 0;
}

