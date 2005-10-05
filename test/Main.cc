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

using namespace std;
using __gnu_cxx::hash_set;

/******************************************************************
**
*/
inline void memusage()
{
  system( zypp::base::string::form( "ps v %d", getpid() ).c_str() );
}

/******************************************************************
**
*/
struct StringHashFnc
{
  size_t operator()( const string & str ) const
  {
    unsigned long __h = 0;
    for ( const char* __s = str.c_str(); *__s; ++__s)
      __h = 5*__h + *__s;

    return size_t(__h);
    //return str.size();
  }
};

/******************************************************************
**
*/
template<typename Cont>
  struct lookupKey
  {
    const Cont & _cont;
    lookupKey( const Cont & cont )
    : _cont( cont )
    {}
    void operator()( const string & key ) const
    {
      typename Cont::const_iterator it = _cont.find( key );
    }
  };

template<typename Cont>
  struct lookupNoKey
  {
    const Cont & _cont;
    lookupNoKey( const Cont & cont )
    : _cont( cont )
    {}
    void operator()( const string & key )
    {
      typename Cont::const_iterator it = _cont.find( key+'x' );
    }
  };

template<typename Cont>
  void lookup( const Cont & unames )
  {
    for ( unsigned i = 0; i < 1000; ++i ) {
      for_each( unames.begin(), unames.end(), lookupKey<Cont>( unames ) );
      for_each( unames.begin(), unames.end(), lookupNoKey<Cont>( unames ) );
    }
  }

template<typename Cont>
  void lookupTest( const vector<string> & names )
  {
    Cont unames( names.begin(), names.end() );
    MIL << "Unique names: " << unames.size() << endl;
    lookup( unames );
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

  ifstream f( "./NameList" );
  vector<string> names( (istream_iterator<string>(f)), istream_iterator<string>() );
  MIL << "Total names: " << names.size() << endl;
  memusage();

  INT << ">===[lookupTest<set<string> >]" << endl;
  lookupTest<set<string> >( names );
  memusage();

  INT << ">===[lookupTest<hash_set<string> >]" << endl;
  lookupTest<hash_set<string,StringHashFnc> >( names );
  memusage();

  INT << "===[END]============================================" << endl;
  return 0;
}
