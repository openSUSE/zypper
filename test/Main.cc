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

namespace zypp
{
  namespace base
  {
    class StringVal
    {
    public:
      operator const std::string &() const
      { return _value; }
    protected:
      StringVal()
      {}
      explicit
      StringVal( const std::string & rhs )
      : _value( rhs )
      {}
      StringVal( const StringVal & rhs )
      : _value( rhs._value )
      {}
      ~StringVal()
      {}
      const StringVal & operator=( const std::string & rhs )
      { _value = rhs; return *this; }
      const StringVal & operator=( const StringVal & rhs )
      { _value = rhs._value; return *this; }
    private:
      std::string _value;
    };

    inline std::ostream & operator<<( std::ostream & str, const StringVal & obj )
    { return str << static_cast<const std::string &>(obj); }

  }

  class ResKind : public base::StringVal
  {
  public:
    ResKind()
    {}
    explicit
    ResKind( const std::string & rhs )
    : base::StringVal( rhs )
    {}
    ResKind( const ResKind & rhs )
    : base::StringVal( rhs )
    {}
    ~ResKind()
    {}
  };
  class ResName : public base::StringVal
  {
  public:
    ResName()
    {}
    explicit
    ResName( const std::string & rhs )
    : base::StringVal( rhs )
    {}
    ResName( const ResName & rhs )
    : base::StringVal( rhs )
    {}
    ~ResName()
    {}
  };
  class ResEdition
  {
  public:
    typedef unsigned epoch_t;
    ResEdition()
    {}
    ResEdition( const ResEdition & rhs )
    {}
    ~ResEdition()
    {}
  public:
    epoch_t epoch() const
    { return 0; }
    const std::string & version() const
    { return std::string(); }
    const std::string & release() const
    { return std::string(); }
  private:

  };
  class ResArch : public base::StringVal
  {
  public:
    ResArch()
    {}
    explicit
    ResArch( const std::string & rhs )
    : base::StringVal( rhs )
    {}
    ResArch( const ResArch & rhs )
    : base::StringVal( rhs )
    {}
    ~ResArch()
    {}
  };

}

using namespace std;
using namespace zypp;

void tt ( const string & s )
{
  string t( s );
  t = string();
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

  ResKind a( "fool" );
  ResName b;
  DBG << a << endl;
  DBG << b << endl;

  //b=a;

  DBG << a << endl;
  DBG << b << endl;

  tt( a );
  tt( b );

  INT << "===[END]============================================" << endl;
  return 0;
}
