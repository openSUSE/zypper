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


namespace zypp
{
  namespace detail {
    ResObjectImplIf::~ResObjectImplIf()
    {}
  }


// connect resolvables interface and implementation.
template<class _Res>
  class ResImplConnect : public _Res
  {
  public:
    typedef ResImplConnect                  Self;
    typedef typename _Res::Impl             Impl;
    typedef base::shared_ptr<Impl>          ImplPtr;
    typedef base::intrusive_ptr<Self>       Ptr;
    typedef base::intrusive_ptr<const Self> constPtr;
  public:
    /** \todo protect against NULL Impl. */
    ResImplConnect( const std::string & name_r,
                    const Edition & edition_r,
                    const Arch & arch_r,
                    ImplPtr impl_r )
    : _Res( name_r, edition_r, arch_r )
    , _impl( impl_r )
    { _impl->_backRef = this; }
    virtual ~ResImplConnect() {}
  private:
    ImplPtr _impl;
    virtual Impl &       pimpl()       { return *_impl; }
    virtual const Impl & pimpl() const { return *_impl; }
  };


template<class _Impl>
  typename _Impl::ResType::Ptr
  makeResolvable( const std::string & name_r,
                  const Edition & edition_r,
                  const Arch & arch_r,
                  base::shared_ptr<_Impl> & impl_r )
  {
    impl_r.reset( new _Impl );
    return new ResImplConnect<typename _Impl::ResType>( name_r,
                                                        edition_r,
                                                        arch_r,
                                                        impl_r );
  }
template<class _Impl>
  typename _Impl::ResType::Ptr
  makeResolvable( base::shared_ptr<_Impl> & impl_r )
  {
    impl_r.reset( new _Impl );
    return new ResImplConnect<typename _Impl::ResType>( "n",
                                                        Edition("v","r"),
                                                        Arch(),
                                                        impl_r );
  }

}//ns

struct PI : public zypp::detail::MessageImplIf
{
  virtual std::string text() const { return "message text"; }
  virtual std::string type() const { return "message type"; }
  virtual ~PI(){}
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

  zypp::base::shared_ptr<PI> pi;
  OUT( zypp::makeResolvable( pi ) );



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

