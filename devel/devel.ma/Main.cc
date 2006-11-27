#include <iostream>

#include "Tools.h"

#include <string>
#include <ext/hash_set>
#include <ext/hash_fun.h>

using std::endl;
using namespace zypp;

///////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
namespace __gnu_cxx
{ /////////////////////////////////////////////////////////////////

  template<>
    struct hash<std::string>
    {
      size_t operator()( const std::string & key_r ) const
      { return __stl_hash_string( key_r.c_str() ); }
    };

  ///////////////////////////////////////////////////////////////////
  namespace strhash
  { /////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////////////////
  } // namespace strhash
  ///////////////////////////////////////////////////////////////////

  /////////////////////////////////////////////////////////////////
} // namespace __gnu_cxx
///////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////



  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////

struct Test
{
  Test( const std::string & val_r = std::string() )
  : _val( val_r )
  , _len( _val.size() )
  {}

  const std::string & val() const
  { return _val; }

  unsigned len() const
  { return _len; }

  private:
    std::string _val;
    unsigned    _len;
};

template<class _Value, class _Key>
  struct HashKey
  {
    typedef _Value value_type;
    typedef _Key   key_type;
  };

struct TestHashByString : public HashKey<Test,std::string>
{
  const std::string & operator()( const Test & value_r ) const
  { return value_r.val(); }
};

struct TestHashByUnsigned : public HashKey<Test,unsigned>
{
  unsigned operator()( const Test & value_r ) const
  { return value_r.len(); }
};


typedef __gnu_cxx::hash_set<std::string> HSet;

std::ostream & operator<<( std::ostream & str, const HSet & obj )
{
  str << "HSet:" << obj.size() << " (" << obj.bucket_count() << ")" << endl;
  for ( HSet::size_type i = 0; i < obj.bucket_count(); ++i )
    {
      HSet::size_type c = obj.elems_in_bucket( i );
      if ( c )
        str << "  [" << i << "] " << c << endl;
    }
  return str;
}

template<class _HashKey>
  struct HashFnc
  {};

///////////////////////////////////////////////////////////////////
namespace __gnu_cxx
{ /////////////////////////////////////////////////////////////////

  template<>
    template<class _HashKey>
    struct hash<HashFnc<_HashKey> >
    {
      typedef typename _HashKey::value_type value_type;
      typedef typename _HashKey::key_type   key_type;

      size_t operator()( const value_type & value_r ) const
      { return hash<key_type>()( _HashKey()( value_r ) ); }
    };

  /////////////////////////////////////////////////////////////////
} // namespace __gnu_cxx
///////////////////////////////////////////////////////////////////


/******************************************************************
**
**      FUNCTION NAME : main
**      FUNCTION TYPE : int
*/
int main( int argc, char * argv[] )
{
  //zypp::base::LogControl::instance().logfile( "" );
  INT << "===[START]==========================================" << endl;

  __gnu_cxx::hash_set<Test> t;
  __gnu_cxx::hash_set<Test, mem_fun(Test::val)> ts;
  __gnu_cxx::hash_set<Test, mem_fun(Test::len)> tu;


  INT << "===[END]============================================" << endl << endl;
  new zypp::base::LogControl::TmpLineWriter;
  return 0;
  HSet hset;
  MIL << hset << endl;

  for ( unsigned i = 0; i < 200; ++i )
    {
      std::string x( i, 'a' );
      hset.insert( x );
      MIL << hset << endl;
    }

  MIL << hset << endl;

  DBG << __gnu_cxx::hash<const char*>()( "" ) << endl;
  DBG << __gnu_cxx::hash<const char*>()( "a" ) << endl;
  DBG << __gnu_cxx::hash<const char*>()( "aa" ) << endl;
  DBG << __gnu_cxx::hash<const char*>()( "ab" ) << endl;
  DBG << __gnu_cxx::hash<const char*>()( "ac" ) << endl;
  DBG << __gnu_cxx::hash<const char*>()( "b" ) << endl;
  DBG << __gnu_cxx::hash<const char*>()( "c" ) << endl;
  DBG << __gnu_cxx::hash<const char*>()( "" ) << endl;
  DBG << __gnu_cxx::hash<std::string>()( std::string("foo") ) << endl;


  INT << "===[END]============================================" << endl << endl;
  new zypp::base::LogControl::TmpLineWriter;
  return 0;
}

