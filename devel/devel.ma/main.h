#ifndef main_h
#define main_h

#include <iosfwd>
#include <string>
#include "zypp/base/ReferenceCounted.h"
#include "zypp/base/NonCopyable.h"
#include <zypp/base/PtrTypes.h>

using std::string;

///////////////////////////////////////////////////////////////////

class Resolvable : public zypp::base::ReferenceCounted, private zypp::base::NonCopyable
{
  friend std::ostream & operator<<( std::ostream & str, const Resolvable & res );
  public:
    string kind() const;
    string name() const;
  public:
    typedef Resolvable Self;
    typedef zypp::base::intrusive_ptr<Self>       Ptr;
    typedef zypp::base::intrusive_ptr<const Self> constPtr;
  protected:
    Resolvable( const string kind_r /*NVRA*/ );
    virtual ~Resolvable();
    virtual std::ostream & dumpOn( std::ostream & str ) const;
  private:
    struct Impl;
    zypp::base::ImplPtr<Impl> _impl;
};

inline void intrusive_ptr_add_ref( const Resolvable * ptr_r )
{ zypp::base::ReferenceCounted::add_ref( ptr_r ); }
inline void intrusive_ptr_release( const Resolvable * ptr_r )
{ zypp::base::ReferenceCounted::release( ptr_r ); }

inline std::ostream & operator<<( std::ostream & str, const Resolvable & res )
{ return res.dumpOn( str ); }

///////////////////////////////////////////////////////////////////

// connect resolvables interface and implementation.
template<class TRes>
  struct ResImplConnect : public TRes
  {
  public:
    typedef ResImplConnect               Self;
    typedef typename TRes::Impl          Impl;
    typedef zypp::base::shared_ptr<Impl> ImplPtr;
    typedef zypp::base::intrusive_ptr<Self>       Ptr;
    typedef zypp::base::intrusive_ptr<const Self> constPtr;
  public:
    ResImplConnect( /*NVRA*/ ImplPtr impl_r )
    : _impl( impl_r ? impl_r : ImplPtr(new Impl /*NVRA*/) )
    { _impl->_backRef = this; }
    virtual ~ResImplConnect() {}
  private:
    ImplPtr _impl;
    virtual Impl &       impl()       { return *_impl; }
    virtual const Impl & impl() const { return *_impl; }
  };


template<class TImpl>
  typename TImpl::ResType::Ptr
  makeResolvable( /*NVRA*/ zypp::base::shared_ptr<TImpl> & impl_r )
  {
    impl_r.reset( new TImpl );
    return new ResImplConnect<typename TImpl::ResType>( /*NVRA*/ impl_r );
  }

///////////////////////////////////////////////////////////////////
#include <list>
using std::list;

struct ObjectImpl
{
  const Resolvable * self() const { return _backRef; }
  Resolvable *       self()       { return _backRef; }

  virtual string       summary()         const { return string(); }
  virtual list<string> description()     const { return list<string>(); }
  //virtual FSize        size()            const { return 0; }
  //virtual bool         providesSources() const { return false; }

  ObjectImpl()
  : _backRef( 0 )
  {}
  virtual ~ObjectImpl() {};

  private:
    template<class TRes>
      friend class ResImpl;
    Resolvable * _backRef;
};

class Object : public Resolvable
{
  public:
    string       summary()         const;
    list<string> description()     const;
    //FSize        size()            const;
    //bool         providesSources() const;
  public:
    typedef Object     Self;
    typedef ObjectImpl Impl;
    typedef zypp::base::intrusive_ptr<Self>       Ptr;
    typedef zypp::base::intrusive_ptr<const Self> constPtr;
  protected:
    Object( const string kind_r /*NVRA*/ );
    virtual ~Object();
  private:
    virtual Impl &       impl()       = 0;
    virtual const Impl & impl() const = 0;
};

///////////////////////////////////////////////////////////////////
class Package;
struct PackageImpl : public ObjectImpl
{
  typedef Package ResType;
  virtual string packagedata() const { return string(); }
};

class Package : public Object
{
  public:
    string packagedata() const;
  public:
    typedef Package     Self;
    typedef PackageImpl Impl;
    typedef zypp::base::intrusive_ptr<Self>       Ptr;
    typedef zypp::base::intrusive_ptr<const Self> constPtr;
  protected:
    Package( /*NVRA*/ );
    virtual ~Package();
  private:
    virtual Impl &       impl()       = 0;
    virtual const Impl & impl() const = 0;
};

///////////////////////////////////////////////////////////////////

class Selection;
struct SelectionImpl : public ObjectImpl
{
  typedef Selection ResType;
  virtual string selectiondata() const { return string(); }
};

class Selection : public Object
{
  public:
    string selectiondata() const;
  public:
    typedef Selection     Self;
    typedef SelectionImpl Impl;
    typedef zypp::base::intrusive_ptr<Self>       Ptr;
    typedef zypp::base::intrusive_ptr<const Self> constPtr;
  protected:
    Selection( /*NVRA*/ );
    virtual ~Selection();
  private:
    virtual Impl &       impl()       = 0;
    virtual const Impl & impl() const = 0;
};

///////////////////////////////////////////////////////////////////
#endif // main_h
