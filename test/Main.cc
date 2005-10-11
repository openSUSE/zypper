#include <iostream>
#include <fstream>
#include <iterator>
#include <algorithm>
#include <set>
#include <map>
#include <list>
#include <vector>
#include <ext/hash_set>
#include <ext/hash_map>
#include <ext/rope>

#include <zypp/base/Logger.h>
#include <zypp/base/String.h>

///////////////////////////////////////////////////////////////////

#include <zypp/Resolvable.h>

namespace zypp
{
  namespace detail
  {
    class PackageImpl;
    typedef base::shared_ptr<PackageImpl> PackageImplPtr;
  }

  class Package : public Resolvable
  {
  public:
    Package();
    Package( detail::PackageImplPtr impl_r );
    ~Package();
    const std::string & label() const;
  private:
    /** Pointer to implementation */
    detail::PackageImplPtr _pimpl;
  };
}

///////////////////////////////////////////////////////////////////

#include <zypp/detail/ResolvableImpl.h>

namespace zypp
{
  namespace detail
  {
    class PackageImpl
    {
    public:
      PackageImpl()
      {}

      ResolvableImplPtr _resolvable;
      std::string       _label;
    };
  }

  Package::Package()
  : _pimpl( new detail::PackageImpl )
  {}
  Package::Package( detail::PackageImplPtr impl_r )
  : Resolvable( impl_r->_resolvable )
  , _pimpl( impl_r )
  {}
  Package::~Package()
  {}
  const std::string & Package::label() const
  { return _pimpl->_label; }

}

///////////////////////////////////////////////////////////////////

using namespace std;
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

  detail::PackageImplPtr pi( new detail::PackageImpl );
  pi->_resolvable.reset( new detail::ResolvableImpl( ResKind("PKG"),
                                                     ResName("foo"),
                                                     ResEdition("1.0","42"),
                                                     ResArch("noarch") ) );
  pi->_label = "label for foo";

  Package p( pi );

  DBG << p << endl;
  DBG << "  \"" << p.label() << "\"" << endl;

  INT << "===[END]============================================" << endl;
  return 0;
}
